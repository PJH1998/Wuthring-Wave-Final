#include"EnginePch.h"
#include"Model_Streaming.h"
#include"MeshMaterial.h"

CModel_Streaming::CModel_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent(pDevice, pContext)
{
}

CModel_Streaming::CModel_Streaming(const CModel_Streaming& Prototype)
	:CComponent(Prototype)
{

	for (_uint i = 0; i < 4; ++i)
		m_Meshes[i] = dynamic_cast<CMesh_Streaming*>(Prototype.m_Meshes[i]->Clone(nullptr));

	for (_uint i = 0; i < 4; ++i)
		if (m_Meshes[i])
			Safe_AddRef(m_Meshes[i]);
}

HRESULT CModel_Streaming::Initialize_Prototype(const _char* pFilePath)
{
	Ready_Mesh(pFilePath);
	Ready_Material(pFilePath);
	return S_OK;
}

HRESULT CModel_Streaming::Initialize_Clone(void* pArg)
{
	return S_OK;
}


HRESULT CModel_Streaming::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	return S_OK;
}

HRESULT CModel_Streaming::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	return S_OK;
}

HRESULT CModel_Streaming::Render(_uint iLODIndex, _uint iMeshIndex)
{

	if (m_Meshes[iLODIndex]->IsLoaded() == LOADSTATE::LOADED)
	{
		m_Meshes[iLODIndex]->Render(iMeshIndex);
		return S_OK;
	}
	else if (m_Meshes[iLODIndex]->IsLoaded() == LOADSTATE::LOADING)
	{
		//요청
	}
	//모델 매니저에 해당하는 LOD단계 요청할것.
	//m_pGameInstance
	//제일 높은 LOD 단계 렌더시키기.

	m_Meshes[m_iMaxLOD]->Render(iMeshIndex);
	return S_OK;
}

HRESULT CModel_Streaming::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	return S_OK;
}

HRESULT CModel_Streaming::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	return S_OK;
}

HRESULT CModel_Streaming::Render(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	return S_OK;
}

void CModel_Streaming::Ready_BoundingBox(_float* pMinPos, _float* pMaxPos)
{

}

atomic<LOADSTATE>& CModel_Streaming::Get_MeshState(_uint iLODIndex)
{
	if (iLODIndex >= m_iMaxLOD)
		CRASH("Failed");

	return m_Meshes[iLODIndex]->IsLoaded();
}

HRESULT CModel_Streaming::Ready_Mesh(const _char* pFilePath)
{
	//Dat 파일을 읽는 게 아니라 경로를 읽고 내부 데이터를 읽어야함.
	_bool IsNameSave = { true };
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
			m_ModelPath = entry.path().stem().string();
	}

	m_ModelPath.pop_back();
	
	return S_OK;
}

HRESULT CModel_Streaming::Ready_Material(const _char* pFilePath)
{
	//LOD 0번만 하게 합시다.
	return S_OK;
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

	for (_uint i = 0; i < 4; ++i)
		if (m_Meshes[i])
			Safe_Release(m_Meshes[i]);

	for (auto& pMat : m_Materials)
		Safe_Release(pMat);
	m_Materials.clear();
}
