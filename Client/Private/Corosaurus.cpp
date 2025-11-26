#include "ClientPch.h"
#include "Corosaurus.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "Coro_Rock.h"

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

	m_pGameSystem = CGameSystem::GetInstance();
	Safe_AddRef(m_pGameSystem);
	CORROSAURUS_DESC* pDesc = static_cast<CORROSAURUS_DESC*>(pArg);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pDesc->vInitRotate.x), XMConvertToRadians(pDesc->vInitRotate.y), XMConvertToRadians(pDesc->vInitRotate.z));
	m_pTransformCom->Rotation_Quaternion(vQuat);
	//m_vDistanceRange = _float2(4.f, 5.f);
	Ready_Component(pDesc);
	Ready_PartObjects(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);
#pragma region ATTACK_STATE
	m_fAttackCoolTime[ATK_PATTERN::ATTACK1] = 6.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK2] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::BURST] = /*m_fAttackAcc[ATK_PATTERN::BURST] =*/ 70.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK8] = /*m_fAttackAcc[ATK_PATTERN::ATTACK8] =*/ 5.f;
#pragma endregion
	m_fStamina = m_fMaxStamina = pDesc->fMaxStamina;
	m_fHP = pDesc->fHP;
	m_fHitStopRatio = 1.f;
	m_fParalysisAcc = 5.f;
	return S_OK;
}

void CCorosaurus::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();

	for (auto& Pair : m_PartObjects)
	{
		if(Pair.second->IsActivate())
			Pair.second->Priority_Update(fTimeDelta);
	}
}

void CCorosaurus::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);
	m_pBehaviorTreeCom->tick(this);

	After_Condition(fTimeDelta);
	//m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * m_fHitStopRatio);
	m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * m_fHitStopRatio);
	_vector vVelocity = m_pTransformCom->Get_Velocity();

	if(m_isDist_Interp_Enable)
	{
		_float temp = clamp(m_fDistanceNonY - 1.5f, 0.f, 1.f);
		m_pColliderCom->Update(vVelocity / fTimeDelta * temp);
	}
	else
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Update(fTimeDelta);
	}
	m_pParryVolume->Update(fTimeDelta);

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Update(fTimeDelta);
	}
}

void CCorosaurus::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_fStamina <= 0.f && m_fParalysisAcc >= 5.f)
		m_isParalysis = true;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Late_Update(fTimeDelta);
	}
}

void CCorosaurus::Render()
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
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		//m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL));
		m_pShaderCom->Begin(m_ShaderIndices[i]);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pColliderCom->Render();
	m_pRigidBodyCom->Render();
	for (_uint i = 0; i < ATK_SOCKET::END; i++)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Render();
	}
	//m_pParryVolume->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif // DEBUG

}

void CCorosaurus::Collider_Active(const _wstring& wStrColliderTag, _bool isActive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);
	if (wstrTypeTag == TEXT("Attack"))
	{
		if (wstrPartTag == TEXT("Head"))
		{
			m_pAtkVolumes[ATK_SOCKET::HEAD0]->TriggerActivate(isActive);
		}
		else
		{
			m_pAtkVolumes[ATK_SOCKET::TAIL]->TriggerActivate(isActive);
		}
	}
	else if (wstrTypeTag == TEXT("Parry"))
	{
		m_pParryVolume->TriggerActivate(isActive);
	}
	else if (wstrTypeTag == TEXT("Gravity"))
	{
		m_pColliderCom->Set_Gravity(isActive);
	}
	else if (wstrTypeTag == TEXT("Lerp"))
	{
		m_isTurnLerp = isActive;
	}
	else if (wstrTypeTag == TEXT("Distance"))
	{
		m_isDist_Interp_Enable = isActive;
	}
}

void CCorosaurus::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CCorosaurus::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrAnimTag = wStrObjectTag.substr(Index + 1);
	if (wstrTypeTag == TEXT("Look"))
	{
			m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	}
	else if (wstrTypeTag == TEXT("LookRev"))
	{
			m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir) * -1.f);
	}

}

