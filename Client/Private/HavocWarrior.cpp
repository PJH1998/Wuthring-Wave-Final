#include "ClientPch.h"
#include "HavocWarrior.h"
#include "AttackVolume.h"

CHavocWarrior::CHavocWarrior(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor{ pDevice, pContext }
{
}

CHavocWarrior::CHavocWarrior(const CHavocWarrior& Prototype)
	: CActor { Prototype }
{
}

HRESULT CHavocWarrior::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHavocWarrior::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	HAVOCWARRIOR_DESC* pDesc = static_cast<HAVOCWARRIOR_DESC*>(pArg);

	m_pTransformCom->Scale({ 1.f, 1.f, 1.f });
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
#pragma region ATTACK_STATE
	m_fAttackCoolTime[0] = 8.f;
	m_fAttackCoolTime[1] = 20.f;
	m_fAttackCoolTime[2] = 30.f;
#pragma endregion

	Ready_Component(pDesc);
	Ready_PartObjects(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);
	m_vDistanceRange = _float2(2.7f, 3.3f);
	m_fHP = pDesc->fHp;
	m_fAttackDmg = pDesc->fAttackDmg;
	m_fIdleDuration = 30.f;
	m_fIdleAcc = 10.f;
	return S_OK;
}

void CHavocWarrior::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CHavocWarrior::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);

	// 1. Update Current State
	m_pBehaviorTreeCom->tick(this);
	After_Condition(fTimeDelta);
	// 2. Setting Animation & Run
	m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta); //cpu

	// 3. Collider Update
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	m_pColliderCom->Update(vVelocity / fTimeDelta);
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	//Tigger Volume Update
	m_pAtkVolume->Update(fTimeDelta);
}

void CHavocWarrior::Late_Update(_float fTimeDelta)
{
	m_pRigidBodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pColliderCom->Sync_Position(m_pTransformCom);
#ifdef _DEBUG
	if (KEYSTATE::DOWN == m_pGameInstance->Get_DIKeyState(DIK_O))
		m_iState |= ENUM_CLASS(TEST_STATE::STRIKE);
#endif // _DEBUG

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CHavocWarrior::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	//Tigger Volume Render
	m_pAtkVolume->Render();

	//m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CHavocWarrior::Collider_Active(const _wstring& wStrColliderTag, _bool isActive)
{
	if (wStrColliderTag == TEXT("Attack"))
		m_pAtkVolume->TriggerActivate(isActive);
}

void CHavocWarrior::Effect_Active(const _wstring& wStrEffectTag)
{
	//if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
	//	return;
	//
	//_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	//m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}

HRESULT CHavocWarrior::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CHavocWarrior::Ready_Component(HAVOCWARRIOR_DESC* pDesc)
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

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 1.35f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.8f;
	ColliderDesc.fRadius = 0.4f;
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});
	m_tCallDesc.pTransform = m_pTransformCom;
	m_tCallDesc.fAttack = 10.f;
	m_pColliderCom->Set_Desc(&m_tCallDesc);
	m_pColliderCom->Set_Gravity(true);

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("HavocWarrior/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("HavocWarrior/Com_ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("HavocWarrior/Com_Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_AnimMachine_HavocWarrior"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("HavocWarrior/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(0, 3.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(1, 4.f); });
	pBlackBoard->Add_Condition("Attack3", [this]() ->_bool { return Attack(2, 15.f); });
	pBlackBoard->Add_Condition("isChase", [this]() ->_bool { return isChase(); });
	pBlackBoard->Add_Condition("isPatrol", [this]() ->_bool { return isPatrol(); });
	pBlackBoard->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard->Add_Condition("Right", [this]() ->_bool { return Right(); });
	
	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_BehaviorTree_Ordinary"),
		TEXT("Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion
}

void CHavocWarrior::Ready_PartObjects(HAVOCWARRIOR_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bip001RHand");
	TriggerDesc.vExtent = _float3(0.5f, 0.5f, 1.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold); 
		};

	//CContainerObject::Add_PartObject(TEXT("Part_ATKVolume"), m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), &TriggerDesc);
	m_pAtkVolume = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolume)
		CRASH(m_pAtkVolume);
	m_pAtkVolume->TriggerActivate(false);
}

void CHavocWarrior::Reset_Condition(_float fTimeDelta)
{
	if (m_isAnimationFinished)
	{
		_uint iRemainState{};
		if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_3))
			iRemainState |= ENUM_CLASS(TEST_STATE::ATTACK_3);
		m_iState = ENUM_CLASS(TEST_STATE::NONE);
		m_iState |= iRemainState;
	}
	if (m_isTrigger == true)
	{
		if (m_isAggro != m_isDetecting)
			m_iState |= ENUM_CLASS(TEST_STATE::SPAWN);
		m_isDetecting = true;
	}
	else
		m_isDetecting = false;
	if (m_isDetecting)
	{
		Calculate_PosAndDir();

#ifdef _DEBUG
		//cout << "x : " << m_vTargetPosition.x << " y : " << m_vTargetPosition.y << " z : " << m_vTargetPosition.z << endl;
		//cout << "distance: " << m_fDistance << endl;
#endif
	}
	for (_uint i = 0; i < 3; ++i)
	{
		if (m_fAttackAcc[i] > 0.f)
			m_fAttackAcc[i] -= fTimeDelta;
	}
	if (m_fIdleAcc > 0.f)
		m_fIdleAcc -= fTimeDelta;
	else
	{
		m_iState |= ENUM_CLASS(TEST_STATE::LAND);
		m_fIdleAcc = m_fIdleDuration;
	}
}

