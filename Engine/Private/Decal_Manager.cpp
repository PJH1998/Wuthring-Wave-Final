#include "EnginePch.h"
#include "Decal_Manager.h"
#include "Shader.h"
#include "VIBuffer_Decal.h"
#include "Texture.h"
#include "GameInstance.h"
#include "Decal.h"

CDecal_Manager::CDecal_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
	, m_pGameInstance{ CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CDecal_Manager::Initialize()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CDecal_Manager::Update(_float fTimeDelta)
{
	for (auto& Pair : m_Decals)
		Pair.second->Update(fTimeDelta);
}

HRESULT CDecal_Manager::Add_DecalTexture(const _wstring& strDecalTag, const _tchar* pFilePath, TEXTURETYPE eTextureType)
{
	CDecal* pDecal = Find_Decal(strDecalTag);
	if (nullptr == pDecal)
	{
		pDecal = CDecal::Create(m_pDevice, m_pContext);
		ASSERT_CRASH(pDecal);

		m_Decals.emplace(strDecalTag, pDecal);
	}

	if (FAILED(pDecal->Add_DecalTexture(pFilePath, eTextureType)))
		CRASH("Failed Add DecalTexture");

	return S_OK;
}

HRESULT CDecal_Manager::Add_Decal(const _wstring& strDecalTag, const DECAL_DESC& Decal)
{
	CDecal* pDecal = Find_Decal(strDecalTag);
	ASSERT_CRASH(pDecal);

	pDecal->Add_DecalData(Decal);

	return S_OK;
}

HRESULT CDecal_Manager::Render()
{
	for (auto& Pair : m_Decals)
		Pair.second->Render(m_pShader);

	return S_OK;
}

HRESULT CDecal_Manager::Ready_Components()
{
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Decal.hlsl"), VTX_DECAL::Elements, VTX_DECAL::iNumElements);
	ASSERT_CRASH(m_pShader);

	return S_OK;
}

CDecal* CDecal_Manager::Find_Decal(const _wstring& strDecalTag)
{
	auto iter = m_Decals.find(strDecalTag);
	if(iter == m_Decals.end())
		return nullptr;
	return iter->second;
}


CDecal_Manager* CDecal_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDecal_Manager* pInstance = new CDecal_Manager(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CDecal_Manager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDecal_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	
	for (auto& Pair : m_Decals)
		Safe_Release(Pair.second);
	m_Decals.clear();
	
	Safe_Release(m_pShader);
}
