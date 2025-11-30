#include "ClientPch.h"
#include "Leviatan.h"
#include "AttackVolume.h"
#include "Levi_Bayonet.h"
#include "GameSystem.h"

CLeviatan::CLeviatan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CLeviatan::CLeviatan(const CLeviatan& Prototype)
	: CActor { Prototype }
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLeviatan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLeviatan::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	LEVIATAN_DESC* pDesc = static_cast<LEVIATAN_DESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pDesc->vInitRotate.x), XMConvertToRadians(pDesc->vInitRotate.y), XMConvertToRadians(pDesc->vInitRotate.z));
	m_pTransformCom->Rotation_Quaternion(vQuat);
	m_fHP = pDesc->fHP;
	m_fAttackDmg = pDesc->fAttackDmg;
	m_fMaxStamina = pDesc->fMaxStamina;
	m_fStamina = m_fMaxStamina;

#pragma region ATTACK_STATE
	m_fAttackCoolTime[PHASE::ONE][ATK_PATTERN::ATTACK3] = 25.f;
	m_fAttackCoolTime[PHASE::ONE][ATK_PATTERN::ATTACK5] = 25.f;
	m_fAttackCoolTime[PHASE::ONE][ATK_PATTERN::ATTACK12] = 40.f;
	m_fAttackCoolTime[PHASE::ONE][ATK_PATTERN::ATTACK13] = 40.f;
	m_fAttackCoolTime[PHASE::ONE][ATK_PATTERN::BURST] = 90.f;
	m_fAttackCoolTime[PHASE::ONE][ATK_PATTERN::ATTACK18] = 40.f;

	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK3] = 25.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK5] = 25.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK12] = 40.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK13] = 40.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::BURST] = 120.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK18] = 40.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK1] = 80.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK20] = 80.f;
	m_fAttackCoolTime[PHASE::TWO][ATK_PATTERN::ATTACK22] = 80.f;
#pragma endregion
	Ready_Component(pDesc);
	Ready_PartObjects(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);
	_float temp{};
	m_pModelCom->Play_Animation_CPU(pDesc->pAnimationTag, 0.f, &temp);
	//m_iPhase = PHASE::TWO;
	
	m_fParalysisAcc = 5.f;
	m_fHitStopRatio = 1.f;
	m_ShaderIndices[LEVIATAN_SHADER::FX] = ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL);
	m_isRender = true;
	return S_OK;
}

void CLeviatan::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
	if (m_isTrigger == true)
		m_isDetecting = true;
	else
		m_isDetecting = false;
	m_isTrigger = false;

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Priority_Update(fTimeDelta);
	}
}

void CLeviatan::Update(_float fTimeDelta)
{
	Reset_Condition(fTimeDelta);
	// 1. 행동트리로 상태 갱신
	if(m_pBehaviorTreeCom[m_iPhase])
	{
		if (m_iState & ENUM_CLASS(TEST_STATE::SPLINT))
		{
			//1페 사망 애니메이션 진행 중
		}
		else
			m_pBehaviorTreeCom[m_iPhase]->tick(this);
	}
	After_Condition(fTimeDelta);

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	if(m_pAnimMachineCom[m_iPhase])
		m_pAnimMachineCom[m_iPhase]->Update(m_pModelCom, m_pComputeShaderCom, m_pFacialComputeShaderCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta); // gpu
	//m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * m_fHitStopRatio); //cpu
	else
	{
		_float temp{};
		m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_pFacialComputeShaderCom, "Stand2", fTimeDelta, &temp);
	}
	

	//3. 거리 보간
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if (m_isDist_Interp_Enable)
	{
		_float temp = clamp(m_fDistanceNonY - 1.5f, 0.f, 1.f);
		m_pColliderCom->Update(vVelocity / fTimeDelta * temp);
	}
	else
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	
	// 탐지 볼륨 갱신
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	//4. 충돌 상호작용 볼륨 갱신
	for (_uint i = 0; i < ATK_SOCKET::ATKEND; i++)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Update(fTimeDelta);
	}
	if(m_pParryVolume)
		m_pParryVolume->Update(fTimeDelta);

	//5. 파츠 갱신
	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Update(fTimeDelta);
	}
}

void CLeviatan::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_fStamina <= 0.f && m_fParalysisAcc >= 5.f)
		m_isParalysis = true;

	if(m_isRender)
		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this))) return;

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Late_Update(fTimeDelta);
	}
}

