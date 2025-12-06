#include "ClientPch.h"
#include "Levi_Wave.h"

CLevi_Wave::CLevi_Wave(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CLevi_Wave::CLevi_Wave(const CLevi_Wave& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CLevi_Wave::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLevi_Wave::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	WAVEDESC* pDesc = static_cast<WAVEDESC*>(pArg);
	Ready_Component(pDesc);
	m_wstrEffectTag = pDesc->wstrEffectTag;
	m_fMaxLifeTime = 4.f;
	m_isActivate = false;
    return S_OK;
}

void CLevi_Wave::Priority_Update(_float fTimeDelta)
{
	if (m_isDisolve && m_fDesolveTime < 1.f)
		m_fDesolveTime += fTimeDelta;

}

void CLevi_Wave::Update(_float fTimeDelta)
{
	m_pTransformCom->Go_Straight(fTimeDelta);
	if(m_fLifeTime > m_fMaxLifeTime)
	{
		if (false == m_isDisolve)
		{
			m_isDisolve = true;
			m_pRigidBodyCom->IsActivate(false);

			// 터지는 이펙트 호출
			// to do...
		}
	}
	if(false == m_isDisolve)
	{
		m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
		m_fLifeTime += fTimeDelta;
	}
}

void CLevi_Wave::Late_Update(_float fTimeDelta)
{
	if(m_fDesolveTime >= 1.f)
	{
		m_isActivate = false;
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
		m_pTransformCom->Set_State(STATE::POSITION, vPosition);
		m_pRigidBodyCom->IsActivate(false);
		return;
	}
	
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CLevi_Wave::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");

#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif
}

void CLevi_Wave::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	WAVERESET* pDesc = static_cast<WAVERESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	//m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));
	m_pRigidBodyCom->IsActivate(true);
	m_isDisolve = false;
	m_isActivate = true;
	m_fLifeTime = 0.f;
	m_fDesolveTime = 0.f;

	//이펙트 스폰 위치

}

HRESULT CLevi_Wave::Bind_Resources()
{
	//m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	//m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	//m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CLevi_Wave::Ready_Component(WAVEDESC* pDesc)
{
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RigidbodyDesc.vExtent = _float3(2.f,1.f,1.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = pDesc->fAttackDamage;
	//m_CallBack.pCondition = &m_iState;
	//m_tCallDesc.strEffectTag = ;
	m_CallBack.eType = TEXT_COLOR_TYPE::DARK;
	m_pRigidBodyCom->Set_Desc(&m_CallBack);
	m_pRigidBodyCom->IsActivate(false);

	//// Com_Shader 
	//if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_MonsterProp"),
	//	TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
	//	CRASH("Anchor/Com_Shader");
	//
	//// Com_Model
	//if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_Model_Leviatan_Anchor"),
	//	TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
	//	CRASH("Anchor/Com_Model");
	
}

void CLevi_Wave::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if(iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
#ifdef _DEBUG
		cout << "On Hit! (Levi Wave)" << endl;
#endif // _DEBUG
	}
}

CLevi_Wave* CLevi_Wave::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Wave* pInstance = new CLevi_Wave(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Wave");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Wave::Clone(void* pArg)
{
	CLevi_Wave* pClone = new CLevi_Wave(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Wave (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Wave::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
	//Safe_Release(m_pModelCom);
	//Safe_Release(m_pShaderCom);
}