HRESULT CCorosaurus::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CCorosaurus::Ready_Component(CORROSAURUS_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vDetectRange;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});
	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnDetect_Enter(iLayer, pDesc, Manifold);
		});
	// 숙면하는 짱룡
	//m_pRigidBodyCom->IsActivate(false);

	// Com_Collider (Body)
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 1.8f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.7f;
	ColliderDesc.fRadius = 1.f;
	ColliderDesc.fRayOffset = -0.3f;
	//XMStoreFloat4(&ColliderDesc.vQuat, XMQuaternionRotationRollPitchYaw(XMConvertToRadians(90.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f)));
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});
	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = pDesc->fAttackDmg;
	m_CallBack.pCondition = &m_iState;
	//m_CallBack.strEffectTag = ;
	m_CallBack.eType = TEXT_COLOR_TYPE::FUSI;
	m_pColliderCom->Set_Desc(&m_CallBack);


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
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag = pDesc->pAnimationTag;
	//Com_AnimMachine
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_AnimMachine_CoroSaurus"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("Corrosaurus/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("ATKArrange", [this]() ->_bool { return AttackArrange(); });
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK1, 4.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK2, 5.f); });
	pBlackBoard->Add_Condition("Attack8", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK8, 15.f); });
	pBlackBoard->Add_Condition("Attack10", [this]() ->_bool { return Attack(ATK_PATTERN::BURST, 50.f); });
	pBlackBoard->Add_Condition("BeHit", [this]() ->_bool { return CheckHit(); });
	pBlackBoard->Add_Condition("isChase", [this]() ->_bool { return isChase(); });
	pBlackBoard->Add_Condition("isPatrol", [this]() ->_bool { return isPatrol(); });
	pBlackBoard->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard->Add_Condition("Right", [this]() ->_bool { return Right(); });
	
	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_Component_BehaviorTree_CoroSaurus"),
		TEXT("Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion
}

void CCorosaurus::Ready_PartObjects(CORROSAURUS_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Prop003_M");
	TriggerDesc.vExtent = _float3(2.f, 0.5f, 0.5f);
	TriggerDesc.vOffsetPos = _float3(-0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::FUSI;
	//TriggerDesc.pCondition = &m_iState;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	m_pAtkVolumes[ATK_SOCKET::HEAD0] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::HEAD0])
		CRASH(m_pAtkVolume);
	m_pAtkVolumes[ATK_SOCKET::HEAD0]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Tail006_M");
	TriggerDesc.vExtent = _float3(3.f, 0.55f, 0.55f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::TAIL] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::TAIL])
		CRASH(m_pAtkVolume);
	m_pAtkVolumes[ATK_SOCKET::TAIL]->TriggerActivate(false);

	TriggerDesc.eLayer = COLLISIONLAYER::PARRY;
	TriggerDesc.eTargetLayers = { COLLISIONLAYER::ATTACK, COLLISIONLAYER::SKILL, COLLISIONLAYER::KNOCKBACK };
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(2); // Root
	TriggerDesc.vExtent = _float3(2.f, 4.f, 4.f);
	TriggerDesc.vOffsetPos = _float3(0.0f, 0.f, -4.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->ParryEnter(iLayer, pOther, Manifold);
		};
	m_pParryVolume = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pParryVolume)
		CRASH(m_pParryVolume);
	m_pParryVolume->TriggerActivate(false);

	CCoro_Rock::CORO_ROCK_DESC RockDesc{};
	RockDesc.pParentTransform = m_pTransformCom;
	RockDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_WeaponProp001");
	RockDesc.vOffsetTrans = _float3(2.5f, 0.f, 0.f);
	RockDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(-90.f));
	RockDesc.fAttackDmg = pDesc->fAttackDmg * 2.f;
	RockDesc.eType = m_CallBack.eType;
	if (FAILED(CContainerObject::Add_PartObject(TEXT("Part_Rock"), ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_GameObject_CoroRock"), &RockDesc)))
		CRASH("Failed to Clone PartObj : Coro_Rock");
	m_pCoroRock = dynamic_cast<CCoro_Rock*>(Find_PartObject(TEXT("Part_Rock")));
	if (nullptr == m_pCoroRock)
		CRASH("Failed to Find PartObj");
	Safe_AddRef(m_pCoroRock);
}

void CCorosaurus::Reset_Condition(_float fTimeDelta)
{
	if (m_fHP <= 0.f)
	{
		m_iState = ENUM_CLASS(TEST_STATE::DEAD);
		return;
	}
	if (m_isAnimationFinished)
	{
		_uint iRemainState{};
		// burst
		if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_7))
		{
			if (m_iState & ENUM_CLASS(TEST_STATE::STRIKE))
			{
				iRemainState |= ENUM_CLASS(TEST_STATE::STRIKE);
			}
			else
				iRemainState |= ENUM_CLASS(TEST_STATE::ATTACK_7);
		}
		if (m_iState & (ENUM_CLASS(TEST_STATE::ATTACK_1) | ENUM_CLASS(TEST_STATE::ATTACK_2)))
		{

		}
		else
		{
			if (m_iState & ENUM_CLASS(TEST_STATE::MOVE_LEFT))
			{
				_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, -1.f * XM_PIDIV2, 0.f);
				m_pTransformCom->Turn_Quaternion(vQuat);
			}
			else if (m_iState & ENUM_CLASS(TEST_STATE::MOVE_RIGHT))
			{
				_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, XM_PIDIV2, 0.f);
				m_pTransformCom->Turn_Quaternion(vQuat);
			}
		}
		m_iState = ENUM_CLASS(TEST_STATE::NONE);
		m_iState |= iRemainState;
	}
	if (m_isTrigger == true)
	{
		m_isDetecting = true;
	}
	else
		m_isDetecting = false;
	if (m_isDetecting)
	{
		Calculate_PosAndDir();
	}
	for (_uint i = 0; i < ATK_PATTERN::ATK_END; ++i)
	{
		if (m_fAttackAcc[i] > 0.f)
			m_fAttackAcc[i] -= fTimeDelta;
	}

