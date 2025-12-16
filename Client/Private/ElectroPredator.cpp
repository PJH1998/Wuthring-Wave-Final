#include "ClientPch.h"
#include "ElectroPredator.h"
#include "Projectile.h"
#include "AoEDoT.h"
#include "GameSystem.h"

CElectroPredator::CElectroPredator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor{ pDevice, pContext }
{
}

CElectroPredator::CElectroPredator(const CElectroPredator& Prototype)
	: CActor{ Prototype }
	, m_vMonsterDissolveColor{ Prototype.m_vMonsterDissolveColor }
{
}

HRESULT CElectroPredator::Initialize_Prototype()
{
	m_vMonsterDissolveColor = _float4(0.9882f, 0.3843f, 0.145f, 1.f);

	return S_OK;
}

HRESULT CElectroPredator::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pGameSystem = CGameSystem::GetInstance();
	Safe_AddRef(m_pGameSystem);
	ELECTROPREDATOR_DESC* pDesc = static_cast<ELECTROPREDATOR_DESC*>(pArg);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
#pragma region ATTACK_STATE
	m_fAttackCoolTime[0] = 8.f;
	m_fAttackCoolTime[1] = 20.f;
	m_fAttackCoolTime[2] = 10.f;
#pragma endregion

	Ready_Component(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);

	m_pCameraSocket = m_pModelCom->Get_BoneMatrixPtr("CameraPosition");
	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = m_fAttackDmg;
	m_CallBack.pCondition = &m_iState;
	//m_CallBack.strEffectTag = ;
	m_CallBack.eType = TEXT_COLOR_TYPE::ELEC;
	m_CallBack.pSocketMatrix = m_pCameraSocket;
	m_pColliderCom->Set_Desc(&m_CallBack);

	m_fHP = pDesc->fHp;
	m_fAttackDmg = pDesc->fAttackDmg;
	m_vDistanceRange = _float2(7.f, 12.95f);
	m_fIdleDuration = 30.f;
	m_fIdleAcc = 10.f;
	m_fImpluseRate = pDesc->fImpluseRate;

	m_pRigidBodyCom->IsActivate(false);
	m_pColliderCom->IsActivate(false);
	m_isActivate = false;
	m_fHitStopRatio = 1.f;
	m_vBaseColor = _float4(1.f, 1.f, 1.f, 1.f);
	//m_vMonsterDissolveColor = _float4(0.5f, 0.3f, 0.5f, 1.f);
	m_fBehitMaxTime = 0.15f;
	_float temp{};
	m_pModelCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom, pDesc->pAnimationTag, 0.f, &temp);

	return S_OK;
}

void CElectroPredator::Priority_Update(_float fTimeDelta)
{
	if (m_pGameSystem->IsSonoro())
	{
		return;
	}
	m_pTransformCom->Save_PreviousPosition();
	//m_fAttackAcc[2] = m_fAttackCoolTime[2];
}

