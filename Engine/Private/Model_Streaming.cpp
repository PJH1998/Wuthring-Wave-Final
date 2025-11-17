#include"EnginePch.h"
#include"Model_Streaming.h"
#include"MeshMaterial.h"
#include"GameInstance.h"

CModel_Streaming::CModel_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent(pDevice, pContext)
{

}

CModel_Streaming::CModel_Streaming(const CModel_Streaming& Prototype)
	:CComponent(Prototype),
	m_pModelPrototype{Prototype.m_pModelPrototype},
	m_iNumMaterials{ Prototype.m_iNumMaterials }
	, m_iMaxLOD{ Prototype.m_iMaxLOD },
	m_ModelPath{ Prototype.m_ModelPath },
	m_Materials{ Prototype.m_Materials }
{
	for (_uint i = 0; i < 4; ++i)
		m_iNumMeshes[i] = Prototype.m_iNumMeshes[i];

	for (_uint i = 0; i < 4; ++i)
		if (Prototype.m_Meshes[i])
			m_Meshes[i] = dynamic_cast<CMesh_Streaming*>(Prototype.m_Meshes[i]->Clone(nullptr));

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);
	Safe_AddRef(m_pModelPrototype);
}

HRESULT CModel_Streaming::Initialize_Prototype(const _char* pFilePath)
{
	m_pGameInstance->RegisterPrototype(pFilePath, this);
	Ready_Mesh(pFilePath);
	Ready_Material();
	//정말 마음에 안드는 코드. 추후 수정 하고싶음
	m_pModelPrototype = this;
	return S_OK;
}

HRESULT CModel_Streaming::Initialize_Clone(void* pArg)
{
	return S_OK;
}

HRESULT CModel_Streaming::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return S_OK;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel_Streaming::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return S_OK;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType);
}

HRESULT CModel_Streaming::Render(_uint iLODIndex, _uint iMeshIndex)
{
	if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::LOADED)
	{
		//m_Meshes[iLODIndex]->Bind_Resources(iMeshIndex);
		m_fRenderTime[iLODIndex] = m_pGameInstance->Get_TimeDelta(TEXT("Timer_Default"));
		m_pModelPrototype->m_Meshes[iLODIndex]->Render(iMeshIndex);
		return S_OK;
	}
	else if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::NOTLOADED)
		m_pGameInstance->RequestData(this, m_ModelPath, iLODIndex);
	//모델 매니저에 해당하는 LOD단계 요청할것.
	//m_pGameInstance
	//준비된 것 중 제일 높은 LOD 단계 렌더시키기.


	//LOD단계가 준비가 안돼있으면 어쩔 수 없이 버퍼를 다시 바인딩 하므로 렉이 살짝 먹을듯? 아니면 Real_Late_Renedr 함수를 만들어서 늦어진 친구들을 다시 렌더하는 걸 만들어?
	_uint RenderLOD = m_iMaxLOD;
	for (_uint i = 0; i < m_iMaxLOD - 1; ++i)
	{
		if (m_pModelPrototype->m_Meshes[i]->IsLoaded() == LOADSTATE::LOADED)
		{
			RenderLOD = i;
			break;
		}
	}
	if (m_pModelPrototype->m_Meshes[RenderLOD]->Is_Overed(iMeshIndex))
		return S_OK;
	m_fRenderTime[RenderLOD] = m_pGameInstance->Get_TimeDelta(TEXT("Timer_Default"));
	m_pModelPrototype->m_Meshes[RenderLOD]->Bind_Resources(iMeshIndex);
	m_pModelPrototype->m_Meshes[RenderLOD]->Render(iMeshIndex);
	return S_OK;
}

HRESULT CModel_Streaming::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return E_FAIL;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex, pEffect);
}

HRESULT CModel_Streaming::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return S_OK;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType, pEffect);
}

HRESULT CModel_Streaming::Render(_uint iLODIndex, _uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::LOADED)
	{
		m_fRenderTime[iLODIndex] = m_pGameInstance->Get_TimeDelta(TEXT("Timer_Default"));
		m_Meshes[iLODIndex]->Render(iMeshIndex, pDC);
		return S_OK;
	}
	else if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::NOTLOADED)
		m_pGameInstance->RequestData(this, m_ModelPath, iLODIndex);

	_uint RenderLOD = m_iMaxLOD;
	for (_uint i = 0; i < m_iMaxLOD - 1; ++i)
	{
		if (m_pModelPrototype->m_Meshes[i]->IsLoaded() == LOADSTATE::LOADED)
		{
			RenderLOD = i;
			break;
		}
	}

	if (m_pModelPrototype->m_Meshes[RenderLOD]->Is_Overed(iMeshIndex))
		return S_OK;

	m_fRenderTime[RenderLOD] = m_pGameInstance->Get_TimeDelta(TEXT("Timer_Default"));
	m_pModelPrototype->m_Meshes[RenderLOD]->Bind_Resources(iMeshIndex, pDC);
	m_Meshes[m_iMaxLOD]->Render(iMeshIndex, pDC);
	return S_OK;
}

void CModel_Streaming::Ready_BoundingBox(_float* pMinPos, _float* pMaxPos)
{

}

