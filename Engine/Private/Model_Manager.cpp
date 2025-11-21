#include"EnginePch.h"
#include "Model_Manager.h"
#include"GameInstance.h"
#include"Model_Streaming.h"
#include"StaticObject.h"

CModel_Manager::CModel_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pDevice(pDevice), m_pContext(pContext),m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext); 
	Safe_AddRef(m_pGameInstance);
}

HRESULT CModel_Manager::Initialize()
{
	_float fSize = 0.01f;
	XMStoreFloat4x4(&m_PreTransformMatrix, XMMatrixScaling(fSize, fSize, fSize));
	m_pBufferPool[0] = CBufferPool::Create(m_pDevice, m_pContext, 256, sizeof(VTXMESH));
	m_pBufferPool[1] = CBufferPool::Create(m_pDevice, m_pContext, 128, sizeof(VTXMESH));
	m_pBufferPool[2] = CBufferPool::Create(m_pDevice, m_pContext, 64, sizeof(VTXMESH));
	m_pBufferPool[3] = CBufferPool::Create(m_pDevice, m_pContext, 64, sizeof(VTXMESH));

	D3D11_BUFFER_DESC StagingDesc = {};
	StagingDesc.ByteWidth = 1024 * 1024 * 64;
	StagingDesc.Usage = D3D11_USAGE_STAGING;
	StagingDesc.BindFlags = 0;
	StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&StagingDesc, nullptr, &m_pStagingBuffer)))
		CRASH("Failed");
	m_iSearchIndex = m_ModelPrototypes.begin();
	m_RenderObjects[0].reserve(500);
	m_RenderObjects[1].reserve(500);
	m_RenderObjects[2].reserve(500);
	m_RenderObjects[3].reserve(500);
	return S_OK;
}