void CElectroPredator::Update(_float fTimeDelta)
{
	if (m_pGameSystem->IsSonoro())
	{
		return;
	}
	Reset_Condition(fTimeDelta);

	// 1. Update Current State
	m_pBehaviorTreeCom->tick(this);
	if (false == m_isActivate)
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel);
		m_pGameInstance->Return_Channel(m_iSoundChannel);
		m_iSoundChannel = -1;
	}
	After_Condition(fTimeDelta);

	// 2. Setting Animation & Run
	//m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * m_fHitStopRatio); //cpu
	_float fTimeRatio = m_pGameSystem->TimeLack(COLLISIONLAYER::ENEMY);
	m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * fTimeRatio); //gpu
	//_float temp;
	//m_pModelCom->Play_Animation_CPU("Stand2", fTimeDelta, &temp, false, true, false, true, 1.f);
	//m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	// 3. Collider Update
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if (m_isPushed)
	{
		_vector vBeHitDir = XMVector3Normalize(XMLoadFloat3(&m_vBeHit_Normal) * 2.5f + XMVectorSet(0.f, 1.f, 0.f, 0.f));
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
	if (m_pGameSystem->IsSonoro())
	{
		if (!m_isSonoro)
		{
			m_pColliderCom->IsActivate(false);
			m_pRigidBodyCom->IsActivate(false);
			m_isSonoro = true;
		}
		return;
	}
	else
	{
		if (m_isSonoro)
		{
			m_pColliderCom->IsActivate(true);
			m_pRigidBodyCom->IsActivate(true);
			m_isSonoro = false;
			return;
		}
	}
	if (m_isDissolve)
	{
		if (m_fDissolveRate < 1.f)			
			m_fDissolveRate += fTimeDelta;
		else
			m_fDissolveRate = 1.f;
	}
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

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
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

	//_uint iShaderPass = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
	////IF DISSOLVE
	//{
	//	iShaderPass = ENUM_CLASS(SHADER_ANIMMESH::MONSTER_SPAWN); // or ENUM_CLASS(SHADER_ANIMMESH::MONSTER_DEAD)
	//
	//	if(FAILED(m_pShaderCom->Bind_Value("g_fDissolveRate", &m_fDesolveRate, sizeof(_float))))
	//		CRASH("Failed to Bind DissolveRate");
	//
	//	if(FAILED(m_pShaderCom->Bind_Value("g_vMonsterDissolveColor", &m_vDissovleColor, sizeof(_float4))))
	//		CRASH("Failed to Bind DissolveColor");
	//}

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			CRASH("Failed to Bind NormalTexture");
	
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, 0)))
			CRASH("Failed to Bind MaskTexture");

		//_bool HasNormal = { false };
		//if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
		//	HasNormal = true;
		//if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
		//	CRASH("Ready g_HasNormal Failed");

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		if (m_fBehitAcc < m_fBehitMaxTime)
			m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::ENEMY_BEHIT));
		else
		{
			if (m_isDissolve)
			{
				if(m_isDeadTrigger)
					m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::MONSTER_DEAD));
				else
					m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::MONSTER_SPAWN));
			}
			else
				m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
		}

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG

	//m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CElectroPredator::Render_Shadow()
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

void CElectroPredator::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	MONSTER_INFO* pDesc = static_cast<MONSTER_INFO*>(pArg);
	m_fHP = pDesc->fMaxHp;
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->Save_PreviousPosition();
	m_isActivate = true;
	m_pAnimMachineCom->Reset(m_pModelCom, "Born02");
	_float temp{};
	m_pModelCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom, "Born02", 0.f, &temp, false);
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));
	m_pColliderCom->IsActivate(true);
	m_pRigidBodyCom->IsActivate(true);
	m_isDeadTrigger = false;
	m_fDissolveRate = 0.f;
	m_isDissolve = true;
	m_iState = ENUM_CLASS(TEST_STATE::NONE);
	m_fAttackAcc[1] = 5.f;
	m_fAttackAcc[2] = 20.f;
	m_fBehitAcc = m_fBehitMaxTime;
	m_iSoundChannel = m_pGameInstance->Register_Channel();
}

void CElectroPredator::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	if (wStrColliderTag == TEXT("Lerp"))
	{
		TurnLerp(Isactive);
	}
	else if (wStrColliderTag == TEXT("Dissolve"))
	{
		m_isDissolve = Isactive;
		m_fDissolveRate = 0.f;
	}
}

void CElectroPredator::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;
	
	PREFAB_INFO Desc = {};
	Desc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	Desc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &Desc);
}

