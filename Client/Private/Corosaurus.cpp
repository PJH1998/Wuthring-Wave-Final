#include "ClientPch.h"
#include "Corosaurus.h"
#include "AttackVolume.h"

CCorosaurus::CCorosaurus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CActor { pDevice, pContext }
{
}

CCorosaurus::CCorosaurus(const CCorosaurus& Prototype)
	: CActor { Prototype }
{
}

HRESULT CCorosaurus::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCorosaurus::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	CORROSAURUS_DESC* pDesc = static_cast<CORROSAURUS_DESC*>(pArg);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	m_vDistanceRange = _float2(4.f, 5.f);
	Ready_Component(pDesc);
	//Ready_PartObjects(pDesc);
	//CActor::Register_AllNotifies(pDesc->strFolderPath);

	return S_OK;
}

void CCorosaurus::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CCorosaurus::Update(_float fTimeDelta)
{
}

void CCorosaurus::Late_Update(_float fTimeDelta)
{
}

void CCorosaurus::Render()
{
}

void CCorosaurus::Collider_Active(const _wstring& wStrColliderTag, _bool isActive)
{
}

void CCorosaurus::Effect_Active(const _wstring& wStrEffectTag)
{
}

HRESULT CCorosaurus::Bind_Resources()
{
	return E_NOTIMPL;
}

void CCorosaurus::Ready_Component(CORROSAURUS_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(13.f, 9.f, 13.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});

	// Com_Collider (Body)
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 1.35f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.8f;
	ColliderDesc.fRadius = 0.4f;
	XMStoreFloat4(&ColliderDesc.vQuat, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(90.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f)));
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});
	m_tCallDesc.pTransform = m_pTransformCom;
	m_tCallDesc.fAttack = 10.f;
	m_pColliderCom->Set_Desc(&m_tCallDesc);


	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Corrosaurus/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("Corrosaurus/Com_ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Corrosaurus/Com_Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_AnimMachine_Corrosaurus"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("Corrosaurus/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("ATKArrange", [this]() ->_bool { return AttackArrange(); });
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK1, 3.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK2, 4.f); });
	pBlackBoard->Add_Condition("Attack3", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK3, 15.f); });
	pBlackBoard->Add_Condition("Attack4", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK4, 15.f); });
	pBlackBoard->Add_Condition("Attack7", [this]() ->_bool { return Attack(ATK_PATTERN::BURST, 50.f); });
	pBlackBoard->Add_Condition("Attack8", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK8, 15.f); });
	pBlackBoard->Add_Condition("isChase", [this]() ->_bool { return isChase(); });
	pBlackBoard->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard->Add_Condition("Right", [this]() ->_bool { return Right(); });
	
	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_BehaviorTree_Corrosaurus"),
		TEXT("Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion
}

void CCorosaurus::Ready_PartObjects(CORROSAURUS_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Prop003_M");
	TriggerDesc.vExtent = _float3(0.5f, 0.5f, 1.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	m_pAtkVolumes[ATK_SOCKET::HEAD] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::HEAD])
		CRASH(m_pAtkVolume);
	m_pAtkVolumes[ATK_SOCKET::HEAD]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Tail006_M");
	TriggerDesc.vExtent = _float3(0.5f, 0.5f, 1.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::TAIL] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::TAIL])
		CRASH(m_pAtkVolume);
	m_pAtkVolumes[ATK_SOCKET::TAIL]->TriggerActivate(false);
}

void CCorosaurus::Reset_Condition(_float fTimeDelta)
{
}

void CCorosaurus::After_Condition(_float fTimeDelta)
{
}

void CCorosaurus::Calculate_PosAndDir()
{
}

void CCorosaurus::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CCorosaurus::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CCorosaurus::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
#ifdef _DEBUG
		cout << "Be Hit! (Corro)" << endl;
		cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG

	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
#ifdef _DEBUG
		cout << "Be Hit! SKILL (Corro)" << endl;
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
		//m_isPushed = true;
		//m_isAir = true;
		m_iState |= ENUM_CLASS(TEST_STATE::AIR);
		memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));
#ifdef _DEBUG
		cout << "Knock Back! (Corro)" << endl;
		cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG
	}
}

_bool CCorosaurus::isKnockDown()
{
	return _bool();
}

_bool CCorosaurus::isAttackEnable()
{
	if (!m_isDetecting)
		return false;

	return _bool();
}

_bool CCorosaurus::AttackArrange() const
{
	if (m_iState == 0)
	{

	}
	else
	{

	}
	return true;
}

_bool CCorosaurus::Attack(_uint iIndex, _float fInterval)
{
	return _bool();
}

_bool CCorosaurus::isChase()
{
	return _bool();
}

_bool CCorosaurus::isPatrol()
{
	return _bool();
}

_bool CCorosaurus::Back()
{
	return _bool();
}

_bool CCorosaurus::Front()
{
	return _bool();
}

_bool CCorosaurus::Left()
{
	return _bool();
}

_bool CCorosaurus::Right()
{
	return _bool();
}

CCorosaurus* CCorosaurus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCorosaurus* pInstance = new CCorosaurus(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCorosaurus");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCorosaurus::Clone(void* pArg)
{
	CCorosaurus* pClone = new CCorosaurus(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CCorosaurus (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CCorosaurus::Free()
{
	__super::Free();

	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		Safe_Release(m_pAtkVolumes[i]);
	}

	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