void CModel_Manager::Update(_float fTimeDelta)
{
	m_fTotalPlayTime = m_pGameInstance->Get_PlayTime();
	iCurrentLoadCnt = 0;
	for (_uint i = 0; i < m_DeleteList.size(); ++i)
	{
		auto& Data = m_DeleteList[i];
		Data.iLifeCount--;
		if (Data.iLifeCount <= 0)
		{
			m_pBufferPool[Data.iLODIndex]->FreeMemory_Vertex(Data.VertexOffset, Data.VertexSize);
			m_pBufferPool[Data.iLODIndex]->FreeMemory_Index(Data.IndexOffset, Data.IndexSize);
			if (i != m_DeleteList.size() - 1)
				m_DeleteList[i] = m_DeleteList.back();
			m_DeleteList.pop_back();
		}
		else
			i++;
	}
	
	if (!m_StagingData.empty())
	{
		vector<MODEL_DATA> pTempVector;
		{
			lock_guard<mutex> lock(m_StagingMutex);
			pTempVector = move(m_StagingData);
		}

		for (auto& Data : pTempVector)
		{
			//Data의 Data.LoadData 개수가 메쉬의 개수.
			vector< SHARED_DATA_DESC>* pData = Data.pModel->Get_MeshDesc(Data.iLODIndex);
			pData->clear();
			if (Data.LoadData.empty())
				continue;
			for (_uint i = 0; i < Data.LoadData.size(); ++i)
			{
				_uint VertexSize = Data.LoadData[i].VertexData.size() * sizeof(VTXMESH);
				_uint VertexOffset = m_pBufferPool[Data.iLODIndex]->Allocate_Vertex(VertexSize);

				if (VertexOffset == -1)
					CRASH("Failed");

				D3D11_BOX PoolBox{};
				PoolBox.left = VertexOffset;
				PoolBox.top = 0;
				PoolBox.front = 0;
				PoolBox.right = VertexOffset + VertexSize;
				PoolBox.bottom = 1;
				PoolBox.back = 1;

				m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(), 0, &PoolBox, Data.LoadData[i].VertexData.data(), 0, 0);

				D3D11_MAPPED_SUBRESOURCE StagingDesc{};
				/*m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
				memcpy(StagingDesc.pData, Data.LoadData[i].VertexData.data(), VertexSize);
				m_pContext->Unmap(m_pStagingBuffer, 0);*/

				///*D3D11_BOX */PoolBox = { 0,0,0,VertexSize,1,1 };
				//m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(),
				//	0, VertexOffset, 0, 0, m_pStagingBuffer, 0, &PoolBox);


				_uint IndexSize = Data.LoadData[i].IndexData.size() * sizeof(_uint);
				_uint IndexOffSet = m_pBufferPool[Data.iLODIndex]->Allocate_Index(IndexSize);

				if (IndexOffSet == -1)
					CRASH("Failed");


				PoolBox.left = IndexOffSet;
				PoolBox.top = 0;
				PoolBox.front = 0;
				PoolBox.right = IndexOffSet + IndexSize;
				PoolBox.bottom = 1;
				PoolBox.back = 1;

				m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(), 0, &PoolBox, Data.LoadData[i].IndexData.data(), 0, 0);

				//m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
				//memcpy(StagingDesc.pData, Data.LoadData[i].IndexData.data(), IndexSize);
				//m_pContext->Unmap(m_pStagingBuffer, 0);

				/*PoolBox = { 0,0,0,IndexSize,1,1 };
				m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(),
					0, IndexOffSet, 0, 0, m_pStagingBuffer, 0, &PoolBox);*/

				SHARED_DATA_DESC Desc{};
				Desc.IndexOffset = IndexOffSet / sizeof(_uint);
				Desc.IndexSize = IndexSize;
				Desc.NumIndices = Data.LoadData[i].iNumIndices;
				Desc.VertexOffset = VertexOffset / sizeof(VTXMESH);
				Desc.VertexSize = VertexSize;

				pData->push_back(Desc);
			}
			Data.pModel->Get_MeshState(Data.iLODIndex).store(LOADSTATE::LOADED);
		}
		Release_Vector(pTempVector);
	}

	if (m_ModelPrototypes.empty())
		return;


	if (m_iSearchIndex == m_ModelPrototypes.end())
		m_iSearchIndex = m_ModelPrototypes.begin();
	_uint iCheckCount = { 0 };
	while (iCheckCount < m_iCheckPerFrame && m_iSearchIndex != m_ModelPrototypes.end())
	{
		CModel_Streaming* pModel = m_iSearchIndex->second;
		for (_uint i = 0; i < 3; ++i)
		{
			if (pModel->Is_RenderTimeOver(i) && pModel->Get_MeshState(i) == LOADSTATE::LOADED)
			{
				if (pModel->Get_MeshDesc(0)[i].empty())
					continue;
				vector<SHARED_DATA_DESC>* pMeshVector = pModel->Get_MeshDesc(i);
				if (pMeshVector->empty())
					continue;
				for (auto& pDesc : *pMeshVector)
				{
					DELETE_DATA Data{};
					Data.iLifeCount = 3;
					Data.iLODIndex = i;
					Data.IndexOffset = pDesc.IndexOffset * sizeof(_uint);
					Data.IndexSize = pDesc.IndexSize;
					Data.VertexOffset = pDesc.VertexOffset * sizeof(VTXMESH);
					Data.VertexSize = pDesc.VertexSize;

					m_DeleteList.push_back(Data);
				}
				pModel->Get_MeshState(i).store(LOADSTATE::NOTLOADED);
			}
		}
		m_iSearchIndex++;
		iCheckCount++;
	}

}

HRESULT CModel_Manager::RegisterPrototype(const _char* pFilePath, CModel_Streaming* pModel)
{
	if (!pModel)
		CRASH("Failed");

	m_ModelPrototypes.emplace(pFilePath, pModel);
	Safe_AddRef(pModel);

	return S_OK;
}