void CLeviatan::Render()
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
		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		_bool HasMask = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, 0)))
			HasMask = true;

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		if (FAILED(m_pShaderCom->Bind_Value("g_HasSkinMask", &HasMask, sizeof(_bool))))
			CRASH("Ready g_HasSkinMask Failed");

		if (FAILED(m_pModelCom->Bind_MorphedResult(m_pShaderCom, i, "g_MorphedVertices")))
			CRASH("Bind Morph Result Failed");

		m_pShaderCom->Begin(m_ShaderIndices[i]);

		m_pModelCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
	}

#ifdef _DEBUG
#pragma region ATTACK_VOLUME
	for (_uint i = 0; i < ATK_SOCKET::ATKEND; ++i)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Render();
	}
	if (m_pParryVolume)
		m_pParryVolume->Render();
#pragma endregion
	m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CLeviatan::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::SHADOW));

		m_pModelCom->Render(i);
	}
}

void CLeviatan::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		m_isTrigger = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		//if (false == m_isAggro)
		//	m_isAggro = true;
	}
}

void CLeviatan::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	MONSTER_INFO Info = *m_pGameSystem->Get_MonsterInfo("Leviatan");
	m_fHP = Info.fMaxHp;
	m_fStamina = m_fMaxStamina;
	m_fParalysisAcc = 5.f;
	m_fHitStopRatio = 1.f;
	m_isRender = true;
}

void CLeviatan::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);

	if (wstrTypeTag == TEXT("Attack"))
	{
		if (wstrPartTag == TEXT("FL"))
			m_pAtkVolumes[ATK_SOCKET::FOOT_L]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("FR"))
			m_pAtkVolumes[ATK_SOCKET::FOOT_R]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("RAY"))
		{
			//m_pAtkVolumes[ATK_SOCKET::RAY1]->TriggerActivate(Isactive);
		}
		else if (wstrPartTag == TEXT("G"))
		{
			m_pAtkVolumes[ATK_SOCKET::WEAPON_GL]->TriggerActivate(Isactive);
		}
	}
	else if (wstrTypeTag == TEXT("Parry"))
	{
		if (m_pParryVolume)
			m_pParryVolume->TriggerActivate(Isactive);
	}
	else if (wstrTypeTag == TEXT("Gravity"))
	{
		m_pColliderCom->Set_Gravity(Isactive);
	}
	else if (wstrTypeTag == TEXT("Lerp"))
	{
		m_isTurnLerp = Isactive;
	}
	else if (wstrTypeTag == TEXT("Distance"))
	{
		m_isDist_Interp_Enable = Isactive;
	}
	else if (wstrTypeTag == TEXT("Visible"))
	{
		m_isRender = Isactive;
	}
}

void CLeviatan::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CLeviatan::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrAnimTag = wStrObjectTag.substr(Index + 1);
	if (false)
	{

	}
	else if (wstrTypeTag == TEXT("Type"))
	{
		if (wstrAnimTag == TEXT("ATK"))
		{
			for (auto& pATKVolume : m_pAtkVolumes)
				pATKVolume->Change_Layer(COLLISIONLAYER::ENEMY_ATTACK);
		}
		else if (wstrAnimTag == TEXT("HARD"))
		{
			for (auto& pATKVolume : m_pAtkVolumes)
				pATKVolume->Change_Layer(COLLISIONLAYER::ENEMY_HARDATTACK);
		}
		else if (wstrAnimTag == TEXT("SKILL"))
		{
			for (auto& pATKVolume : m_pAtkVolumes)
				pATKVolume->Change_Layer(COLLISIONLAYER::ENEMY_SKILL);
		}
	}
	else if (wstrTypeTag == TEXT("Look"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	}
	else if (wstrTypeTag == TEXT("LookRev"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir) * -1.f);
		_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(0.f), 0.f);
		m_pTransformCom->Turn_Quaternion(vQuat);
	}
	else if (wstrTypeTag == TEXT("Reset"))
	{
		Reset_NotifyInteraction();
	}
}

HRESULT CLeviatan::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	return S_OK;
}

