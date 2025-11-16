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
	return S_OK;
}

void CModel_Manager::Update(_float fTimeDelta)
{
	m_fTotalPlayTime += fTimeDelta;
	if (!m_StagingData.empty())
	{
		auto pTempVector = move(m_StagingData);
		m_StagingData.clear();

		/*iFrame++;
		if(iFrame>=iTestFrame)
			iFrame = 0;*/
		for (auto& Data : pTempVector)
		{
			//auto& Data = m_StagingData[m_StagingData.size()-1];
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
				m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
				memcpy(StagingDesc.pData, Data.LoadData[i].VertexData.data(), VertexSize);
				m_pContext->Unmap(m_pStagingBuffer, 0);

				D3D11_BOX PoolBox = { 0,0,0,VertexSize,1,1 };
				m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(),
					0, VertexOffset, 0, 0, m_pStagingBuffer, 0, &PoolBox);


				_uint IndexSize = Data.LoadData[i].IndexData.size() * sizeof(_uint);
				_uint IndexOffSet = m_pBufferPool[Data.iLODIndex]->Allocate_Index(IndexSize);

				if (IndexOffSet == -1)
					CRASH("Failed");

				m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
				memcpy(StagingDesc.pData, Data.LoadData[i].IndexData.data(), IndexSize);
				m_pContext->Unmap(m_pStagingBuffer, 0);

				PoolBox = { 0,0,0,IndexSize,1,1 };
				m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(),
					0, IndexOffSet, 0, 0, m_pStagingBuffer, 0, &PoolBox);

				SHARED_DATA_DESC Desc{};
				Desc.IndexOffset = IndexOffSet;
				Desc.IndexSize = IndexSize;
				Desc.NumIndices = Data.LoadData[i].iNumIndices;
				Desc.VertexOffset = VertexOffset;
				Desc.VertexSize = VertexSize;

				pData->push_back(Desc);
			}
			Data.pModel->Get_MeshState(Data.iLODIndex).store(LOADSTATE::LOADED);
		//m_StagingData.pop_back();
		}
		pTempVector.clear();
	}

	//지연 해제 하면 좋다고 함? 어떻게 하는지 몰라서 아직 내비두는 중 + 옥토트리 및 디퍼드 컨텍스트 적용 전.

	//이터레이터를 이동.
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
			if (pModel->Is_RenderTimeOver(i)&& pModel->Get_MeshState(i) == LOADSTATE::LOADED)
			{
				auto& MeshVector = pModel->Get_MeshDesc(0)[i];

				for (auto& pDesc : MeshVector)
				{
					m_pBufferPool[i]->FreeMemory_Vertex(pDesc.VertexOffset, pDesc.VertexSize);
					m_pBufferPool[i]->FreeMemory_Index(pDesc.IndexOffset, pDesc.IndexSize);
				}
				pModel->Get_MeshDesc(0)[i].clear();
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
	LOADSTATE ExpectedState = LOADSTATE::NOTLOADED;
	if (LoadState.compare_exchange_strong(ExpectedState, LOADSTATE::LOADING))
	{
		//파일 경로 전체는 모델 매니저에 저장. 파일 이름(뒤에 LOD가 붙어야하니까)은 모델에 저장?
		m_pGameInstance->Add_Work([=,lModel = pModel, lFilePath = pFilePath, liLODIndex = iLODIndex]() {
			LoadData(lModel, lFilePath, liLODIndex);
			});
	}

	//게임이니셜라이즈 하기 전에 LOD3번은 전부 미리 만들어두라고 요청하는 함수 만들기.(내부에는 Wait걸고)
}