void CElectroPredator::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrPartTag = wStrObjectTag.substr(Index + 1);

	if (wstrTypeTag == TEXT("Sound"))
	{
		Sound_Active(wstrPartTag);
	}
	else if (wstrTypeTag == TEXT("Shoot"))
	{
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
		_matrix WorldMat = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), 
														XMVectorSet(0.f, 0.f, 0.f, 1.f), 
														XMVectorSet(0.f, 0.f, 0.f, 1.f),
														vPos + vLook + XMVectorSet(0.f, 2.f, 0.f, 0.f));
		CProjectile::PROJECTILERESET ProiDesc{};
		ProiDesc.vTargetPos = m_vTargetPosition;
		ProiDesc.vTargetPos.y += 0.5f; // 대상 높이 offset
		ProiDesc.pOwnerTransform = m_pTransformCom;
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Projectile_Electro"), WorldMat, &ProiDesc);
	}
	else if (wstrTypeTag == TEXT("AoE"))
	{
		_vector vScale{}, vQuat{}, vTranslate{};
		XMMatrixDecompose(&vScale, &vQuat, &vTranslate, m_pTransformCom->Get_WorldMatrix());

		vTranslate = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
		_matrix WorldMat = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuat, vTranslate);

		CAoEDoT::AOEDOT_RESET AoEDesc{};
		AoEDesc.fLifeTime = 3.f;
		AoEDesc.iTickCount = 8;

		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_AOEDOT_Electro"), WorldMat, &AoEDesc);
	}
	else if (wstrTypeTag == TEXT("Look"))
	{
		TurnFix();
	}
}

void CElectroPredator::Sound_Active(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrPartTag = wStrObjectTag.substr(Index + 1);

	if (wstrTypeTag == TEXT("Atk01"))
	{
		if (wstrPartTag == TEXT("1"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("ord_leilie_atk01_1_02 (SFX)"), m_iSoundChannel, 0.25f, m_pTransformCom, 0.f, 25.f);
		}
		else if (wstrPartTag == TEXT("2"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_attack01_impact (SFX)"), m_iSoundChannel, 0.25f, m_pTransformCom, 0.f, 25.f);
		}
	}
	else if (wstrTypeTag == TEXT("Atk02"))
	{
		if (wstrPartTag == TEXT("1"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_attack02_cast (SFX)"), m_iSoundChannel, 0.25f, m_pTransformCom, 0.f, 25.f);
		}
		else if (wstrPartTag == TEXT("2"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_attack02_impact (SFX)"), m_iSoundChannel, 0.25f, m_pTransformCom, 0.f, 25.f);
		}
	}
	else if (wstrTypeTag == TEXT("Atk03"))
	{
		if (wstrPartTag == TEXT("1"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_attack03_cast (SFX)"), m_iSoundChannel, 0.25f, m_pTransformCom, 0.f, 25.f);
		}
		else if (wstrPartTag == TEXT("2"))
		{
			m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_attack03_impact (SFX)"), m_iSoundChannel, 0.25f, m_pTransformCom, 0.f, 25.f);
		}
	}
	else if (wstrTypeTag == TEXT("Aggro"))
	{
		m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_patroltofight (SFX)"), m_iSoundChannel, 0.15f, m_pTransformCom, 0.f, 32.f);
	}
	else if (wstrTypeTag == TEXT("Death"))
	{
		m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_death (SFX)"), m_iSoundChannel, 0.2f, m_pTransformCom, 0.f, 25.f);
	}
	else if (wstrTypeTag == TEXT("Stand"))
	{
		m_pGameInstance->Play_Sound_Dynamic(TEXT("mon_leilie_stand2_action01 (SFX)"), m_iSoundChannel, 0.3f, m_pTransformCom, 0.f, 18.f);
	}
}

HRESULT CElectroPredator::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_vBaseColor", &m_vBaseColor, sizeof(_float4));
	m_pShaderCom->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4));
	if (m_fBehitAcc < m_fBehitMaxTime)
	{
		m_pShaderCom->Bind_Value("g_fMaxTime", &m_fBehitMaxTime, sizeof(_float));
		m_pShaderCom->Bind_Value("g_fCurrentTime", &m_fBehitAcc, sizeof(_float));
	}
	if (m_isDissolve)
	{
		m_pShaderCom->Bind_Value("g_fDissolveRate", &m_fDissolveRate, sizeof(_float));
		m_pShaderCom->Bind_Value("g_vMonsterDissolveColor", &m_vMonsterDissolveColor, sizeof(_float4));
	}
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
	ColliderDesc.fRayOffset = -0.15f;
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);

	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		BeHit(iLayer, pDesc, Manifold);
		});

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
#pragma region MONSTER_HP
	// 변수
	_float3 vMobPos = {};
	XMStoreFloat3(&vMobPos, m_pTransformCom->Get_State(STATE::POSITION));

	// 체력바
	UI_MOBINFO_DESC tDesc = {};
	tDesc.fMobCurHP = m_fHP;
	tDesc.fMobMaxHP = m_pGameSystem->Get_MonsterInfo("ElectroPredator")->fMaxHp;
	tDesc.isAtkedCurFrame = m_beHit;
	tDesc.pMonsterPtrKey = this;
	tDesc.vMobPos = vMobPos;
	tDesc.vMobPos.y += 1.25f;
	m_pGameSystem->Update_MobStatus(tDesc);
