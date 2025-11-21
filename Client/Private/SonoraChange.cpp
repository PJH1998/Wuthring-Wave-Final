#include "ClientPch.h"
#include "SonoraChange.h"

CSonoraChange::CSonoraChange(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CScreenEffect { pDevice, pContext }
{
}

CSonoraChange::CSonoraChange(const CSonoraChange& Prototype)
	: CScreenEffect { Prototype }
{
}

HRESULT CSonoraChange::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

    return S_OK;
}

HRESULT CSonoraChange::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_vRadialCenter = _float2(0.5f, 0.5f);

	m_vRadialDistanceRange = _float2(1.f, 0.1f);

	m_vRadialIntensityRange = _float2(0.f, -0.4f);

	m_vFadeColor = _float3(1.f, 1.f, 1.f);
	
	if (FAILED(Ready_Components()))
		return E_FAIL;

	Setting_Scale(m_vWinSize.x, m_vWinSize.y);
	Setting_Pos(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.5f);

	return S_OK;
}

void CSonoraChange::Priority_Update(_float fTimeDelta)
{
}

void CSonoraChange::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	_float fRadialRatio = SmoothStep(m_fRadialTime, m_vEffectTime.y, m_fCurrentTime);

	if(fRadialRatio > 0.f)
		m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::RADIAL);

	_float fRadialDistance = lerp(m_vRadialDistanceRange.x, m_vRadialDistanceRange.y, fRadialRatio);
	_float fRadialIntensity = lerp(m_vRadialIntensityRange.x, m_vRadialIntensityRange.y, fRadialRatio);

	m_fFadeIntensity = SmoothStep(m_fFadeTime, m_vEffectTime.y, m_fCurrentTime);

	m_pGameInstance->Setting_Radial(m_vRadialCenter, _float2(0.f, fRadialDistance), fRadialIntensity);
	
	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		m_pGameInstance->End_SFX();
	}
}

void CSonoraChange::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CSonoraChange::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return;

	m_pShader->Begin(0);
	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CSonoraChange::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;

	SONORA_CHANGE_DESC* pDesc = static_cast<SONORA_CHANGE_DESC*>(pArg);

	m_vEffectTime.x = 0.f;
	m_vEffectTime.y = pDesc->fEffectTime;
	m_fRadialTime = pDesc->fRadialTime;
	m_fFadeTime = pDesc->fFadeTime;
}

HRESULT CSonoraChange::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Sonora"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

HRESULT CSonoraChange::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if(FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");
	
	if (FAILED(m_pShader->Bind_Value("g_fIntensity", &m_fFadeIntensity, sizeof(_float))))
		CRASH("Failed to Bind Color");
	
	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vFadeColor, sizeof(_float3))))
		CRASH("Failed to Bind Color");

	return S_OK;
}

CSonoraChange* CSonoraChange::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSonoraChange* pInstance = new CSonoraChange(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CSonoraChange");
		Safe_Release(pInstance);
	}
    return pInstance;
}

CGameObject* CSonoraChange::Clone(void* pArg)
{
	CSonoraChange* pInstance = new CSonoraChange(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CSonoraChange");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSonoraChange::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
}