void CModel_Manager::RequestData(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex)
{
	//여기에는 LOD가 안붙어있고 모델에는 붙어있음.

	atomic<LOADSTATE>& LoadState = pModel->Get_MeshState(iLODIndex);
	
	if (LoadState != LOADSTATE::NOTLOADED)
		return;

	if (iCurrentLoadCnt > 5)
		return;

	LOADSTATE ExpectedState = LOADSTATE::NOTLOADED;
	if (LoadState.compare_exchange_strong(ExpectedState, LOADSTATE::LOADING))
	{
		iCurrentLoadCnt++;
		//파일 경로 전체는 모델 매니저에 저장. 파일 이름(뒤에 LOD가 붙어야하니까)은 모델에 저장?
		m_pGameInstance->Add_Work([=, lModel = pModel, lFilePath = pFilePath, liLODIndex = iLODIndex, Matrix = XMLoadFloat4x4(&m_PreTransformMatrix)]() {
			LoadData(lModel, lFilePath, liLODIndex, Matrix);
			});
	}

	//게임이니셜라이즈 하기 전에 LOD3번은 전부 미리 만들어두라고 요청하는 함수 만들기.(내부에는 Wait걸고)
}

void CModel_Manager::SetUp_Data(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex)
{
	atomic<LOADSTATE>& LoadState = pModel->Get_MeshState(iLODIndex);

	if (LoadState != LOADSTATE::NOTLOADED)
		return;

	LOADSTATE ExpectedState = LOADSTATE::NOTLOADED;
	if (LoadState.compare_exchange_strong(ExpectedState, LOADSTATE::LOADING))
	{
		m_pGameInstance->Add_Work([=, lModel = pModel, lFilePath = pFilePath, liLODIndex = iLODIndex, Matrix = XMLoadFloat4x4(&m_PreTransformMatrix)]() {
			LoadData(lModel, lFilePath, liLODIndex, Matrix);
			});
	}
}

void CModel_Manager::LoadData(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex,_fmatrix PreMatrix)
{
	//pFilePath는 파일 경로 말고 _LOD까지 붙은거. 
	ifstream File(pFilePath + to_string(iLODIndex) + ".dat", ios::binary);
	if (!File.is_open())
		CRASH("Failed to Open File");

	_uint iNumMeshes = 0;
	File.read(reinterpret_cast<_char*>(&iNumMeshes), sizeof(_uint));

	if (iNumMeshes > 1000 || iNumMeshes == 0)
		CRASH("Invalid Mesh Count: Memory Corruption Suspected");

	MODEL_DATA Datas = Acquire_Vector();
	//MODEL_DATA Datas{};
	Datas.pModel = pModel;
	Datas.LoadData.resize(iNumMeshes);
	Datas.iLODIndex = iLODIndex;
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint iNumVertices = {};
		vector<VTXMESH>& pVertices = Datas.LoadData[i].VertexData;
		vector<_uint>& pIndices = Datas.LoadData[i].IndexData;
		_uint iNumMaterialIndex = {};
		File.read(reinterpret_cast<_char*>(&iNumVertices), sizeof(_uint));
		//pVertices = new VTXMESH[iNumVertices];

		File.read(reinterpret_cast<_char*>(&Datas.LoadData[i].iNumIndices), sizeof(_uint));
		Datas.LoadData[i].iNumIndices = Datas.LoadData[i].iNumIndices * 3;

		File.read(reinterpret_cast<_char*>(&iNumMaterialIndex), sizeof(_uint));

		Datas.LoadData[i].VertexData.resize(iNumVertices);
		File.read(reinterpret_cast<_char*>(Datas.LoadData[i].VertexData.data()), sizeof(VTXMESH) * iNumVertices);

		Datas.LoadData[i].IndexData.resize(Datas.LoadData[i].iNumIndices);
		File.read(reinterpret_cast<_char*>(Datas.LoadData[i].IndexData.data()), sizeof(_uint) * Datas.LoadData[i].iNumIndices);

		for (size_t j = 0; j < iNumVertices; ++j)
		{
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vPosition, XMVector3TransformCoord(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vPosition), PreMatrix));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vNormal, XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vNormal), PreMatrix));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vTangent, XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vTangent), PreMatrix));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vBinormal), PreMatrix));
		}
	}
	
	{
		lock_guard<mutex> lock(m_StagingMutex);
		m_StagingData.push_back(move(Datas));
	}
}

