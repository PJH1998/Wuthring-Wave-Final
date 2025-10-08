#include"EnginePch.h"
#include"Model_Instance.h"
#include"MeshMaterial.h"
#include"Mesh_Instance.h"

CModel_Instance::CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CComponent(pDevice,pContext)
{
}

CModel_Instance::CModel_Instance(const CModel_Instance& Prototype)
    :CComponent(Prototype),
	m_eType{ Prototype.m_eType },
	m_iNumMeshes{ Prototype.m_iNumMeshes },
	m_Meshes{ Prototype.m_Meshes },
	m_iNumMaterials{ Prototype.m_iNumMaterials },
	m_Materials{ Prototype.m_Materials },
	m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
{
	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);
        
}

void CModel_Instance::Sync_RootNode(CTransform* pOwnerTransform, CNavigation* pOwnerNavigation, _float fTimeDelta)
{
}

void CModel_Instance::Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions)
{
}

HRESULT CModel_Instance::Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
    m_eType = MODELTYPE::NONANIM;
	
	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	ifstream InputFile(pFilePath, ios::binary);
	if (false == InputFile.is_open())
	{
		MSG_BOX("Failed Open : Model");
		return E_FAIL;
	}

	if (FAILED(Ready_Mesh(InputFile)))
		return E_FAIL;

	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;

	InputFile.close();

    return S_OK;
}

HRESULT CModel_Instance::Initialize_Clone(void* pArg)
{
	//임시
	for (auto& pMesh : m_Meshes)
		pMesh->Initialize_Clone(pArg);

    return S_OK;
}

HRESULT CModel_Instance::Render(_uint iMeshIndex)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
		return E_FAIL;
	
	m_Meshes[iMeshIndex]->Render();
	return S_OK;
}

_bool CModel_Instance::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
    return _bool();
}

HRESULT CModel_Instance::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel_Instance::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType);

}

HRESULT CModel_Instance::Ready_Mesh(ifstream& InputFile)
{
	InputFile.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_iNumMeshes; ++i)
	{
		CMesh_Instance* pMesh = CMesh_Instance::Create(m_pDevice, m_pContext, XMLoadFloat4x4(&m_PreTransformMatrix), InputFile);

		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel_Instance::Ready_Material(const _char* pFilePath)
{
	_char szMaterialFilePath[MAX_PATH] = {};
	_char szMaterialDrivePath[MAX_PATH] = {};
	_char szMaterialDirPath[MAX_PATH] = {};
	_char szMaterialFileName[MAX_PATH] = {};
	_splitpath_s(pFilePath, szMaterialDrivePath, MAX_PATH, szMaterialDirPath, MAX_PATH, szMaterialFileName, MAX_PATH, nullptr, 0);

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

CModel_Instance* CModel_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, const _char* pFilePath)
{
	CModel_Instance* pInstance = new CModel_Instance(pDevice, pContext);

	//임시. 나중에 애님 모델도 메쉬 인스턴싱이 필요할 경우 대비 없애진 않음.
	MODELTYPE eType = MODELTYPE::NONANIM;

	if (FAILED(pInstance->Initialize_Prototype(eType, PreTransformMatrix, pFilePath)))
	{
		MSG_BOX("Failed to Create : Model_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel_Instance::Clone(void* pArg)
{
	CModel_Instance* pClone = new CModel_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model_Instance (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CModel_Instance::Free()
{
	__super::Free();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);

	m_Materials.clear();
}
