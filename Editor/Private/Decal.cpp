#include "Editorpch.h"
#include "Decal.h"

CDecal::CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CDecal::CDecal(const CDecal& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CDecal::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDecal::Initialize_Clone(void* pArg)
{
	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_vColor = pDesc->vColor;
    m_LifeTime = pDesc->LifeTime;
	m_wstrMyTag = pDesc->strMyTag;


   // m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    //처음 만들어질 땐 무조건 활성화 ?
    m_isActivate = false;

    return S_OK;
}

void CDecal::Priority_Update(_float fTimeDelta)
{
}

void CDecal::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	DECAL_DATA Desc{};
	Desc.eType = DECAL_DATA::STATIC;
	Desc.fLifeTime = m_LifeTime;
	Desc.vColor = m_vColor;
	Desc.WorldMatrix = m_pTransformCom->Get_WorldMatrix();

	m_pGameInstance->Add_DecalData(m_wstrMyTag, Desc);

	m_isActivate = false;
}

void CDecal::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CDecal::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
    if(_bool* IsActivate = static_cast<_bool*>(pArg))
        m_isActivate = *IsActivate;

     Root_Transform(WorldMatrix);
}

void CDecal::Root_Transform(_fmatrix WorldMatrix)
{
    _vector vPos =  XMVectorSetW(WorldMatrix.r[3], 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}


CDecal* CDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDecal* pInstance = new CDecal(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CDecal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CDecal::Clone(void* pArg)
{
    CDecal* pInstance = new CDecal(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CDecal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CDecal::Free()
{
    __super::Free();

}