CModel_Manager::MODEL_DATA CModel_Manager::Acquire_Vector()
{
	{
		lock_guard<mutex> lock(m_DataPoolMutex);
		if (m_DataPool.empty())
			return MODEL_DATA();

		MODEL_DATA Data = move(m_DataPool.back());
		m_DataPool.pop_back();
		return Data;
	}
}

void CModel_Manager::Release_Vector(vector<MODEL_DATA>& data)
{
	for (auto& Data : data)
		Data.pModel = nullptr;

	{
		lock_guard<mutex> lock(m_DataPoolMutex);
		if (m_DataPool.empty())
			m_DataPool = move(data);
		else
			m_DataPool.insert(m_DataPool.end(), make_move_iterator(data.begin()), make_move_iterator(data.end()));
	}
}

void CModel_Manager::RenderBufferPool(_uint iLODIndex)
{
	m_pBufferPool[iLODIndex]->Bind_BufferPool();
	for (auto& pObject : m_RenderObjects[iLODIndex])
	{
		pObject->Render(m_pContext, iLODIndex);
		//호출된 시간을 밑으로 체크해야하는데 StaticObject에는 저 함수가 없어서 보류.
		pObject->Set_RenderTime(iLODIndex, m_fTotalPlayTime);
		Safe_Release(pObject);
	}
	m_RenderObjects[iLODIndex].clear();
}

void CModel_Manager::RenderBufferPool(_uint iThreadIndex, _uint iLODIndex, _uint iStartIndex, _uint iEndIndex, ID3D11DeviceContext* pContext)
{
	if (m_RenderObjects[iLODIndex].empty())
		return;
	for (_uint i = iStartIndex; i < iEndIndex; ++i)
	{
		m_RenderObjects[iLODIndex][i]->Render(pContext, iThreadIndex);
		m_RenderObjects[iLODIndex][i]->Set_RenderTime(iLODIndex, m_fTotalPlayTime);
		Safe_Release(m_RenderObjects[iLODIndex][i]);
	}
}

void CModel_Manager::Clear_BufferPool()
{
	for (auto& pRenderObjects : m_RenderObjects) {
		pRenderObjects.second.clear();
	}
}

_uint CModel_Manager::Render_ObjectsNum(_uint iLODIndex)
{
	if (iLODIndex >= 4)
		return 0;

	if (m_RenderObjects[iLODIndex].empty())
		return 0;
	return m_RenderObjects[iLODIndex].size();
}

void CModel_Manager::Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext** pDC, _uint iNumThread)
{
	for (_uint i = 0; i < iNumThread; ++i)
		m_pBufferPool[iLODIndex]->Bind_BufferPool(pDC[i]);
}

void CModel_Manager::Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext* pDC)
{
	m_pBufferPool[iLODIndex]->Bind_BufferPool(pDC);
}

