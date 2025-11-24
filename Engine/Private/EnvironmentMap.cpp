#include "EnginePch.h"
#include "EnvironmentMap.h"
#include "GameInstance.h"
#include "Probe.h"

CEnvironmentMap::CEnvironmentMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CEnvironmentMap::Initialize()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(static_cast<_float>(g_iEnvMapSize), static_cast<_float>(g_iEnvMapSize), 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(static_cast<_float>(g_iEnvMapSize), static_cast<_float>(g_iEnvMapSize), 0.f, 1.f));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_RT()))
		return E_FAIL;

	if (FAILED(Ready_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnvironmentMap::Add_Probe(_float3 vCenter, _float fRange)
{
	CProbe* pProbe = CProbe::Create(m_pDevice, m_pContext, vCenter, fRange);
	ASSERT_CRASH(pProbe);

	m_Probes.push_back(pProbe);

	return S_OK;
}

void CEnvironmentMap::Bake_EnvMaps()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		CRASH("Failed Bind WorldMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed Bind ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed Bind ProjMatrix");

	for (auto& pProbe : m_Probes)
		pProbe->Render(m_pShader, m_pVIBuffer_Rect, m_iIndex++);

}

ID3D11ShaderResourceView* CEnvironmentMap::Get_EnvMap(_uint iIndex)
{
	if (iIndex >= m_Probes.size())
		return nullptr;

	return m_Probes[iIndex]->Get_EnvMap();
}

void CEnvironmentMap::Add_EnvMap_SkyBox(CGameObject* pSkyBox)
{
	for (auto& pProbe : m_Probes)
		pProbe->Add_SkyBox(pSkyBox);
}

void CEnvironmentMap::Add_EnvMap_StaticObject(CStaticObject* pStaticObject)
{
	for (auto& pProbe : m_Probes)
		pProbe->Add_StaticObject(pStaticObject);
}

void CEnvironmentMap::Clear()
{
	for (auto& pProbe : m_Probes)
		Safe_Release(pProbe);
	m_Probes.clear();
}

HRESULT CEnvironmentMap::Ready_Components()
{
	m_pVIBuffer_Rect = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer_Rect)
		CRASH("Buffer Fail");

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_EnvMapDeferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		CRASH("Shader Fail");

	return S_OK;
}


HRESULT CEnvironmentMap::Ready_MRT()
{
#pragma region MRT_OBJECT    
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvDiffuse"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvNormal"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvDepth"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvEmissive"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvDistortion"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvPBR"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EnvObject"), TEXT("RT_EnvSSS"))))
		ASSERT_CRASH(false);
#pragma endregion

	return S_OK;
}

HRESULT CEnvironmentMap::Ready_RT()
{
	/* RenderTarget Diffuse */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvDiffuse"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 0.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Normal */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvNormal"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Depth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvDepth"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Emissive*/
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvEmissive"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Distortion */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvDistortion"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Metaliic */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvPBR"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget SSS */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_EnvSSS"), g_iEnvMapSize, g_iEnvMapSize, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	return S_OK;
}


CEnvironmentMap* CEnvironmentMap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvironmentMap* pInstance = new CEnvironmentMap(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CEnvironmentMap");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEnvironmentMap::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Clear();

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer_Rect);
}
