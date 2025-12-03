#include "ClientPch.h"
#include "Levi_Drop.h"

CLevi_Drop::CLevi_Drop(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CLevi_Drop::CLevi_Drop(const CLevi_Drop& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CLevi_Drop::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLevi_Drop::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DROPDESC* pDesc = static_cast<DROPDESC*>(pArg);
	Ready_Component(pDesc);
	m_wstrEffectTag = pDesc->wstrEffectTag;
	m_fMaxLifeTime = 1.f;
	m_isActivate = false;
    return S_OK;
}

void CLevi_Drop::Priority_Update(_float fTimeDelta)
{
	if (m_isDisolve && m_fLifeTime < m_fMaxLifeTime)
		m_fLifeTime += fTimeDelta;
}

void CLevi_Drop::Update(_float fTimeDelta)
{
	_vector vDir = XMVectorSetW(XMLoadFloat3(&m_vTargetPos) - m_pTransformCom->Get_State(STATE::POSITION), 1.f);
	if (XMVectorGetX(XMVector3Dot(m_pTransformCom->Get_State(STATE::UP), vDir)) < 0.f)
		m_pTransformCom->Go_Dir(XMVectorSet(0.f, -1.f, 0.f, 0.f), fTimeDelta);
	else
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
		m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CLevi_Drop::Late_Update(_float fTimeDelta)
{
	if(m_fLifeTime >= m_fMaxLifeTime)
	{
		m_isActivate = false;
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
		vPosition = XMVectorSetY(vPosition, 1000.f);
		m_pTransformCom->Set_State(STATE::POSITION, vPosition);
		m_pRigidBodyCom->IsActivate(false);
		return;
	}
	
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CLevi_Drop::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");

#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif
}

void CLevi_Drop::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	DROPRESET* pDesc = static_cast<DROPRESET*>(pArg);
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	//m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&pDesc->vTargetPos), 1.f));
	m_vTargetPos = pDesc->vTargetPos;
	m_pRigidBodyCom->IsActivate(true);
	m_isDisolve = false;
	m_isActivate = true;
	m_fLifeTime = 0.f;
}

HRESULT CLevi_Drop::Bind_Resources()
{
	//m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	//m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	//m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CLevi_Drop::Ready_Component(DROPDESC* pDesc)
{
	CRigidbody::SPHEREBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::SPHERE;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RigidbodyDesc.fRadius = 0.8f;
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

void CLevi_Drop::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if(iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
#ifdef _DEBUG
		cout << "On Hit! (Levi Drop)" << endl;
#endif // _DEBUG
	}
}

CLevi_Drop* CLevi_Drop::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Drop* pInstance = new CLevi_Drop(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Drop");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Drop::Clone(void* pArg)
{
	CLevi_Drop* pClone = new CLevi_Drop(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Drop (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Drop::Free()
{
	__super::Free();

	Safe_Release(m_pRigidBodyCom);
	//Safe_Release(m_pModelCom);
	//Safe_Release(m_pShaderCom);
}