void CModel_Manager::LoadLastLOD()
{
	for (auto& pModel : m_ModelPrototypes)
	{
		pModel.second->RequestModel();
		for (_uint i = 0; i < 4; ++i)
			pModel.second->Get_SharedBuffers(i, m_pBufferPool[i]->Get_VertexBuffer(), m_pBufferPool[i]->Get_IndexBuffer());
	}

	m_pGameInstance->Wait_Thread_End();
	for (auto& Data : m_StagingData)
	{
		//Data의 Data.LoadData 개수가 메쉬의 개수.
		vector< SHARED_DATA_DESC>* pData = Data.pModel->Get_MeshDesc(Data.iLODIndex);
		pData->clear();
		for (_uint i = 0; i < Data.LoadData.size(); ++i)
		{
			_uint VertexSize = Data.LoadData[i].VertexData.size() * sizeof(VTXMESH);
			_uint VertexOffset = m_pBufferPool[Data.iLODIndex]->Allocate_Vertex(VertexSize);

			if (VertexOffset == -1)
				CRASH("Failed");
			
			D3D11_MAPPED_SUBRESOURCE StagingDesc{};
			D3D11_BOX PoolBox{};
			PoolBox.left = VertexOffset;
			PoolBox.top = 0;
			PoolBox.front = 0;
			PoolBox.right = VertexOffset + VertexSize;
			PoolBox.bottom = 1;
			PoolBox.back = 1;

			m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(), 0, &PoolBox, Data.LoadData[i].VertexData.data(), 0, 0);
			//m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
			//memcpy(StagingDesc.pData, Data.LoadData[i].VertexData.data(), VertexSize);
			//m_pContext->Unmap(m_pStagingBuffer, 0);

			//PoolBox = { 0,0,0,VertexSize,1,1 };
			//m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(),
			//	0, VertexOffset, 0, 0, m_pStagingBuffer, 0, &PoolBox);


			_uint IndexSize = Data.LoadData[i].IndexData.size() * sizeof(_uint);
			_uint IndexOffSet = m_pBufferPool[Data.iLODIndex]->Allocate_Index(IndexSize);

			if (IndexOffSet == -1)
				CRASH("Failed");

			PoolBox.left = IndexOffSet;
			PoolBox.top = 0;
			PoolBox.front = 0;
			PoolBox.right = IndexOffSet + IndexSize;
			PoolBox.bottom = 1;
			PoolBox.back = 1;

			m_pContext->UpdateSubresource(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(), 0, &PoolBox, Data.LoadData[i].IndexData.data(), 0, 0);
			/*m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
			memcpy(StagingDesc.pData, Data.LoadData[i].IndexData.data(), IndexSize);
			m_pContext->Unmap(m_pStagingBuffer, 0);*/

			//PoolBox = { 0,0,0,IndexSize,1,1 };
			//m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(),
			//	0, IndexOffSet, 0, 0, m_pStagingBuffer, 0, &PoolBox);

			SHARED_DATA_DESC Desc{};
			Desc.IndexOffset = IndexOffSet / sizeof(_uint);
			Desc.IndexSize = IndexSize;
			Desc.NumIndices = Data.LoadData[i].iNumIndices;
			Desc.VertexOffset = VertexOffset / sizeof(VTXMESH);
			Desc.VertexSize = VertexSize;


			pData->push_back(Desc);
		}
		Data.pModel->Get_MeshState(Data.iLODIndex).store(LOADSTATE::LOADED);
	}
	Release_Vector(m_StagingData);
	m_StagingData.clear();
}

void CModel_Manager::Add_To_RenderTest(_uint iLODIndex, CStaticObject* pObject)
{
	{
		lock_guard<mutex> lock(m_RenderMutex);
		m_RenderObjects[iLODIndex].push_back(pObject);
		Safe_AddRef(pObject);
	}
}

void CModel_Manager::Add_To_RenderTest(vector<class CStaticObject*>* Container)
{
	{
		lock_guard<mutex> lock(m_RenderMutex);

		for (_uint i = 0; i < 4; ++i)
		{
			if (Container[i].empty())
				continue;
			for (auto& pObject : Container[i])
			{
				if (!pObject) continue;
				m_RenderObjects[i].push_back(pObject);
				Safe_AddRef(pObject);
			}
			//m_RenderObjects[i].insert(m_RenderObjects[i].end(), Container[i].begin(), Container[i].end());

			Container[i].clear();
		}
	}
}

CModel_Manager* CModel_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CModel_Manager* pInstance = new CModel_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Model_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel_Manager::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	for (_uint i = 0; i < 4; ++i)
		Safe_Release(m_pBufferPool[i]);
	Safe_Release(m_pStagingBuffer);

	for(auto& pPair: m_ModelPrototypes)
		Safe_Release(pPair.second);
	m_ModelPrototypes.clear();

	Safe_Release(m_pGameInstance);

	for (auto& Pair: m_RenderObjects)
		for (auto& pObject : Pair.second)
			Safe_Release(pObject);
	m_RenderObjects.clear();
}