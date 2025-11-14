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

	return S_OK;
}

void CSonoraChange::Priority_Update(_float fTimeDelta)
{
}

void CSonoraChange::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	_float fRadialRatio = SmoothStep(0.f, m_fRadialTime, m_fCurrentTime);

	_float fRadialDistance = floatlerp(m_vRadialDistanceRange.x, m_vRadialDistanceRange.y, fRadialRatio);
	_float fRadialIntensity = floatlerp(m_vRadialIntensityRange.x, m_vRadialIntensityRange.y, fRadialRatio);

}

void CSonoraChange::Late_Update(_float fTimeDelta)
{
}

void CSonoraChange::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return;

	m_pShader->Begin(ENUM_CLASS(SHADER_SCREENEFFECT::SONORA_CHANGE));
	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CSonoraChange::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_fCurrentTime = 0.f;

	SONORA_CHANGE_DESC* pDesc = static_cast<SONORA_CHANGE_DESC*>(pArg);

	m_fEffectTime = pDesc->fEffectTime;
	m_fRadialTime = pDesc->fRadialTime;
	m_fFadeTime = pDesc->fFadeTime;
}

HRESULT CSonoraChange::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if(FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vFadeColor, sizeof(_float4))))
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

}
