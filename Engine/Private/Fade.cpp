#include "EnginePch.h"
#include "Fade.h"

#include "VIBuffer_Rect.h"
#include "Shader.h"

CFade::CFade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CFade::OnFade(FADE eFade, _float fDuration, function<void()> func)
{
	FADE_DESC FadeDesc = {};
	FadeDesc.eFade = eFade;
	FadeDesc.fDuration = fDuration;
	FadeDesc.Func = func;
	m_Fades.push(FadeDesc);
}

HRESULT CFade::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	// VIBuffer
	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pVIBuffer);

	// Shader
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Fade.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	ASSERT_CRASH(m_pShader);

	// Matrix SetUp
	_float fWinSizeX = static_cast<_float>(iWinSizeX);
	_float fWinSizeY = static_cast<_float>(iWinSizeY);

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(fWinSizeX, fWinSizeY, 1.f));// *XMMatrixTranslation(fWinSizeX * 0.5f, fWinSizeY * 0.5f, 0.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(fWinSizeX, fWinSizeY, 0.f, 1.f));

	return S_OK;
}

void CFade::Update(_float fTimeDelta)
{
	if (m_fTimeAcc >= m_fDuration)
	{
		if (false == m_Fades.empty())
		{
			FADE_DESC FadeDesc = m_Fades.front();
			m_eFade = FadeDesc.eFade;
			m_fDuration = FadeDesc.fDuration;
			m_Func = FadeDesc.Func;
			m_isFade = true;
		}
		else
			m_isFade = false;

		m_fTimeAcc = 0.f;
		return;
	}

	m_fTimeAcc += fTimeDelta;

	// Fade End
	if (m_fTimeAcc >= m_fDuration)
	{
		if(nullptr != m_Func)
			m_Func();
		m_fDuration = 0.f;
		m_Fades.pop();
	}
}

void CFade::Render()
{
	if (false == m_isFade)
		return;

	Bind_Resource();

	m_pShader->Begin(ENUM_CLASS(m_eFade));
	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CFade::Bind_Resource()
{
	m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix);
	m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
	m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);

	_float fTimeRate = m_fTimeAcc / m_fDuration;
	m_pShader->Bind_Value("g_fTimeRate", &fTimeRate, sizeof(_float));
}

CFade* CFade::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CFade* pInstance = new CFade(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
		CRASH("Fade");

	return pInstance;
}

void CFade::Free()
{
	__super::Free();

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