void CModel_Manager::LoadData(CModel_Streaming* pModel,const _string& pFilePath, _uint iLODIndex)
{
	//pFilePath는 파일 경로 말고 _LOD까지 붙은거. 
	ifstream File(pFilePath  + to_string(iLODIndex) + ".dat", ios::binary);
	if (!File.is_open())
		CRASH("Failed");

	_uint iNumMeshes;
	File.read(reinterpret_cast<_char*>(&iNumMeshes), sizeof(_uint));
	
	MOEDL_DATA Datas;
	Datas.pModel = pModel;
	Datas.LoadData.resize(iNumMeshes);
	Datas.iLODIndex = iLODIndex;
	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint iNumVertices = {};
		vector<VTXMESH> pVertices = Datas.LoadData[i].VertexData;
		vector<_uint> pIndices = Datas.LoadData[i].IndexData;
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
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vPosition,	 XMVector3TransformCoord(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vPosition), XMLoadFloat4x4(&m_PreTransformMatrix)));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vNormal,		XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vNormal), XMLoadFloat4x4(&m_PreTransformMatrix)));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vTangent,	XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vTangent), XMLoadFloat4x4(&m_PreTransformMatrix)));
			XMStoreFloat3(&Datas.LoadData[i].VertexData[j].vBinormal,	XMVector3TransformNormal(XMLoadFloat3(&Datas.LoadData[i].VertexData[j].vBinormal), XMLoadFloat4x4(&m_PreTransformMatrix)));
		}
	}

		//여기서 스테이징 버퍼에 올리고 버퍼풀에 올려야함.
	{
		lock_guard<mutex> lock(m_Mutex);
		m_StagingData.push_back(move(Datas));
	}

}

void CModel_Manager::RenderBufferPool(_uint iLODIndex)
{
	m_pBufferPool[iLODIndex]->Bind_BufferPool();
	for (auto& pObject : m_RenderObjects[iLODIndex])
	{
		pObject->Render(m_pContext, iLODIndex);
		//호출된 시간을 밑으로 체크해야하는데 StaticObject에는 저 함수가 없어서 보류.
		//pObject->Set_RenderTime(iLODIndex, m_fTotalPlayTime);
		Safe_Release(pObject);
	}
	m_RenderObjects[iLODIndex].clear();
}

void CModel_Manager::LoadLastLOD()
{
	for (auto& pModel : m_ModelPrototypes)
	{
		pModel.second->RequestLastLODModel();
		for (_uint i = 0; i < 4; ++i)
			pModel.second->Get_SharedBuffers(i, m_pBufferPool[i]->Get_VertexBuffer(), m_pBufferPool[i]->Get_IndexBuffer());
	}

	m_pGameInstance->Wait_Thread_End();
	for (auto& Data : m_StagingData)
	{
		Data.pModel;
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
			m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
			memcpy(StagingDesc.pData, Data.LoadData[i].VertexData.data(), VertexSize);
			m_pContext->Unmap(m_pStagingBuffer, 0);

			D3D11_BOX PoolBox = { 0,0,0,VertexSize,1,1 };
			m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_VertexBuffer(),
				0, VertexOffset, 0, 0, m_pStagingBuffer, 0, &PoolBox);


			_uint IndexSize = Data.LoadData[i].IndexData.size() * sizeof(_uint);
			_uint IndexOffSet = m_pBufferPool[Data.iLODIndex]->Allocate_Index(IndexSize);

			if (IndexOffSet == -1)
				CRASH("Failed");

			m_pContext->Map(m_pStagingBuffer, 0, D3D11_MAP_WRITE, 0, &StagingDesc);
			memcpy(StagingDesc.pData, Data.LoadData[i].IndexData.data(), IndexSize);
			m_pContext->Unmap(m_pStagingBuffer, 0);

			PoolBox = { 0,0,0,IndexSize,1,1 };
			m_pContext->CopySubresourceRegion(m_pBufferPool[Data.iLODIndex]->Get_IndexBuffer(),
				0, IndexOffSet, 0, 0, m_pStagingBuffer, 0, &PoolBox);

			SHARED_DATA_DESC Desc{};
			Desc.IndexOffset = IndexOffSet;
			Desc.IndexSize = IndexSize;
			Desc.NumIndices = Data.LoadData[i].iNumIndices;
			Desc.VertexOffset = VertexOffset;
			Desc.VertexSize = VertexSize;

			pData->push_back(Desc);
		}
		Data.pModel->Get_MeshState(Data.iLODIndex).store(LOADSTATE::LOADED);
	}
	m_StagingData.clear();
}

void CModel_Manager::Add_To_RenderTest(_uint iLODIndex, CStaticObject* pObject)
{
	m_RenderObjects[iLODIndex].push_back(pObject);
	Safe_AddRef(pObject);
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