void CModel_Streaming::RequestLastLODModel()
{
	--m_iMaxLOD;

	size_t lastDotPos = m_ModelPath.find_last_of('.');

	// 2. '.'을 찾았는지, 그리고 '.'이 경로의 맨 앞이 아닌지 확인합니다.
	// (e.g., ".config" 같은 숨김 파일을 방지)
	if (lastDotPos != std::string::npos && lastDotPos > 0)
	{
		// 3. '.' 위치 "앞까지"의 문자열만 잘라서(substr) 다시 저장합니다.
		m_ModelPath = m_ModelPath.substr(0, lastDotPos);
	}
	m_ModelPath.pop_back();

	m_pGameInstance->RequestData(this, m_ModelPath, m_iMaxLOD);
}

_bool CModel_Streaming::Is_RenderTimeOver(_uint iLODIndex)
{
	if (iLODIndex >= m_iMaxLOD)
		return false;

	return  m_pGameInstance->Get_TimeDelta(TEXT("Timer_Default")) - m_fRenderTime[iLODIndex] >= 5.f;
}

HRESULT CModel_Streaming::Ready_Mesh(const _char* pFilePath)
{
	//Dat 파일을 읽는 게 아니라 경로를 읽고 내부 데이터를 읽어야함.
	_bool IsNameSave = { true };
	_string LastModelPath;
	for (const auto& entry : filesystem::directory_iterator(pFilePath)) {
		if (m_iMaxLOD >= 4)
			CRASH("??");

		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() != ".dat")
			continue;

		//_string DatFile = entry.path().string();

		ifstream File(entry.path(), ios::binary);
		if (!File.is_open())
			CRASH("Failed");
		File.read(reinterpret_cast<_char*>(&m_iNumMeshes[m_iMaxLOD]), sizeof(_uint));
		File.close();
		m_Meshes[m_iMaxLOD] = CMesh_Streaming::Create(m_pDevice, m_pContext, m_iNumMeshes[m_iMaxLOD]);

		if (!m_Meshes[m_iMaxLOD++])
			CRASH("Failed");
		
		if (IsNameSave)
			LastModelPath = m_ModelPath = entry.path().string();
	}
	return S_OK;
}

HRESULT CModel_Streaming::Ready_Material()
{
	_char szMaterialFilePath[MAX_PATH] = {};
	_char szMaterialDrivePath[MAX_PATH] = {};
	_char szMaterialDirPath[MAX_PATH] = {};
	_char szMaterialFileName[MAX_PATH] = {};
	_splitpath_s(m_ModelPath.c_str(), szMaterialDrivePath, MAX_PATH, szMaterialDirPath, MAX_PATH, szMaterialFileName, MAX_PATH, nullptr, 0);

	strcpy_s(szMaterialFilePath, szMaterialDrivePath);
	strcat_s(szMaterialFilePath, szMaterialDirPath);
	strcat_s(szMaterialFilePath, "Mat/");
	strcat_s(szMaterialFilePath, szMaterialFileName);
	strcat_s(szMaterialFilePath, ".json");

	ifstream MaterialFile(szMaterialFilePath);
	if (false == MaterialFile.is_open())
	{
		MSG_BOX("Failed Open : Material");
		return E_FAIL;
	}

	json MaterialsData;
	MaterialFile >> MaterialsData;

	m_iNumMaterials = MaterialsData["NumMaterial"];

	for (auto& MaterialData : MaterialsData["Materials"])
	{
		CMeshMaterial* pMeshMaterial = CMeshMaterial::Create(m_pDevice, m_pContext, szMaterialFilePath, MaterialData);
		if (nullptr == pMeshMaterial)
			return E_FAIL;

		m_Materials.push_back(pMeshMaterial);
	}

	MaterialFile.close();

	return S_OK;
}

void CModel_Streaming::PlusRenderdTime(_float fTimeDelta)
{
	m_fRenderTime[0] += fTimeDelta;
	m_fRenderTime[1] += fTimeDelta;
	m_fRenderTime[2] += fTimeDelta;
	m_fRenderTime[3] += fTimeDelta;
}

HRESULT CModel_Streaming::Get_SharedBuffers(_uint iLODIndex, ID3D11Buffer* pVertex, ID3D11Buffer* pIndex)
{
	if (m_Meshes[iLODIndex])
		m_Meshes[iLODIndex]->Set_Buffers(pVertex, pIndex);
	return S_OK;
}

_bool CModel_Streaming::Is_Overed(_uint iLODIndex, _uint iMeshIndex)
{
	return m_Meshes[iLODIndex]->Is_Overed(iMeshIndex);
}

CModel_Streaming* CModel_Streaming::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath)
{
	CModel_Streaming* pInstance = new CModel_Streaming(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pFilePath)))
	{
		MSG_BOX("Failed to Create : Model_Streaming");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel_Streaming::Clone(void* pArg)
{
	CModel_Streaming* pInstance = new CModel_Streaming(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model_Streaming (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel_Streaming::Free()
{
	__super::Free();
	if (m_isClone)
		Safe_Release(m_pModelPrototype);
	m_pModelPrototype = nullptr;
	for (_uint i = 0; i < 4; ++i)
		if (m_Meshes[i])
			Safe_Release(m_Meshes[i]);

	for (auto& pMat : m_Materials)
		Safe_Release(pMat);
	m_Materials.clear();
}
