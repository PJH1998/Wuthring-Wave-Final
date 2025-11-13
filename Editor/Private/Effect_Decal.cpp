#include "Editorpch.h"
#include "Effect_Decal.h"

CEffect_Decal::CEffect_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Decal::CEffect_Decal(const CEffect_Decal& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Decal::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Decal::Initialize_Clone(void* pArg)
{
	DECAL_DESC* pDesc = static_cast<DECAL_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_vColor = pDesc->vColor;
    m_LifeTime = pDesc->LifeTime;
	m_wstrMyTag = pDesc->wstrDecalTag;


   // m_pTransformCom->Scale(_float3(pDesc->vSize.x, pDesc->vSize.y, pDesc->vSize.z));

    //처음 만들어질 땐 무조건 활성화 ?
    m_isActivate = false;

    return S_OK;
}

void CEffect_Decal::Priority_Update(_float fTimeDelta)
{
}

void CEffect_Decal::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	DECAL_DATA Desc{};
	Desc.eType = DECAL_DATA::NONSTATIC;
	Desc.fLifeTime = m_LifeTime;
	Desc.vColor = m_vColor;
	Desc.WorldMatrix = m_ComBindMatrix; /*m_pTransformCom->Get_WorldMatrix();*/

	m_pGameInstance->Add_DecalData(m_wstrMyTag, Desc);

	m_isActivate = false;
}

void CEffect_Decal::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

}

void CEffect_Decal::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	m_isActivate = pDesc->IsActive;

     Root_Transform(WorldMatrix);
}

void CEffect_Decal::Root_Transform(_fmatrix WorldMatrix)
{
	//_vector vScale = {};
	//_vector vPos = {};
	//_vector vRot = {};
	//XMMatrixDecompose(&vScale, &vRot, &vPos, WorldMatrix);

	//_float3 vfScale = {};
	//XMStoreFloat3(&vfScale, vScale);

	//m_pTransformCom->Scale(vfScale);
 //   m_pTransformCom->Set_State(STATE::POSITION, vPos);

	m_ComBindMatrix = WorldMatrix;
}


CEffect_Decal* CEffect_Decal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Decal* pInstance = new CEffect_Decal(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Decal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Decal::Clone(void* pArg)
{
    CEffect_Decal* pInstance = new CEffect_Decal(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Decal");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Decal::Free()
{
    __super::Free();

}
