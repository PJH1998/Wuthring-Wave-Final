#include"EnginePch.h"
#include"Model_Streaming.h"
#include"MeshMaterial.h"
#include"GameInstance.h"
#include"Material.h"

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
	if (iMeshIndex >= m_pModelPrototype->m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return S_OK;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel_Streaming::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_pModelPrototype->m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return S_OK;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType);
}

HRESULT CModel_Streaming::Render(_uint iLODIndex, _uint iMeshIndex)
{
	if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::LOADED)
	{
		m_pModelPrototype->m_Meshes[iLODIndex]->Render(iMeshIndex);
		return S_OK;
	}
	else if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::NOTLOADED)
		m_pGameInstance->RequestData(this, m_pModelPrototype->m_ModelPath, iLODIndex);
	return S_OK;
}

HRESULT CModel_Streaming::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_pModelPrototype->m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return E_FAIL;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex, pEffect);
}

HRESULT CModel_Streaming::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_pModelPrototype->m_Meshes[iLODIndex]->Get_MeshDesc()->size())
		return S_OK;

	return m_Materials[iMeshIndex]->Bind_Resource(pShader, pConstantName, eTextureType, pEffect);
}

HRESULT CModel_Streaming::Render(_uint iLODIndex, _uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (!m_pModelPrototype->m_Meshes[iLODIndex])
		return S_OK;
	if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::LOADED)
	{
		m_pModelPrototype->m_Meshes[iLODIndex]->Render(iMeshIndex, pDC);
		return S_OK;
	}
	else if (m_pModelPrototype->Get_MeshState(iLODIndex) == LOADSTATE::NOTLOADED)
		m_pGameInstance->RequestData(this, m_pModelPrototype->m_ModelPath, iLODIndex);

	//if (m_pModelPrototype->m_Meshes[m_iMaxLOD]->Is_Overed(iMeshIndex))
	//	return S_OK;

	//m_pModelPrototype->m_Meshes[m_iMaxLOD]->Bind_Resources(iMeshIndex, pDC);
	//m_pModelPrototype->m_Meshes[m_iMaxLOD]->Render(iMeshIndex, pDC);
	return S_OK;
}

void CModel_Streaming::Ready_BoundingBox(_float* pMinPos, _float* pMaxPos)
{

}

HRESULT CModel_Streaming::Bind_Buffer(ID3D11DeviceContext* pDeferredContext, _uint iLODIndex)
{
	return m_pModelPrototype->m_Meshes[iLODIndex]->Bind_Resources(0, pDeferredContext);
}

vector<CModel_Manager::SHARED_DATA_DESC>* CModel_Streaming::Get_MeshDesc(_uint iLODIndex)
{
	auto vector = m_pModelPrototype->m_Meshes[iLODIndex]->Get_MeshDesc();

	return vector;
}

void CModel_Streaming::RequestModel(_uint iLODIndex)
{
	m_pGameInstance->SetUp_Data(this, m_ModelPath, 0);
	m_pGameInstance->SetUp_Data(this, m_ModelPath, m_iMaxLOD);
}

_bool CModel_Streaming::Is_RenderTimeOver(_uint iLODIndex)
{
	if (iLODIndex >= m_iMaxLOD || !m_isClone)
		return false;

	return  m_pGameInstance->Get_PlayTime() - m_pModelPrototype->m_fRenderTime[iLODIndex] >= 5.f;
}

HRESULT CModel_Streaming::Ready_Mesh(const _char* pFilePath)
{
	//Dat 파일을 읽는 게 아니라 경로를 읽고 내부 데이터를 읽어야함.
	_bool IsNameSave = { true };
	_string LastModelPath;
	for (const auto& entry : filesystem::directory_iterator(pFilePath)) {

		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() != ".dat")
			continue;

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
	--m_iMaxLOD;

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
		CMaterial* pMeshMaterial = CMaterial::Create(m_pDevice, m_pContext, MaterialData);
		if (nullptr == pMeshMaterial)
			return E_FAIL;

		m_Materials.push_back(pMeshMaterial);
	}

	MaterialFile.close();

	if (!m_ModelPath.empty())
	{
		size_t lastDotPos = m_ModelPath.find_last_of('.');
		if (lastDotPos != std::string::npos)
			m_ModelPath = m_ModelPath.substr(0, lastDotPos);

		if (!m_ModelPath.empty())
			m_ModelPath.pop_back();
	}
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

_uint CModel_Streaming::Get_LastLODIndex()
{
	for (_uint i = 0; i < 4; ++i)
	{
		if (!m_pModelPrototype->m_Meshes[i])
			return i - 1;
	}
	return m_pModelPrototype->m_iMaxLOD;
}

void CModel_Streaming::Request_LOD(_uint iLODIndex)
{
	m_pGameInstance->RequestData(this, m_pModelPrototype->m_ModelPath, iLODIndex);
}

_uint CModel_Streaming::Get_ReadyLOD()
{
	_uint RenderLOD = m_pModelPrototype->m_iMaxLOD;
	for (_uint i = 0; i < m_pModelPrototype->m_iMaxLOD - 1; ++i)
	{
		if (m_pModelPrototype->m_Meshes[i]->IsLoaded() == LOADSTATE::LOADED)
		{
			RenderLOD = i;
			break;
		}
	}
	return RenderLOD;
}

void CModel_Streaming::Set_RigidData(vector<_float3>& vecVertexPos, vector<_uint>& vecIndices, _uint iMeshIndex)
{
	m_pModelPrototype->m_Meshes[0]->Set_RigidData(vecVertexPos, vecIndices, iMeshIndex);
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
