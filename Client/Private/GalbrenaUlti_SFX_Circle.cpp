#include "ClientPch.h"
#include "GalbrenaUlti_SFX_Circle.h"

CGalbrenaUlti_SFX_Circle::CGalbrenaUlti_SFX_Circle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect{ pDevice, pContext }
{
}

CGalbrenaUlti_SFX_Circle::CGalbrenaUlti_SFX_Circle(const CGalbrenaUlti_SFX_Circle& Prototype)
	: CScreenEffect{ Prototype }
	, m_vCenter{ Prototype.m_vCenter }
	, m_vScale{ Prototype.m_vScale }
	, m_vColor{ Prototype.m_vColor }
	, m_vSize{ Prototype.m_vSize }
	, m_fCircleRadius { Prototype.m_fCircleRadius }
	, m_fCircleWidth{ Prototype.m_fCircleWidth }
	, m_vUpdateTime{ Prototype.m_vUpdateTime }
{
}

HRESULT CGalbrenaUlti_SFX_Circle::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vCenter = _float2(m_vWinSize.x * 0.7f, m_vWinSize.y * 0.75f);

	m_vSize = _float2(300.f, 300.f);

	m_fCircleRadius = 110.f;
	m_fCircleWidth = 10.f; 

	m_vEffectTime = _float2(0.f, 0.5f);
	m_vUpdateTime = _float2(0.f, 0.5f);
	m_vScale = _float2(0.8f, 6.f);
	m_vColor = _float3(1.f, 1.f, 1.f);

	
	return S_OK;
}

HRESULT CGalbrenaUlti_SFX_Circle::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fCurrentScale = m_vScale.x;
	m_fCurrentRadius = m_fCircleRadius;
	m_vCurrentSize = m_vSize;

	return S_OK;
}

void CGalbrenaUlti_SFX_Circle::Priority_Update(_float fTimeDelta)
{
}

void CGalbrenaUlti_SFX_Circle::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		return;
	}

	_float fRatio = SmoothStep(m_vUpdateTime.x, m_vUpdateTime.y, m_fCurrentTime);

	m_fCurrentScale = lerp(m_vScale.x, m_vScale.y, fRatio);
}

void CGalbrenaUlti_SFX_Circle::Late_Update(_float fTimeDelta)
{
	m_fCurrentRadius = m_fCircleRadius * m_fCurrentScale;
	m_vCurrentSize = _float2(m_vSize.x * m_fCurrentScale, m_vSize.y * m_fCurrentScale);
	Setting_Scale(m_vCurrentSize.x, m_vCurrentSize.y);
	Setting_Pos(m_vCenter.x, m_vCenter.y);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CGalbrenaUlti_SFX_Circle::Render()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_fCircleRadius", &m_fCurrentRadius, sizeof(_float))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_fCircleWidth", &m_fCircleWidth, sizeof(_float))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_vScreenSize", &m_vCurrentSize, sizeof(_float2))))
		CRASH("Failed to Bind vColor");

	m_pShader->Begin(ENUM_CLASS(SHADER_SFX_BURST::GALBRENA_CIRCLE));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CGalbrenaUlti_SFX_Circle::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = m_vScale.x;
	m_fCurrentRadius = m_fCircleRadius;
	m_vCurrentSize = m_vSize;
}

HRESULT CGalbrenaUlti_SFX_Circle::Ready_Components()
{

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

CGalbrenaUlti_SFX_Circle* CGalbrenaUlti_SFX_Circle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrenaUlti_SFX_Circle* pInstance = new CGalbrenaUlti_SFX_Circle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrenaUlti_SFX_Circle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrenaUlti_SFX_Circle::Clone(void* pArg)
{
	CGalbrenaUlti_SFX_Circle* pInstance = new CGalbrenaUlti_SFX_Circle(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrenaUlti_SFX_Circle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrenaUlti_SFX_Circle::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
}