#pragma region UI_BIND
	m_fParalysisRatio = m_fParalysisAcc * 0.2f;
#pragma endregion

	if (m_isParalysis)
	{
		m_fParalysisAcc -= fTimeDelta;
		if (m_fParalysisAcc <= 0.f)
		{
			//그로기 유지시간 정의하기
			m_fParalysisAcc = 5.f;
			m_isParalysis = false;
			m_fStamina = m_fMaxStamina;
		}
	}
	else
		m_isKnockDown = m_isParalysis;
}

void CCorosaurus::After_Condition(_float fTimeDelta)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
	{
		if (!m_isDeadTrigger)
		{
			m_isDeadTrigger = true;
			m_pColliderCom->IsActivate(false);
			m_pRigidBodyCom->IsActivate(false);
		}
		return;
	}
	if (m_isTurnLerp)
		m_pTransformCom->LookLerp(XMLoadFloat3(&m_vTargetDir), fTimeDelta);
	if (m_beHit)
		m_beHit = false;
	if (m_isParalysis)
	{
		if (m_isKnockDown)
			m_iState = ENUM_CLASS(TEST_STATE::PARALYSIS);
		else
		{
			m_iState = (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::MOVE_FORWARD));
			m_isKnockDown = true;
		}
	}
	else
		m_isKnockDown = m_isParalysis;

}

void CCorosaurus::Calculate_PosAndDir()
{
	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
	_vector vDir = vTargetPos - vPosition;
	m_fDistance = XMVectorGetX(XMVector3Length(vDir));
	m_fDistanceNonY = XMVectorGetX(XMVector3Length(XMVectorSetY(vTargetPos, 0.f) - XMVectorSetY(vPosition, 0.f)));
	vDir = XMVector3Normalize(vDir);
	m_fFrontDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK))));
	m_fRightDot = XMVectorGetX(XMVector3Dot(vDir, XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT))));

	XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(XMVectorSetY(vDir, 0.f)));
}

void CCorosaurus::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		m_isTrigger = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
	}
}

void CCorosaurus::OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		if (m_isAggro)
			return;
		//UI Binding (몬스터 데이터 찾기용 키값, 현재 체력 변수 주소, 현재 무력화게이지 변수 주소, 텍스트 출력용 한글 wtring)
		m_pGameSystem->HUD_Bind_BossStatus(TEXT("코로사우로스"), "CoroSaurus", &m_fHP, &m_fStamina, &m_isParalysis, &m_fParalysisRatio);
		m_pGameSystem->HUD_Toggle_BossStatusUI(true);
	}
}

void CCorosaurus::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_7))
	{
		m_iState |= ENUM_CLASS(TEST_STATE::STRIKE);
	}
#ifdef _DEBUG
	cout << "On Hit! (Coro)" << endl;
#endif // _DEBUG
}

void CCorosaurus::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
		return;
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fHP -= pDesc->fAttack;
		if (!m_isParalysis && m_fStamina >= 0.f)
			m_fStamina -= 1.f;
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 0.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		m_beHit = true;
		if (m_fHP <= 0.f)
		{
			m_pGameSystem->HUD_Toggle_BossStatusUI(false);
		}
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);
#pragma endregion
#ifdef _DEBUG
		cout << "Be Hit! (Corro)" << endl;
		//cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG

	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fHP -= pDesc->fAttack;
		if (!m_isParalysis && m_fStamina >= 0.f)
			m_fStamina -= 1.f;
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 0.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		m_beHit = true;
		if (m_fHP <= 0.f)
		{
			m_pGameSystem->HUD_Toggle_BossStatusUI(false);
		}
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);
#pragma endregion
#ifdef _DEBUG
		cout << "Be Hit! SKILL (Corro)" << endl;
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fHP -= pDesc->fAttack;
		if (!m_isParalysis && m_fStamina >= 0.f)
			m_fStamina -= 1.f;
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 0.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		m_beHit = true;
		memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));
		if (m_fHP <= 0.f)
		{
			m_pGameSystem->HUD_Toggle_BossStatusUI(false);
		}
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);
#pragma endregion
#ifdef _DEBUG
		cout << "Knock Back! (Corro)" << endl;
		cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG
	}
}

