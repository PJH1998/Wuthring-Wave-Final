#include "EnginePch.h"
#include "ScreenSpaceReflection.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "GameInstance.h"

CScreenSpaceReflection::CScreenSpaceReflection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSFX { pDevice, pContext }
{
}

HRESULT CScreenSpaceReflection::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_fMinStepSize = 5.f;
	m_fMaxStepSize = 20.f;
	m_fStartOffset = 10.f;

	return S_OK;
}

void CScreenSpaceReflection::Update(_float fTimeDelta)
{
}

HRESULT CScreenSpaceReflection::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	if (FAILED(pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Render Fail");

	if (FAILED(pShader->Bind_Texture("g_EnvMapTexture", m_pGameInstance->Get_EnvMap(0))))
		CRASH("Render Fail");

	if (FAILED(pShader->Bind_Value("g_fMinStepSize", &m_fMinStepSize, sizeof(_float))))
		CRASH("Failed Bind g_fMinStepSize");
	if (FAILED(pShader->Bind_Value("g_fMaxStepSize", &m_fMaxStepSize, sizeof(_float))))
		CRASH("Failed Bind g_fMaxStepSize");
	if (FAILED(pShader->Bind_Value("g_fStartOffset", &m_fStartOffset, sizeof(_float))))
		CRASH("Failed Bind g_fStartOffset");

	if (FAILED(pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::SSR))))
		CRASH("Render Fail")

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	return S_OK;
}

CScreenSpaceReflection* CScreenSpaceReflection::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CScreenSpaceReflection* pInstance = new CScreenSpaceReflection(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CScreenSpaceReflection");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CScreenSpaceReflection::Free()
{
	__super::Free();
}
