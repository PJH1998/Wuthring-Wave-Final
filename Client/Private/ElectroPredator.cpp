#include "ClientPch.h"
#include "ElectroPredator.h"

CElectroPredator::CElectroPredator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor{ pDevice, pContext }
{
}

CElectroPredator::CElectroPredator(const CElectroPredator& Prototype)
	: CActor{ Prototype }
{
}

HRESULT CElectroPredator::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElectroPredator::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	ELECTROPREDATOR_DESC* pDesc = static_cast<ELECTROPREDATOR_DESC*>(pArg);


	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
#pragma region ATTACK_STATE
	m_fAttackCoolTime[0] = 8.f;
	m_fAttackCoolTime[1] = 20.f;
	m_fAttackCoolTime[2] = 30.f;
#pragma endregion

	Ready_Component(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);
	m_iHP = pDesc->fHp;
	m_fAttackDmg = pDesc->fAttackDmg;
	m_vDistanceRange = _float2(7.f, 12.95f);
	m_fIdleDuration = 30.f;
	m_fIdleAcc = 10.f;
	m_fImpluseRate = pDesc->fImpluseRate;
	//임시 patrol 위치 데이터
	m_PatrolPoints.push(_float3(0.f, -8.f, 3.f));
	m_PatrolPoints.push(pDesc->vInitPosition);
	return S_OK;
}

void CElectroPredator::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CElectroPredator::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);

	// 1. Update Current State
	m_pBehaviorTreeCom->tick(this);

	After_Condition(fTimeDelta);

	// 2. Setting Animation & Run
	m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta); //cpu
	//_float temp;
	//m_pModelCom->Play_Animation_CPU("Stand2", fTimeDelta, &temp, false, true, false, true, 1.f);
	//m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	// 3. Collider Update
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if (m_isPushed)
	{
		_vector vBeHitDir = XMVector3Normalize(XMLoadFloat3(&m_vBeHit_Normal) * 2.f + XMVectorSet(0.f, 1.f, 0.f, 0.f));
		m_isPushed = false;
		m_iState |= ENUM_CLASS(TEST_STATE::BLOCK);
		ZeroMemory(&m_vBeHit_Normal, sizeof(_float3));
		vVelocity += vBeHitDir * m_fImpluseRate; //임펄스 수치
	}
	else if ((m_iState & ENUM_CLASS(TEST_STATE::AIR)) && (m_iState & ENUM_CLASS(TEST_STATE::BEHIT)))
	{
		_vector vBeHitDir = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		ZeroMemory(&m_vBeHit_Normal, sizeof(_float3));
		vVelocity += vBeHitDir * m_fImpluseRate; //임펄스 수치
		m_iState &= ~ENUM_CLASS(TEST_STATE::PARALYSIS);
	}
	m_pColliderCom->Update(vVelocity / fTimeDelta);
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CElectroPredator::Late_Update(_float fTimeDelta)
{
	//m_pRigidBodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pColliderCom->Sync_Position(m_pTransformCom);
	if (m_iState & ENUM_CLASS(TEST_STATE::AIR))
	{

		if (m_fAirAcc >= 0.15f)
		{
			if (m_pColliderCom->IsLand() && m_iState & ENUM_CLASS(TEST_STATE::AIR))
			{
				m_iState &= ~ENUM_CLASS(TEST_STATE::AIR);
				//m_isAir = false;
				m_fAirAcc = 0.f;
			}
		}
		else
			m_fAirAcc += fTimeDelta;
	}
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CElectroPredator::Render()
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

	//m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CElectroPredator::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	if (wStrColliderTag == TEXT("Lerp"))
	{
		TurnLerp(Isactive);
	}
}