void CCorosaurus::ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));
	m_isBlocked = true;
#ifdef _DEBUG
	cout << "Parry! (Corro)" << endl;
	cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG
}

_bool CCorosaurus::isKnockDown()
{
	//_bool isKnockDown{};
	//if (m_fStamina <= 0.f)
	//{
	//	if (m_fParalysisAcc <= 0.f)
	//	{
	//		m_fStamina = m_fMaxStamina;
	//		m_fParalysisAcc = 5.f;
	//		isKnockDown = false;
	//	}
	//	else
	//	{
	//		//if (m_isKnockDown)
	//		//{
	//		//	m_iState |= ENUM_CLASS(TEST_STATE::PARALYSIS);
	//		//	isKnockDown = true;
	//		//}
	//		//else
	//		//{
	//		//	m_iState |= (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::MOVE_FORWARD));
	//		//	m_isKnockDown = true;
	//		//	isKnockDown = true;
	//		//}
	//	}
	//}
	//else if (m_isBlocked)
	//{
	//	m_iState |= ENUM_CLASS(TEST_STATE::BLOCK);
	//	isKnockDown = true;
	//	m_isBlocked = false;
	//}
	//
	//return isKnockDown;

	if (m_isParalysis)
		return true;

	return m_iState & (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::BLOCK) | ENUM_CLASS(TEST_STATE::BEHIT));
}

_bool CCorosaurus::isAttackEnable()
{
	if (!m_isDetecting || !m_isAggro)
		return false;
	if (m_fDistance > 20.f)
		return false;

	m_isAttack = true;
	return true;
}

_bool CCorosaurus::AttackArrange()
{
	if (m_iState == 0)
	{
		if (!m_isAggro && m_isDetecting)
		{
			m_isAggro = true;
			m_iState |= ENUM_CLASS(TEST_STATE::SPAWN);
		}
		return false;
	}
	else
	{
		_float fRand = m_pGameInstance->Rand_Normal();
		switch (m_iCurrentAtkIndex)
		{
		case ATK_PATTERN::ATTACK1:
		{
			if (fRand < 0.5f)
			{
				if (m_fRightDot > 0.f)
					m_iState |= ENUM_CLASS(TEST_STATE::MOVE_RIGHT);
				else
					m_iState |= ENUM_CLASS(TEST_STATE::MOVE_LEFT);
			}
			break;
		}
		case ATK_PATTERN::ATTACK2:
		{
			if (m_fRightDot > 0.f)
				m_iState |= ENUM_CLASS(TEST_STATE::MOVE_RIGHT);
			else
				m_iState |= ENUM_CLASS(TEST_STATE::MOVE_LEFT);
			if (fRand < 0.5f)
			{
				m_iState |= ENUM_CLASS(TEST_STATE::MOVE_BACKWARD);
			}
				break;
		}
		default:
			break;
		}
	}
	return true;
}

_bool CCorosaurus::Attack(_uint iIndex, _float fInterval)
{
	if (iIndex != ATK_PATTERN::ATTACK8)
		return false;
	_bool bResult = (m_fAttackAcc[iIndex] <= 0.f) && m_fDistanceNonY < fInterval;
	if (bResult)
	{
		m_fAttackAcc[iIndex] = m_fAttackCoolTime[iIndex];
		m_iCurrentAtkIndex = iIndex;
	}
	else
		m_iCurrentAtkIndex = ATK_PATTERN::ATK_END;
	return bResult;
}

_bool CCorosaurus::CheckHit()
{
	m_isAttack = false;
	if (m_beHit)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
		
		m_beHit = false;
		return true;
	}
	return false;
}

_bool CCorosaurus::isChase()
{
	if (!m_isAggro || m_iState & ENUM_CLASS(TEST_STATE::SPAWN))
		return false;

	_bool bResult{};
	if (m_isDetecting)
	{
		bResult = true;
		if (m_fDistance > 20.f)
			m_iState |= ENUM_CLASS(TEST_STATE::LAND);
		else if (m_fDistance < 3.f)
			return false;
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

_bool CCorosaurus::isPatrol()
{
	return false;
	_bool bResult = !m_isAggro;
	if (bResult)
	{
		//Patrol();
	}
	return bResult;
}

_bool CCorosaurus::Back()
{
	if(m_isAttack)
		return m_fFrontDot < 0.f;

	if (m_fDistance < 2.f && m_fFrontDot > 0.f)
		return true;
	return false;
}

_bool CCorosaurus::Front()
{
	return m_fFrontDot > 0.f;
}

_bool CCorosaurus::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.725f;
}

_bool CCorosaurus::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.725f;
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
	Safe_Release(m_pCoroRock);
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pParryVolume);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