void CLeviatan::Ready_Component(LEVIATAN_DESC* pDesc)
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

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = m_fAttackDmg;
	m_CallBack.pCondition = &m_iState;
	//m_tCallDesc.strEffectTag = ;
	m_CallBack.eType = TEXT_COLOR_TYPE::DARK;
	m_pColliderCom->Set_Desc(&m_CallBack);
	m_pColliderCom->Set_Gravity(true);

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Leviatan/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("Leviatan/Com_ComputeShader");

	// Com_ComputeShaderFacial
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Shader_ComputeVtxAnimMorph"), TEXT("Com_ComputeShaderFacial"), reinterpret_cast<CComponent**>(&m_pFacialComputeShaderCom), nullptr)))
		CRASH("Com_ComputeShaderFacial");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Leviatan/Com_Model");
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::AUGUSTA));

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag.assign(pDesc->pAnimationTag);
	//Com_AnimMachine
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_AnimMachine_Leviatan_Phase1"),
		TEXT("Com_AnimMachine1"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom[PHASE::ONE]), &AnimMachineDesc)))
		CRASH("Leviatan/Com_AnimMachine1");

	//Com_AnimMachine
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_AnimMachine_Leviatan_Phase2"),
		TEXT("Com_AnimMachine2"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom[PHASE::TWO]), &AnimMachineDesc)))
		CRASH("Leviatan/Com_AnimMachine2");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard1 = CBlackBoard::Create();
	pBlackBoard1->Add_Data("iState", pBlackBoard1->DeduceType(m_iState), &m_iState);
	pBlackBoard1->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard1->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard1->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard1->Add_Condition("DodgeCooldown", [this]() ->_bool { return DodgeCooldown(); });
	pBlackBoard1->Add_Condition("ATKArrange", [this]() ->_bool { return Attack_Arrange(); });
	pBlackBoard1->Add_Condition("Attack03", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK3, 3.f); });
	pBlackBoard1->Add_Condition("Attack05", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK5, 4.f); });
	pBlackBoard1->Add_Condition("Attack12", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK12, 5.f); });
	pBlackBoard1->Add_Condition("Attack13", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK13, 6.f); });
	pBlackBoard1->Add_Condition("Attack15", [this]() ->_bool { return Attack(ATK_PATTERN::BURST, 15.f); });
	pBlackBoard1->Add_Condition("Attack18", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK18, 10.f); });
	pBlackBoard1->Add_Condition("BeHit", [this]() ->_bool { return CheckHit(); });
	pBlackBoard1->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard1->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard1->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard1->Add_Condition("Right", [this]() ->_bool { return Right(); });
	
	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard1;
	//Com_BehaviorTree
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_BehaviorTree_Leviatan1"),
		TEXT("Com_BehaviorTree1"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom[PHASE::ONE]), &BTDesc)))
		CRASH("Leviatan/Com_BehaviorTree1");

	CBlackBoard* pBlackBoard2 = CBlackBoard::Create();
	pBlackBoard2->Add_Data("iState", pBlackBoard2->DeduceType(m_iState), &m_iState);
	pBlackBoard2->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard2->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard2->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard2->Add_Condition("DodgeCooldown", [this]() ->_bool { return DodgeCooldown(); });
	pBlackBoard2->Add_Condition("ATKArrange", [this]() ->_bool { return Attack_Arrange(); });
	pBlackBoard2->Add_Condition("Attack01", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK1, 3.f); });
	pBlackBoard2->Add_Condition("Attack03", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK3, 4.f); });
	pBlackBoard2->Add_Condition("Attack05", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK5, 5.f); });
	pBlackBoard2->Add_Condition("Attack12", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK12, 6.f); });
	pBlackBoard2->Add_Condition("Attack13", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK13, 8.f); });
	pBlackBoard2->Add_Condition("Attack14", [this]() ->_bool { return Attack(ATK_PATTERN::BURST, 100.f); });
	pBlackBoard2->Add_Condition("Attack18", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK18, 10.f); });
	pBlackBoard2->Add_Condition("Attack20", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK20, 10.f); });
	pBlackBoard2->Add_Condition("Attack22", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK22, 10.f); });
	pBlackBoard2->Add_Condition("BeHit", [this]() ->_bool { return CheckHit(); });
	pBlackBoard2->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard2->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard2->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard2->Add_Condition("Right", [this]() ->_bool { return Right(); });

	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc2{};
	BTDesc2.pBlackBoard = pBlackBoard2;
	//Com_BehaviorTree
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_BehaviorTree_Leviatan2"),
		TEXT("Com_BehaviorTree2"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom[PHASE::TWO]), &BTDesc2)))
		CRASH("Leviatan/Com_BehaviorTree2");
