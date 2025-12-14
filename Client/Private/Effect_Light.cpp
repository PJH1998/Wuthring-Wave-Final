#include "ClientPch.h"
#include "Effect_Light.h"

CEffect_Light::CEffect_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Light::CEffect_Light(const CEffect_Light& Prototype)
    : CGameObject{ Prototype },
	m_tDesc {Prototype.m_tDesc}
{
}

HRESULT CEffect_Light::Initialize_Prototype(const LIGHT_DESC* pDesc)
{
	m_tDesc = *pDesc;
	
    return S_OK;
}

HRESULT CEffect_Light::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_vColor = m_tDesc.vColor;
    m_vLifeTime = m_tDesc.vLifeTime;
	m_wstrLightTag = m_tDesc.wstrLightTag;
	m_fSpeed = m_tDesc.fSpeed;
	m_vRange = m_tDesc.vRange;
	m_fAmbient = m_tDesc.fAmbient;


    m_isActivate = false;

    return S_OK;
}

void CEffect_Light::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Light::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	if (m_vLifeTime.x >= m_vLifeTime.y)
	{
		m_isActivate = false;
		m_pGameInstance->Set_LightActive(m_wstrLightTag, false);

		//초기화
		m_vLifeTime.x = 0.f;
		m_vRange.x = 0.f;

		return;
	}

	m_vLifeTime.x += fTimeDelta;

	Update_LightDesc(fTimeDelta);

	m_pGameInstance->Update_LightDesc(m_wstrLightTag, m_tLightDesc);
}

void CEffect_Light::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CEffect_Light::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	//초기화
	m_vLifeTime.x = 0.f;
	m_vRange.x = 0.f;

	if (m_isActivate = pDesc->IsActive)
	{
		_vector vScale = {};
		_vector vTrans = {};
		_vector vRot = {};
		XMMatrixDecompose(&vScale, &vRot, &vTrans, WorldMatrix);

		_float4 vPos = {};
		XMStoreFloat4(&vPos, vTrans);

		m_tLightDesc.eType = Engine::LIGHT_DESC::POINT;
		m_tLightDesc.vDiffuse = m_tDesc.vColor;
		m_tLightDesc.vPosition = vPos;
		m_tLightDesc.fRange = m_vRange.x;
		m_tLightDesc.vAmbient = _float4(m_fAmbient, m_fAmbient, m_fAmbient, 1.f);

		m_pGameInstance->Update_LightDesc(m_wstrLightTag, m_tLightDesc);
		m_pGameInstance->Set_LightActive(m_wstrLightTag, true);
	}
}

void CEffect_Light::Update_LightDesc(_float fTimeDelta)
{
	if (m_vRange.x < m_vRange.y)
	{
		//일단은 늘어나게만 나중에 늘거나 줄거나로 바꿔주자.
		//빛 갑자기 팍 꺼지는거 조금 어색한듯
		m_vRange.x += fTimeDelta * m_fSpeed;
		m_tLightDesc.fRange = m_vRange.x;
	}
}

CEffect_Light* CEffect_Light::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const LIGHT_DESC* pDesc)
{
    CEffect_Light* pInstance = new CEffect_Light(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pDesc)))
    {
        MSG_BOX("Failed to Created : CEffect_Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Light::Clone(void* pArg)
{
    CEffect_Light* pInstance = new CEffect_Light(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Light::Free()
{
    __super::Free();

}