void CElectroPredator::Effect_Active(const _wstring& wStrEffectTag)
{
	//if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
	//	return;
	//
	//_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	//m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}

void CElectroPredator::Object_Func(const _wstring& wStrObjectTag)
{
	if (wStrObjectTag == TEXT("Look"))
	{
		TurnFix();
	}
}

HRESULT CElectroPredator::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CElectroPredator::Ready_Component(ELECTROPREDATOR_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(25.f, 13.f, 25.f);
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
	ColliderDesc.vOffset = _float3(0.f, 1.15f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.5f;
	ColliderDesc.fRadius = 0.4f;
	ColliderDesc.fRayOffset = -0.11f;
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);

	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = m_fAttackDmg;
	m_pColliderCom->Set_Desc(&m_CallBack);
	m_pColliderCom->Set_Gravity(true);

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("ElectroPredator/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("ElectroPredator/Com_ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("ElectroPredator/Com_Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_AnimMachine_ElectroPredator"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("ElectroPredator/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(0, 13.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(1, 13.f); });
	pBlackBoard->Add_Condition("Attack3", [this]() ->_bool { return Attack(2, 14.f); });
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

void CElectroPredator::Ready_PartObjects(ELECTROPREDATOR_DESC* pDesc)
{
}

void CElectroPredator::Reset_Condition(_float fTimeDelta)
{
	if (m_isAnimationFinished)
	{
		_uint iRemainState{};
		if (m_iState & ENUM_CLASS(TEST_STATE::AIR))
			iRemainState |= ENUM_CLASS(TEST_STATE::AIR);
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
	m_isTrigger = false;
	if (m_isDetecting)
	{
		Calculate_PosAndDir();
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

void CElectroPredator::After_Condition(_float fTimeDelta)
{
	if (m_isTurnLerp)
		m_pTransformCom->LookLerp(XMLoadFloat3(&m_vTargetDir), fTimeDelta);

	if (m_iState & (ENUM_CLASS(TEST_STATE::ATTACK_3)))
	{
		if (m_fDistance < 4.f)
		{
			m_iState &= ~(ENUM_CLASS(TEST_STATE::ATTACK_3));
		}
	}
	if (m_iState & ENUM_CLASS(TEST_STATE::AIR))
	{
		if (!m_AirTrig)
		{
			m_iState = ENUM_CLASS(TEST_STATE::AIR);
			m_AirTrig = true;
		}
	}
	else
		m_AirTrig = false;

	if (true == m_beHit)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
		m_beHit = false;
	}
	else
		m_iState &= ~ENUM_CLASS(TEST_STATE::BEHIT);
}

void CElectroPredator::Calculate_PosAndDir()
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

void CElectroPredator::TurnFix()
{
	m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
}

void CElectroPredator::TurnLerp(_bool isActive)
{
	m_isTurnLerp = isActive;
}

void CElectroPredator::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		m_isTrigger = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		if (!m_isAggro)
			m_isAggro = true;
	}
}

void CElectroPredator::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
		m_beHit = true;
#ifdef _DEBUG
		cout << "Be Hit! (Electro Predator)" << endl;
		//m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
	{
		m_beHit = true;
#ifdef _DEBUG
		cout << "Be Hit! SKILL (False Sovereign)" << endl;
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		m_beHit = true;
		m_isPushed = true;
		m_iState |= ENUM_CLASS(TEST_STATE::AIR);
		memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));
#ifdef _DEBUG
		cout << "Knock Back! (Electro Predator)" << endl;
		cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG
	}
}

void CElectroPredator::Patrol()
{
	m_vTargetPosition = m_PatrolPoints.front();
	//_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	//_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
	//_vector vDir = vTargetPos - vPosition;
	//m_fDistance = XMVectorGetX(XMVector3Length(vDir));
	//XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(XMVectorSetY(vDir, 0.f)));
	//m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	Calculate_PosAndDir();
	m_fFrontDot = 1.f;

	if (m_fDistance < 0.1f)
	{
		m_PatrolPoints.pop();
		m_PatrolPoints.push(m_vTargetPosition);
	}
}

_bool CElectroPredator::isKnockDown()
{
	if (m_beHit)
		return true;

	return m_iState & (ENUM_CLASS(TEST_STATE::BEHIT) | ENUM_CLASS(TEST_STATE::BLOCK) | ENUM_CLASS(TEST_STATE::AIR));
}

_bool CElectroPredator::isAttackEnable()
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

_bool CElectroPredator::Attack(_uint iIndex, _float fInterval)
{
	_bool bResult = (m_fAttackAcc[iIndex] <= 0.f) && m_fDistance < fInterval;
	if (bResult)
	{
		m_fAttackAcc[iIndex] = m_fAttackCoolTime[iIndex];
	}
	return bResult;
}

_bool CElectroPredator::isChase()
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

_bool CElectroPredator::isPatrol()
{
	if (m_PatrolPoints.empty())
		return false;

	_bool bResult = !m_isAggro;
	if (bResult)
	{
		Patrol();
	}
	return bResult;
}

_bool CElectroPredator::Back()
{
	return  m_fDistance < m_vDistanceRange.x;
}

_bool CElectroPredator::Front()
{
	return m_fDistance > m_vDistanceRange.y;
}

_bool CElectroPredator::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.525f;
}

_bool CElectroPredator::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.525f;
}

CElectroPredator* CElectroPredator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElectroPredator* pInstance = new CElectroPredator(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CElectroPredator");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CElectroPredator::Clone(void* pArg)
{
	CElectroPredator* pClone = new CElectroPredator(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CElectroPredator (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CElectroPredator::Free()
{
	__super::Free();

	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