void CHavocWarrior::After_Condition(_float fTimeDelta)
{
	if (m_iState & (ENUM_CLASS(TEST_STATE::ATTACK_1) | ENUM_CLASS(TEST_STATE::ATTACK_2) | ENUM_CLASS(TEST_STATE::ATTACK_3)))
		m_pTransformCom->LookLerp(XMLoadFloat3(&m_vTargetDir), fTimeDelta, 0.9f);
	else if (m_iState & (ENUM_CLASS(TEST_STATE::MOVE_FORWARD) | ENUM_CLASS(TEST_STATE::MOVE_BACKWARD) | ENUM_CLASS(TEST_STATE::MOVE_LEFT) | ENUM_CLASS(TEST_STATE::MOVE_RIGHT)))
	{
		m_pTransformCom->LookLerp(XMLoadFloat3(&m_vTargetDir), fTimeDelta, 0.9f);
	}
	if (m_iState & (ENUM_CLASS(TEST_STATE::ATTACK_3)))
	{
		if (m_fDistance < 4.f)
		{
			m_iState &= ~(ENUM_CLASS(TEST_STATE::ATTACK_3));
		}
		else
			m_iState |= (ENUM_CLASS(TEST_STATE::ATTACK_3));
	}
	if (true == m_beHit)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
		m_beHit = false;
	}
}

void CHavocWarrior::Calculate_PosAndDir()
{
	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
	_vector vDir = vTargetPos - vPosition;
	m_fDistance = XMVectorGetX(XMVector3Length(vDir));
	vDir = XMVector3Normalize(vDir);
	m_fFrontDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK))));
	m_fRightDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT))));

	XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(XMVectorSetY(vDir, 0.f)));
}

void CHavocWarrior::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		m_isTrigger = true;
		CTransform* pTransform = static_cast<CTransform*>(pOther);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		if (!m_isAggro)
			m_isAggro = true;
	}
}

void CHavocWarrior::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_2))
		m_iState |= ENUM_CLASS(TEST_STATE::STRIKE);
#ifdef _DEBUG
	cout << "On Hit! (Havoc Warrior)" << endl;
#endif // _DEBUG
}

void CHavocWarrior::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
		m_beHit = true;
#ifdef _DEBUG
		cout << "Be Hit! (Havoc Warrior)" << endl;
#endif // _DEBUG

	}
}

void CHavocWarrior::Patrol()
{
	m_vTargetPosition = m_PatrolPoints.front();
	Calculate_PosAndDir();
	m_fFrontDot = 1.f;

	if (m_fDistance < 0.1f)
	{
		m_PatrolPoints.pop();
		m_PatrolPoints.push(m_vTargetPosition);
	}
}

_bool CHavocWarrior::isKnockDown()
{
	if (m_beHit)
		return true;

	return m_iState & (ENUM_CLASS(TEST_STATE::BEHIT) | ENUM_CLASS(TEST_STATE::BLOCK) | ENUM_CLASS(TEST_STATE::AIR));
}

_bool CHavocWarrior::isAttackEnable()
{
	if (!m_isDetecting)
		return false;
	_bool bResult{};
	for (_uint i = 0; i < 3; ++i)
	{
		if (m_fAttackAcc[i] <= 0.f)
		{
			bResult = true;
			break;
		}
	}
	return bResult;
}

_bool CHavocWarrior::Attack(_uint iIndex, _float fInterval)
{
	_bool Result = (m_fAttackAcc[iIndex] <= 0.f) && m_fDistance < fInterval;
	if (Result)
	{
		m_fAttackAcc[iIndex] = m_fAttackCoolTime[iIndex];
	}
	return Result;
}

_bool CHavocWarrior::isChase()
{
	if (m_iState & ENUM_CLASS(TEST_STATE::SPAWN))
		return false;

	_bool bResult{};
	if (m_isDetecting)
	{
		bResult = true;
	}
	else
	{
		if (m_fDistance < 0.1f)
		{
			m_isAggro = false;
		}
	}
	return bResult;
}

_bool CHavocWarrior::isPatrol()
{
	if (m_PatrolPoints.empty())
		return false;

	_bool Result = !m_isAggro;
	if (Result)
	{
		Patrol();
	}
	return Result;
}

_bool CHavocWarrior::Back()
{
	return m_fDistance < m_vDistanceRange.x;
}

_bool CHavocWarrior::Front()
{
	return m_fDistance > m_vDistanceRange.y;
}

_bool CHavocWarrior::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.525f;
}

_bool CHavocWarrior::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.525f;
}

CHavocWarrior* CHavocWarrior::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHavocWarrior* pInstance = new CHavocWarrior(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CHavocWarrior");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHavocWarrior::Clone(void* pArg)
{
	CHavocWarrior* pClone = new CHavocWarrior(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CHavocWarrior (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CHavocWarrior::Free()
{
	__super::Free();

	Safe_Release(m_pAtkVolume);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
