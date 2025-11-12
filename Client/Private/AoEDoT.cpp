#include "ClientPch.h"
#include "AoEDoT.h"

CAoEDoT::CAoEDoT(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CAoEDoT::CAoEDoT(const CAoEDoT& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CAoEDoT::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAoEDoT::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	AOEDOT_DESC* pDesc = static_cast<AOEDOT_DESC*>(pArg);
	Ready_Component(pDesc);

	m_iLayer = pDesc->iLayer;
	m_vOffset = pDesc->vOffset;
	m_iTargetLayers = pDesc->iTargetLayers;
    return S_OK;
}

void CAoEDoT::Priority_Update(_float fTimeDelta)
{
	if(m_isAttack)
	{
		m_pRigidBodyCom->IsActivate(false);
		m_isAttack = false;
	}
}

void CAoEDoT::Update(_float fTimeDelta)
{
	if (m_fDelayAcc >= m_fDelayTime)
	{
		m_fDelayAcc = 0.f;
		m_pRigidBodyCom->IsActivate(true);
		m_isAttack = true;
	}
	else
		m_fDelayAcc += fTimeDelta;

	if (m_fLifeTimeAcc >= m_fLifeTime)
	{
		m_fLifeTimeAcc = 0.f;
		m_isActivate = false;
	}
	else
		m_fLifeTimeAcc += fTimeDelta;

	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CAoEDoT::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this)))
		return;
}

void CAoEDoT::Render()
{
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif
}

void CAoEDoT::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	AOEDOT_RESET* pDesc = static_cast<AOEDOT_RESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	//m_pTransformCom->LookAt(XMLoadFloat3(&pDesc->vTargetPos));
	m_fLifeTime = pDesc->fLifeTime;
	m_fDelayTime = m_fLifeTime / (pDesc->iTickCount);
	m_fDelayAcc = m_fDelayTime;
	m_fLifeTimeAcc = 0.f;
	m_isActivate = true;
}

void CAoEDoT::Ready_Component(AOEDOT_DESC* pDesc)
{
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = pDesc->iLayer;
	RigidbodyDesc.vExtent = pDesc->vExtent;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = pDesc->fAttackDamage;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);
}

void CAoEDoT::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	for( auto& iTarget : m_iTargetLayers)
	{
		if (iLayer == iTarget)
		{
#ifdef _DEBUG
			cout << "On Hit! (AoE Dot)" << endl;
#endif // _DEBUG
			return;
		}
	}

}

CAoEDoT* CAoEDoT::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAoEDoT* pInstance = new CAoEDoT(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CAoEDoT");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAoEDoT::Clone(void* pArg)
{
	CAoEDoT* pClone = new CAoEDoT(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CAoEDoT (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CAoEDoT::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
}