#pragma endregion

	if (m_fBehitAcc < m_fBehitMaxTime)
		m_fBehitAcc += fTimeDelta;
	if (m_fHP <= 0.f)
	{
		m_iState = ENUM_CLASS(TEST_STATE::DEAD);
		m_fBehitAcc = m_fBehitMaxTime;
		//return;
	}
}

void CElectroPredator::After_Condition(_float fTimeDelta)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
	{
		if (!m_isDeadTrigger)
		{
			m_isDeadTrigger = true;
			m_pColliderCom->IsActivate(false);
			m_pRigidBodyCom->IsActivate(false);
		}
		//return;
	}
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
#pragma region UI_BIND
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 1.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(m_fBehitDMG), m_eBehitColor, 0.4f);
#pragma endregion

#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.5f, 0.f), &EffectDesc);

		if (!m_strBehitSound.empty())
		{
			m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel);
			m_pGameInstance->Play_Sound_Dynamic(m_strBehitSound, m_iSoundChannel, 0.4f);
		}
#pragma endregion
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
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
		return;
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK) || iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL) || iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		m_beHit = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fBehitDMG = pDesc->fAttack * m_pGameInstance->Rand(0.75f, 1.5f);
		m_fHP -= m_fBehitDMG;
		m_fBehitAcc = 0.f;
		if (m_isDissolve)
			m_isDissolve = false;
		m_eBehitColor = pDesc->eType;
		if (!pDesc->strSoundTag.empty())
			m_strBehitSound = pDesc->strSoundTag;

#pragma region PHYSICS
		XMStoreFloat3(&m_vBeHit_Normal, XMLoadFloat3(&m_vTargetDir) * -1.f);
#pragma endregion
		if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
		{
#ifdef _DEBUG
			cout << "Be Hit! (Electro Predator)" << endl;
			//m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
#endif // _DEBUG
		}
		else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
		{
			if (pDesc->eDir == ATTACKVOULME_DIR::UPPER)
			{
				m_iState |= ENUM_CLASS(TEST_STATE::AIR);
			}
#ifdef _DEBUG
			cout << "Be Hit! SKILL (Electro Predator)" << endl;
#endif // _DEBUG
		}
		else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
		{
			m_isPushed = true;
			m_iState |= ENUM_CLASS(TEST_STATE::AIR);

#ifdef _DEBUG
			cout << "Knock Back! (Electro Predator)" << endl;
			cout << "Nomal- x: " << m_vBeHit_Normal.x << ", y: " << m_vBeHit_Normal.y << ", z: " << m_vBeHit_Normal.z << endl;
#endif // _DEBUG
		}
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

_bool CElectroPredator::isAnimationRunning()
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD) && !m_isDeadTrigger)
		return true;
	return !m_isAnimationFinished;
}

_bool CElectroPredator::isKnockDown()
{
	if (m_beHit)
		return true;

	return m_iState & (ENUM_CLASS(TEST_STATE::BEHIT) | ENUM_CLASS(TEST_STATE::BLOCK) | ENUM_CLASS(TEST_STATE::AIR));
}

_bool CElectroPredator::isAttackEnable()
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
		return false;
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
	if (m_iState & (ENUM_CLASS(TEST_STATE::SPAWN) | ENUM_CLASS(TEST_STATE::DEAD)))
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

	Safe_Release(m_pGameSystem);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