#pragma endregion
}

void CLeviatan::Ready_PartObjects(LEVIATAN_DESC* pDesc)
{
	CLevi_Bayonet::LEVIBAYONET_DESC BayonetDesc{};
	BayonetDesc.eType = TEXT_COLOR_TYPE::DARK;
	BayonetDesc.fAttackDmg = m_fAttackDmg;
	BayonetDesc.pParentTransform = m_pTransformCom;
	BayonetDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp02");
	BayonetDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	BayonetDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));

	if (FAILED(CContainerObject::Add_PartObject(TEXT("Part_Bayonet"), ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_GameObject_Levi_Bayonet"), &BayonetDesc)))
		CRASH("Failed to Add Part : Bayonet");
}

void CLeviatan::Calculate_PosAndDir()
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

void CLeviatan::Reset_Condition(_float fTimeDelta)
{
	if (m_fHP <= 0.f)
	{
		if (m_iPhase == 0)
		{
			m_iState = ENUM_CLASS(TEST_STATE::SPLINT);
			m_fHP = 1.f;
			m_pColliderCom->IsActivate(false);
			m_pRigidBodyCom->IsActivate(false);
		}
		else
			m_iState = ENUM_CLASS(TEST_STATE::DEAD);
		return;
	}
	if (m_isAnimationFinished)
	{
		_uint iRemainState{};
		if (m_iState & ENUM_CLASS(TEST_STATE::BLOCK))
			iRemainState |= ENUM_CLASS(TEST_STATE::BLOCK);
		m_iState = ENUM_CLASS(TEST_STATE::NONE);

		m_iState |= iRemainState;

	}
	if (m_isDetecting)
	{
		Calculate_PosAndDir();
#ifdef _DEBUG
		//cout << "x : " << m_vTargetPosition.x << " y : " << m_vTargetPosition.y << " z : " << m_vTargetPosition.z << endl;
		//cout << "distance: " << m_fDistance << endl;
#endif
	}
	
	for (_uint i = 0; i < ATK_PATTERN::ATK_END; ++i)
	{
		if (m_fAttackAcc[m_iPhase][i] > 0.f)
			m_fAttackAcc[m_iPhase][i] -= fTimeDelta;
	}
	
	if (m_fDodgeCoolTime > 0.f)
		m_fDodgeCoolTime -= fTimeDelta;

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
		m_isKnockDownTrig = m_isParalysis;
}

void CLeviatan::After_Condition(_float fTimeDelta)
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
	else if (m_iState & ENUM_CLASS(TEST_STATE::SPLINT))
	{

	}
	if (m_isTurnLerp)
		m_pTransformCom->LookLerp(XMLoadFloat3(&m_vTargetDir), fTimeDelta);

	//그로기 특수상황
	if (m_isParalysis)
	{
		if (m_isKnockDownTrig)
			m_iState = ENUM_CLASS(TEST_STATE::PARALYSIS);
		else
		{
			m_iState = (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::MOVE_FORWARD));
			m_isKnockDownTrig = true;
		}
	}
	else
		m_isKnockDownTrig = m_isParalysis;

	if (m_beHit)
		m_beHit = false;
}

void CLeviatan::OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		if (m_isAggro)
			return;
		//UI Binding (몬스터 데이터 찾기용 키값, 현재 체력 변수 주소, 현재 무력화게이지 변수 주소, 텍스트 출력용 한글 wtring)
		m_pGameSystem->HUD_Bind_BossStatus(TEXT("명식 레비아탄"), "Leviatan", &m_fHP, &m_fStamina, &m_isParalysis, &m_fParalysisRatio);
		m_pGameSystem->HUD_Toggle_BossStatusUI(true);
		m_isAggro = true;
	}
}

void CLeviatan::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
		return;
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK) || iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL) || iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 1.f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		m_fHP -= pDesc->fAttack;
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);
#pragma endregion
		if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
		{
#ifdef _DEBUG
			cout << "Be Hit! (Leviatan)" << endl;
#endif // _DEBUG
		}
		else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
		{
#ifdef _DEBUG
			cout << "Be Hit! SKILL (Leviatan)" << endl;
#endif // _DEBUG
		}
		else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
		{
#ifdef _DEBUG
			cout << "Be Hit! KNOCKBACK (Leviatan)" << endl;
#endif // _DEBUG
		}
	}
}

void CLeviatan::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer)
{
}

void CLeviatan::ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	m_iState |= ENUM_CLASS(TEST_STATE::BLOCK);
	memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));
#ifdef _DEBUG
	cout << "Parry! Leviatan)" << endl;
#endif // _DEBUG
}

void CLeviatan::TurnFix()
{
	m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
}

void CLeviatan::TurnLerp(_bool isActive)
{
	m_isTurnLerp = isActive;
}

void CLeviatan::DistanceInterpolate(_bool isActive)
{
	m_isDist_Interp_Enable = isActive;
}

void CLeviatan::Reset_NotifyInteraction()
{
	m_isTurnLerp = false;
	m_isDist_Interp_Enable = false;
	m_isRender = true;
	m_pColliderCom->Set_Gravity(true);
	for (_uint i = 0; i < ATK_SOCKET::ATKEND; i++)
	{
		if(nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->SetActivate(false);
	}
	if (nullptr != m_pParryVolume)
		m_pParryVolume->SetActivate(false);
	for (auto& Pair : m_PartObjects)
	{

	}
}

_bool CLeviatan::isKnockDown()
{
	if (m_isParalysis)
		return true;

	return m_iState & (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::BLOCK));
}

_bool CLeviatan::isAttackEnable()
{
	if (!m_isDetecting || !m_isAggro)
		return false;
	_bool bResult{};
	if (m_iPhase == PHASE::ONE)
	{
		for (_uint i = 0; i < ATK_PATTERN::ATTACK1; ++i)
			if (m_fAttackAcc[m_iPhase][i] < 0.f) 
				bResult = true;
	}
	else if (m_iPhase == PHASE::TWO)
	{
		for (_uint i = 0; i < ATK_PATTERN::ATK_END; ++i)
			if (m_fAttackAcc[m_iPhase][i] < 0.f)
				bResult = true;
	}

	return bResult;
}

_bool CLeviatan::DodgeCooldown()
{
	_bool Result = m_isDetecting && m_fDodgeCoolTime <= 0.f && m_fDistanceNonY < 3.f;
	if (Result)
		m_fDodgeCoolTime = 7.f;
	return Result;
}

_bool CLeviatan::Attack(_uint iIndex, _float fInterval)
{
	_bool bResult = (m_fAttackAcc[m_iPhase][iIndex] <= 0.f) && m_fDistanceNonY < fInterval;
	if (bResult)
	{
		m_fAttackAcc[m_iPhase][iIndex] = m_fAttackCoolTime[m_iPhase][iIndex];
		//Attack_Arrange();
	}
	return bResult;
}

_bool CLeviatan::Attack_Arrange()
{
	_float fRand = m_pGameInstance->Rand_Normal();
	if (fRand < 0.5f)
		m_iState |= ENUM_CLASS(TEST_STATE::MOVE_FORWARD);
	return true;
}

_bool CLeviatan::CheckHit()
{
	if (m_beHit)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);

		m_beHit = false;
		return true;
	}
	return false;
}

_bool CLeviatan::Back()
{
	return m_fFrontDot < 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CLeviatan::Front()
{
	return m_fFrontDot > 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CLeviatan::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.525f;
}

_bool CLeviatan::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.525f;
}

CLeviatan* CLeviatan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLeviatan* pInstance = new CLeviatan(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Leviatan");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLeviatan::Clone(void* pArg)
{
	CLeviatan* pClone = new CLeviatan(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Leviatan (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLeviatan::Free()
{
	__super::Free();
	
	for (_uint i = 0; i < ATK_SOCKET::ATKEND; ++i)
		Safe_Release(m_pAtkVolumes[i]);

	Safe_Release(m_pGameSystem);
	Safe_Release(m_pFacialComputeShaderCom);
	Safe_Release(m_pParryVolume);
	for (_uint i = 0; i < PHASE::P_END; ++i)
		Safe_Release(m_pBehaviorTreeCom[i]);
	for (_uint i = 0; i < PHASE::P_END; ++i)
		Safe_Release(m_pAnimMachineCom[i]);
}
