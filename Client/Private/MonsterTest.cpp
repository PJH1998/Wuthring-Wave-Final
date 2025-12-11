#include "ClientPch.h"
#include "MonsterTest.h"
#include "Ggobul.h"
#include "FS_Scythe.h"
#include "AttackVolume.h"
#include "Projectile.h"
#include "GameSystem.h"

CMonsterTest::CMonsterTest(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CMonsterTest::CMonsterTest(const CMonsterTest& Prototype)
	: CActor { Prototype }
	, m_pGameSystem { CGameSystem::GetInstance() }
	, m_vOutLineColor { Prototype.m_vOutLineColor }
	, m_fOutLineRadius { Prototype.m_fOutLineRadius }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMonsterTest::Initialize_Prototype()
{
	m_vOutLineColor = _float4(0.9535f, 0.9015f, 0.3218f, 1.f);
	m_fOutLineRadius = 0.05f;

	return S_OK;
}

HRESULT CMonsterTest::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MONSTERTEST_DESC* pDesc = static_cast<MONSTERTEST_DESC*>(pArg);

	//m_pTransformCom->Scale({ 1.f, 1.f, 1.f});
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	_vector vQuat = XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pDesc->vInitRotate.x), XMConvertToRadians(pDesc->vInitRotate.y), XMConvertToRadians(pDesc->vInitRotate.z));
	m_pTransformCom->Rotation_Quaternion(vQuat);
#pragma region ATTACK_STATE
	m_fAttackCoolTime[ATK_PATTERN::ATTACK1] = 3.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK2] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK3] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK4] = 5.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK5] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK6] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK7] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK9] = 7.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK10] = 4.f;
	m_fAttackCoolTime[ATK_PATTERN::ATTACK11] = 3.f;
#pragma endregion
	Ready_Component(pDesc);
	Ready_PartObjects(pDesc);
	CActor::Register_AllNotifies(pDesc->strFolderPath);
	///////////////////////
	//m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
	/////////////////////
	_float temp{};
	m_pModelCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom, pDesc->pAnimationTag, 0.f, &temp);

	m_pToeMatrix = m_pModelCom->Get_BoneMatrixPtr("Bip001RToe0");
	m_pCameraMatrix = m_pModelCom->Get_BoneMatrixPtr("CameraPosition");

	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = m_fAttackDmg;
	m_CallBack.pCondition = &m_iState;
	//m_tCallDesc.strEffectTag = ;
	m_CallBack.eType = TEXT_COLOR_TYPE::ELEC;
	m_CallBack.pSocketMatrix = m_pCameraMatrix;
	m_pColliderCom->Set_Desc(&m_CallBack);

	m_fHP = pDesc->fHP;
	m_fAttackDmg = pDesc->fAttackDmg;
	m_fMaxStamina = pDesc->fMaxStamina;
	m_fStamina = m_fMaxStamina;
	m_fParalysisAcc = 5.f;
	m_fHitStopRatio = 1.f;
	m_ShaderIndices[SHINWANG_SHADER::FX] = ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL);
	m_ShaderIndices[SHINWANG_SHADER::FX2] = ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL);
	m_vBaseColor = _float4(1.f, 1.f, 1.f, 1.f);
	m_isRender = false;
	m_pTransformCom->Save_PreviousPosition();
	return S_OK;
}

void CMonsterTest::Priority_Update(_float fTimeDelta)
{
	if (!m_isAggro)
		return;
	m_pTransformCom->Save_PreviousPosition();
	if(m_isTrigger == true)
		m_isDetecting = true;
	else
		m_isDetecting = false;
	m_isTrigger = false;
}

void CMonsterTest::Update(_float fTimeDelta)
{
	if (!m_isAggro)
		return;

	Reset_Condition(fTimeDelta);
	// 1. 행동트리로 상태 갱신
	m_pBehaviorTreeCom->tick(this);

	if (false == m_isActivate)
	{
		//소멸 트리거, 포탈 생성
		m_pGameSystem->Set_Potal_Active(true);
	}

	After_Condition(fTimeDelta);

	// 2. 상태 플래그에 맞는 애니메이션 변경	3. 애니메이션 재생
	_float fTimeRatio = m_pGameSystem->TimeLack(COLLISIONLAYER::ENEMY);
	m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * fTimeRatio); // gpu
	//m_pAnimMachineCom->Update(m_pModelCom, m_pTransformCom, &m_iState, m_isAnimationFinished, fTimeDelta * m_fHitStopRatio); //cpu
	//_float temp{};
	//m_pModelCom->Play_Animation_CPU("Attack04", fTimeDelta, &temp);
	if (m_iState & ENUM_CLASS(TEST_STATE::BLOCK))
		m_iState &= ~ENUM_CLASS(TEST_STATE::BLOCK);
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if(m_isDist_Interp_Enable)
	{
		_float temp = clamp(m_fDistanceNonY - 1.3f, 0.f,1.f);
		m_pColliderCom->Update(vVelocity / fTimeDelta * temp);
		//m_isDist_Interp_Enable = false;
	}
	else
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	// y축 수직 회전 lerp 사용할 함수 : CTransform->LookLerp
	// y축 수직으로  look fix할 함수 : CTransform->LookDir
#pragma region ATTACK_VOLUME
	for (_uint i = 0; i < ATK_SOCKET::END; ++i)
	{
		if(nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Update(fTimeDelta);
	}
	m_pParryVolume->Update(fTimeDelta);
#pragma endregion
}

void CMonsterTest::Late_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	//if(KEYSTATE::DOWN == m_pGameInstance->Get_DIKeyState(DIK_APOSTROPHE))
	//	m_isParalysis = true;
#endif // _DEBUG
	if (!m_isAggro)
		return;
	if(m_fStamina <= 0.f && m_fParalysisAcc >= 5.f)
		m_isParalysis = true;
	//m_pRigidBodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_isDesolve)
	{
		if (m_fDesolveRate < 1.f)
			m_fDesolveRate += fTimeDelta;
		else
			m_fDesolveRate = 1.f;
	}

	if (m_isRender)
	{
		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
			return;

		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE_NONCOMPARE, this)))
			return;

		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
			return;
	}
}

void CMonsterTest::Render()
{
	if(FAILED(Bind_Resources()))
		CRASH("Falied to Bind Resources");


	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for(_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		_bool HasNormal{ false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;
		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		//m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL));

		m_pShaderCom->Begin(m_ShaderIndices[i]);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
#pragma region ATTACK_VOLUME
	for (_uint i = 0; i < ATK_SOCKET::END; ++i)
	{
		if (nullptr != m_pAtkVolumes[i])
			m_pAtkVolumes[i]->Render();
	}
	m_pParryVolume->Render();
#pragma endregion
	m_pRigidBodyCom->Render();
	m_pColliderCom->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif
}

void CMonsterTest::Render_Shadow()
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

void CMonsterTest::Render_OutLine()
{
	Bind_Resources();
	
	m_pShaderCom->Bind_Value("g_vOutLineColor", &m_vOutLineColor, sizeof(_float4));
	m_pShaderCom->Bind_Value("g_fOutLineRadius", &m_fOutLineRadius, sizeof(_float));

	m_pShaderCom->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW));

	_uint iNumMeshes = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::BOSS_OUTLINE))))
			CRASH("Ready Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");
	}
}

void CMonsterTest::OnCollide_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if(iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{

		m_isTrigger = true;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		if (false == m_isAggro)
			m_isAggro = true;
	}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::ENEMY)){}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::NONE)){}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK)){}
	else if(iLayer == ENUM_CLASS(COLLISIONLAYER::DETECT)){}
	//else
	//	m_isTrigger = false;
}

void CMonsterTest::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);
	
	if (wstrTypeTag == TEXT("Attack"))
	{
		if(wstrPartTag == TEXT("HR"))
			m_pAtkVolumes[ATK_SOCKET::WEAPON_R]->TriggerActivate(Isactive);
		else if(wstrPartTag == TEXT("HL"))
			m_pAtkVolumes[ATK_SOCKET::WEAPON_L]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("WR"))
			m_pAtkVolumes[ATK_SOCKET::WHIP_R]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("WL"))
			m_pAtkVolumes[ATK_SOCKET::WHIP_L]->TriggerActivate(Isactive);
		else if (wstrPartTag == TEXT("G"))
		{
			m_pAtkVolumes[ATK_SOCKET::WEAPON_GR]->TriggerActivate(Isactive);
			m_pAtkVolumes[ATK_SOCKET::WEAPON_GL]->TriggerActivate(Isactive);
		}
	}
	else if (wstrTypeTag == TEXT("Parry"))
	{
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
	else if (wstrTypeTag == TEXT("Render"))
	{
		m_isRender = Isactive;
	}
}

void CMonsterTest::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;
	
	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CMonsterTest::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrAnimTag = wStrObjectTag.substr(Index + 1);
	if (wstrTypeTag == TEXT("Sound"))
	{
		Sound_Active(wstrAnimTag);
	}
	else if(wstrTypeTag == TEXT("GGOBUL"))
	{
		CGgobul::GGOBUL_RESET Desc{};
		
		//Desc.pWorldMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		//Desc.vInitPosition = m_vTargetPosition;
		//XMStoreFloat3(&Desc.vInitDirection,m_pTransformCom->Get_State(STATE::LOOK));
		Desc.strPatternKey = WStringToString(wstrAnimTag);
		Desc.eType = CGgobul::GGOBULTYPE::HEAD;
		_matrix WorldMatrix;

		_vector vLook;
		if (wstrAnimTag == TEXT("SAttack03"))
		{
			vLook = XMLoadFloat3(&m_vTargetDir);
			WorldMatrix.r[ENUM_CLASS(STATE::POSITION)] = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
		}
		else if (wstrAnimTag == TEXT("SAttack02_2"))
		{
			vLook = m_pTransformCom->Get_State(STATE::LOOK);
			WorldMatrix.r[ENUM_CLASS(STATE::POSITION)] = m_pTransformCom->Get_State(STATE::POSITION);
			Desc.pRootMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		}
		else
		{
			vLook = m_pTransformCom->Get_State(STATE::LOOK);
			WorldMatrix.r[ENUM_CLASS(STATE::POSITION)] = m_pTransformCom->Get_State(STATE::POSITION);
		}

		_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		_vector vUp = XMVector3Cross(vLook, vRight);
		WorldMatrix.r[ENUM_CLASS(STATE::RIGHT)] = vRight;
		WorldMatrix.r[ENUM_CLASS(STATE::UP)] = vUp;
		WorldMatrix.r[ENUM_CLASS(STATE::LOOK)] = vLook;
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Ggobul"), WorldMatrix, &Desc);
	}
	else if (wstrTypeTag == TEXT("SCYTHE"))
	{
		CFS_Scythe::SCYTHE_RESET Desc{};
		Desc.strPatternKey = WStringToString(wstrAnimTag);

		_matrix WorldMatrix;
		_vector vLook;
		vLook = m_pTransformCom->Get_State(STATE::LOOK);
		_vector vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
		_vector vUp = XMVector3Cross(vLook, vRight);
		WorldMatrix.r[ENUM_CLASS(STATE::RIGHT)] = vRight;
		WorldMatrix.r[ENUM_CLASS(STATE::UP)] = vUp;
		WorldMatrix.r[ENUM_CLASS(STATE::LOOK)] = vLook;
		WorldMatrix.r[ENUM_CLASS(STATE::POSITION)] = m_pTransformCom->Get_State(STATE::POSITION);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Scythe"), WorldMatrix, &Desc);
	}
	else if (wstrTypeTag == TEXT("Shoot"))
	{
		_vector vScale{}, vQuat{}, vTrans{};
		_matrix SocketMatrix = XMLoadFloat4x4(m_pToeMatrix);
		XMMatrixDecompose(&vScale, &vQuat, &vTrans, SocketMatrix);
		_float4 vSocketPos{};
		XMStoreFloat4(&vSocketPos, vTrans);
		_matrix WorldMatrix = XMMatrixTranslation(vSocketPos.x, vSocketPos.y, vSocketPos.z) * m_pTransformCom->Get_WorldMatrix();
		CProjectile::PROJECTILERESET ProiDesc{};
		ProiDesc.vTargetPos = m_vTargetPosition;
		//ProiDesc.vTargetPos.y += 0.5f; // 대상 높이 offset
		ProiDesc.pOwnerTransform = m_pTransformCom;
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Projectile_ShinWang"), WorldMatrix, &ProiDesc);
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
	else if (wstrTypeTag == TEXT("Parry"))
	{
		m_pGameSystem->Attach_Parry(&m_vUIPosition);
	}
	else if (wstrTypeTag == TEXT("Look"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	}
	else if (wstrTypeTag == TEXT("LookRev"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir) * -1.f);
		_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(30.f), 0.f);
		m_pTransformCom->Turn_Quaternion(vQuat);
	}
}

void CMonsterTest::Sound_Active(const _wstring& wStrSoundTag)
{
	size_t Index = wStrSoundTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrSoundTag.substr(0, Index);
	_wstring wstrPartTag = wStrSoundTag.substr(Index + 1);
}

HRESULT CMonsterTest::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_vBaseColor", &m_vBaseColor, sizeof(_float4));

	return S_OK;
}

void CMonsterTest::Ready_Component(MONSTERTEST_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vDetectRange;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	
	if(FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
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

	m_pColliderCom->Set_Gravity(true);


	// Com_Shader
	if(FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("MonsterTest/Com_Shader");
	
	// Com_ComputeShader
	if(FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("MonsterTest/Com_ComputeShader");

	// Com_Model
	if(FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("MonsterTest/Com_Model");
	//m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::AUGUSTA));

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag.assign(pDesc->pAnimationTag);
	//Com_AnimMachine
	if(FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_AnimMachine_FalseSovereign"),
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("MonsterTest/Com_AnimMachine");

#pragma region BlackBoard_Value_&_Condition
	CBlackBoard* pBlackBoard = CBlackBoard::Create();
	pBlackBoard->Add_Data("iState", pBlackBoard->DeduceType(m_iState), &m_iState);
	pBlackBoard->Add_Condition("isAnimationRunning", [this]()->_bool { return isAnimationRunning(); });
	pBlackBoard->Add_Condition("isKnockDown", [this]() ->_bool { return isKnockDown(); });
	pBlackBoard->Add_Condition("isAttackEnable", [this]() ->_bool { return isAttackEnable(); });
	pBlackBoard->Add_Condition("DodgeCooldown", [this]() ->_bool { return DodgeCooldown();});
	pBlackBoard->Add_Condition("Attack1", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK1, 3.f); });
	pBlackBoard->Add_Condition("Attack10", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK10, 4.f); });
	pBlackBoard->Add_Condition("Attack4", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK4, 5.f); });
	pBlackBoard->Add_Condition("Attack7", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK7, 6.f); });
	pBlackBoard->Add_Condition("Attack3", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK3, 8.f); });
	pBlackBoard->Add_Condition("Attack2", [this]() ->_bool { return Attack(ATK_PATTERN::ATTACK2, 10.f); });
	pBlackBoard->Add_Condition("Front", [this]() ->_bool { return Front(); });
	pBlackBoard->Add_Condition("Back", [this]() ->_bool { return Back(); });
	pBlackBoard->Add_Condition("Left", [this]() ->_bool { return Left(); });
	pBlackBoard->Add_Condition("Right", [this]() ->_bool { return Right(); });

	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = pBlackBoard;
	//Com_BehaviorTree
	if(FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_Component_BehaviorTree_Test"),
		TEXT("MonsterTest/Com_BehaviorTree"), reinterpret_cast<CComponent**>(&m_pBehaviorTreeCom), &BTDesc)))
		CRASH(m_pBehaviorTreeCom);
#pragma endregion
	
}

void CMonsterTest::Ready_PartObjects(MONSTERTEST_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Weapon002");
	TriggerDesc.vExtent = _float3(1.5f, 0.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(1.2f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = m_fAttackDmg;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::ELEC;
	//TriggerDesc.pCondition = &m_iState;
	TriggerDesc.test = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eLayer) {
		this->OnHitEnter(iLayer, pOther, Manifold, eLayer); 
		};

	//CContainerObject::Add_PartObject(TEXT("Part_ATKVolume"), m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), &TriggerDesc);
	m_pAtkVolumes[ATK_SOCKET::WEAPON_L] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), 
																		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::WEAPON_L])
		CRASH(m_pAtkVolumes[ATK_SOCKET::WEAPON_L]);
	m_pAtkVolumes[ATK_SOCKET::WEAPON_L]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Weapon003");
	TriggerDesc.vExtent = _float3(1.5f, 0.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(1.2f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::WEAPON_R] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::WEAPON_R])
		CRASH(m_pAtkVolumes[ATK_SOCKET::WEAPON_R]);
	m_pAtkVolumes[ATK_SOCKET::WEAPON_R]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("SkinBone021");
	TriggerDesc.vExtent = _float3(2.5f, 0.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::WHIP_R] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::WHIP_R])
		CRASH(m_pAtkVolumes[ATK_SOCKET::WHIP_R]);
	m_pAtkVolumes[ATK_SOCKET::WHIP_R]->TriggerActivate(false);
	
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("SkinBone007");
	TriggerDesc.vExtent = _float3(2.5f, 0.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::WHIP_L] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::WHIP_L])
		CRASH(m_pAtkVolumes[ATK_SOCKET::WHIP_L]);
	m_pAtkVolumes[ATK_SOCKET::WHIP_L]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Weapon003");
	TriggerDesc.vExtent = _float3(3.4f, 1.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(2.1f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::WEAPON_GR] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::WEAPON_GR])
		CRASH(m_pAtkVolumes[ATK_SOCKET::WEAPON_GR]);
	m_pAtkVolumes[ATK_SOCKET::WEAPON_GR]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bone_Weapon002");
	TriggerDesc.vExtent = _float3(3.f, 1.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(2.1f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	m_pAtkVolumes[ATK_SOCKET::WEAPON_GL] = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAtkVolumes[ATK_SOCKET::WEAPON_GL])
		CRASH(m_pAtkVolumes[ATK_SOCKET::WEAPON_GL]);
	m_pAtkVolumes[ATK_SOCKET::WEAPON_GL]->TriggerActivate(false);

	TriggerDesc.eLayer = COLLISIONLAYER::PARRY;
	vector<COLLISIONLAYER> Targets = { COLLISIONLAYER::ATTACK, COLLISIONLAYER::SKILL, COLLISIONLAYER::KNOCKBACK };
	TriggerDesc.eTargetLayers = Targets;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(2); // Root
	TriggerDesc.vExtent = _float3(2.f, 2.f, 6.f);
	TriggerDesc.vOffsetPos = _float3(0.f, 0.f, -2.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->ParryEnter(iLayer, pOther, Manifold);
		};
	m_pParryVolume = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(),
		TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pParryVolume)
		CRASH(m_pParryVolume);
	m_pParryVolume->TriggerActivate(false);
}

void CMonsterTest::Calculate_PosAndDir()
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

void CMonsterTest::Reset_Condition(_float fTimeDelta)
{
	if (m_fHP <= 0.f)
	{
		m_iState = ENUM_CLASS(TEST_STATE::DEAD);
		return;
	}
	if(m_isAnimationFinished)
	{
		_uint iRemainState{};
		if (m_iState & ENUM_CLASS(TEST_STATE::BLOCK))
			iRemainState |= ENUM_CLASS(TEST_STATE::BLOCK);
		m_iState = ENUM_CLASS(TEST_STATE::NONE);

		m_iState |= iRemainState;

	}
	if(m_isDetecting)
	{
		Calculate_PosAndDir();
#ifdef _DEBUG
		//cout << "x : " << m_vTargetPosition.x << " y : " << m_vTargetPosition.y << " z : " << m_vTargetPosition.z << endl;
		//cout << "distance: " << m_fDistance << endl;
#endif
	}
	for(_uint i = 0; i < 10; ++i)
	{
		if(m_fAttackAcc[i] > 0.f)
			m_fAttackAcc[i] -= fTimeDelta;
	}
	if(m_fDodgeCoolTime > 0.f)
		m_fDodgeCoolTime -= fTimeDelta;

#pragma region UI_BIND
	m_fParalysisRatio = m_fParalysisAcc * 0.2f;
	_matrix WorldSpine = XMLoadFloat4x4(m_pCameraMatrix) * m_pTransformCom->Get_WorldMatrix();
	XMStoreFloat3(&m_vUIPosition, WorldSpine.r[3]);
#pragma endregion

	if(m_isParalysis)
	{
		m_fParalysisAcc -= fTimeDelta;
		if(m_fParalysisAcc <= 0.f)
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

void CMonsterTest::After_Condition(_float fTimeDelta)
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
	if (true == m_beHit)
	{
		m_iState |= ENUM_CLASS(TEST_STATE::BEHIT);
		m_beHit = false;
	}
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
}

void CMonsterTest::OnDetect_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		if (m_isAggro)
			return;
		//UI Binding (몬스터 데이터 찾기용 키값, 현재 체력 변수 주소, 현재 무력화게이지 변수 주소, 텍스트 출력용 한글 wtring)
		m_pGameSystem->HUD_Bind_BossStatus(TEXT("거짓된 신왕"), "FalseSovereign", &m_fHP, &m_fStamina, &m_isParalysis, &m_fParalysisRatio);
		m_pGameSystem->HUD_Toggle_BossStatusUI(true);
		m_isRender = true;
		m_pGameSystem->Engage_Battle(true, BOSSBGM::SOERVERIGN);
	}
}

void CMonsterTest::BeHit(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (m_iState & ENUM_CLASS(TEST_STATE::DEAD))
		return;
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::ATTACK))
	{
		m_beHit = true;
		if(!m_isParalysis && m_fStamina >= 0.f)
			m_fStamina -= 1.f;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fHP -= pDesc->fAttack;
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 0.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		if (m_fHP <= 0.f)
		{
			m_pGameSystem->HUD_Toggle_BossStatusUI(false);
			m_pGameSystem->Engage_Battle(false, BOSSBGM::SOERVERIGN);
		}
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);

		const _wstring& strSoundTag = pDesc->strSoundTag;
		if (!strSoundTag.empty())
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::ENEMY_HIT), 0.4f);
#pragma endregion
#ifdef _DEBUG
		cout << "Be Hit! (False Sovereign)" << endl;
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::SKILL))
	{
		m_beHit = true;
		if (!m_isParalysis && m_fStamina >= 0.f)
			m_fStamina -= 1.f;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fHP -= pDesc->fAttack;
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 0.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		if (m_fHP <= 0.f)
		{
			m_pGameSystem->HUD_Toggle_BossStatusUI(false);
		}
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);

		const _wstring& strSoundTag = pDesc->strSoundTag;
		if (!strSoundTag.empty())
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::ENEMY_HIT), 0.4f);
#pragma endregion
#ifdef _DEBUG
		cout << "Be Hit! SKILL (False Sovereign)" << endl;
#endif // _DEBUG
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::KNOCKBACK))
	{
		m_beHit = true;
		if (!m_isParalysis && m_fStamina >= 0.f)
			m_fStamina -= 1.f;
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		m_fHP -= pDesc->fAttack;
		_float4 vPosition{};
		XMStoreFloat4(&vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		vPosition.y += 0.5f;
		m_pGameSystem->Render_Damage(vPosition, static_cast<_int>(pDesc->fAttack), pDesc->eType, 0.4f);
		if (m_fHP <= 0.f)
		{
			m_pGameSystem->HUD_Toggle_BossStatusUI(false);
		}
#pragma region HIT_EFFECT
		PREFAB_INFO EffectDesc{};

		m_pGameInstance->Spawn_PoolingObject(TEXT("A_Attack_Effect"), m_pTransformCom->Get_WorldMatrix()
			* XMMatrixTranslation(0.f, 1.35f, 0.f), &EffectDesc);

		const _wstring& strSoundTag = pDesc->strSoundTag;
		if (!strSoundTag.empty())
			m_pGameInstance->Play_Sound(strSoundTag, ENUM_CLASS(CHANNEL::ENEMY_HIT), 0.4f);
#pragma endregion
#ifdef _DEBUG
		cout << "Be Hit! KNOCKBACK (False Sovereign)" << endl;
#endif // _DEBUG
	}
}

void CMonsterTest::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer)
{
	CAMERA_SHAKE ShakeDesc{};
	
	if(eVolumeLayer == COLLISIONLAYER::ENEMY_ATTACK)
	{
		ShakeDesc.fAmplitude = 1.f;
		ShakeDesc.fDuration = 0.8f;
		ShakeDesc.fFovKick = 0.f;
		ShakeDesc.fFrequency = 2.f;
		ShakeDesc.vRotation = _float3(0.005f, 0.075f, 0.f);
		ShakeDesc.vTranslation;
#ifdef _DEBUG
		cout << "Common" << endl;
#endif // _DEBUG
	}
	else if (eVolumeLayer == COLLISIONLAYER::ENEMY_HARDATTACK)
	{
		ShakeDesc.fAmplitude = 2.f;
		ShakeDesc.fDuration = 0.15f;
		ShakeDesc.fFovKick = 0.f;
		ShakeDesc.fFrequency = 60.f;
		ShakeDesc.vRotation = _float3(0.13f, 0.0f, 0.f);
		ShakeDesc.vTranslation;
#ifdef _DEBUG
		cout << "Hard" << endl;
#endif // _DEBUG
	}
	else if (eVolumeLayer == COLLISIONLAYER::ENEMY_SKILL)
	{
		ShakeDesc.fAmplitude = 1.f;
		ShakeDesc.fDuration = 0.15f;
		ShakeDesc.fFovKick = 0.f;
		ShakeDesc.fFrequency = 45.f;
		ShakeDesc.vRotation = _float3(0.05f, 0.075f, 0.05f);
		ShakeDesc.vTranslation;

	}

	if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_1))
	{
		ShakeDesc.vRotation.y *= 1.2f;
	}
	else if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_3))
	{
		ShakeDesc.vRotation.x *= 0.75f;
		ShakeDesc.vRotation.y *= 0.75f;
	}
	else if (m_iState & ENUM_CLASS(TEST_STATE::ATTACK_4))
	{
		ShakeDesc.vRotation.x *= 1.2f;
	}

	m_pGameInstance->OnShake(ShakeDesc);
#ifdef _DEBUG
	cout << "On Hit! Shim Wang)" << endl;
#endif // _DEBUG
}

void CMonsterTest::ParryEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	m_iState |= ENUM_CLASS(TEST_STATE::BLOCK);
	memcpy(&m_vBeHit_Normal, &Manifold.mWorldSpaceNormal, sizeof(_float3));

#pragma region PARRY_UI
	m_pGameSystem->Enable_Parried();
#pragma endregion

#ifdef _DEBUG
	cout << "Parry! Shim Wang)" << endl;
#endif // _DEBUG
}

void CMonsterTest::TurnFix()
{
	m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
}

void CMonsterTest::TurnLerp(_bool isActive)
{
	m_isTurnLerp = isActive;
}

void CMonsterTest::DistanceInterpolate(_bool isActive)
{
	m_isDist_Interp_Enable = isActive;
}

_bool CMonsterTest::isKnockDown()
{
	//현재 그로기 상태 여부 판단. 행동트리에서 상태 제어 X
	if(m_isParalysis)
		return true;

	return m_iState & (ENUM_CLASS(TEST_STATE::PARALYSIS) | ENUM_CLASS(TEST_STATE::BLOCK));
}

_bool CMonsterTest::isAttackEnable()
{
	if(!m_isDetecting)
		return false;
	_bool Result{};

	if(m_fAttackAcc[ATK_PATTERN::ATTACK1] <= 0.f) Result = true;
	if(m_fAttackAcc[ATK_PATTERN::ATTACK2] <= 0.f) Result = true;
	if(m_fAttackAcc[ATK_PATTERN::ATTACK3] <= 0.f) Result = true;
	if(m_fAttackAcc[ATK_PATTERN::ATTACK4] <= 0.f) Result = true;
	if(m_fAttackAcc[ATK_PATTERN::ATTACK7] <= 0.f) Result = true;
	if(m_fAttackAcc[ATK_PATTERN::ATTACK10] <= 0.f) Result = true;

	return Result;
}

_bool CMonsterTest::DodgeCooldown()
{
	_bool Result = m_isDetecting && m_fDodgeCoolTime <= 0.f;
	if(Result)
		m_fDodgeCoolTime = 7.f;
	return Result;
}

_bool CMonsterTest::Attack(_uint iIndex, _float fInterval)
{

	if (iIndex == ATK_PATTERN::ATTACK7)
		return false;

	//else
	//	return false;
	_bool bResult = (m_fAttackAcc[iIndex] <= 0.f) && m_fDistanceNonY < fInterval;
	if(bResult)
	{
		m_fAttackAcc[iIndex] = m_fAttackCoolTime[iIndex];
		Attack_Arrange();
	}
	return bResult;
}

void CMonsterTest::Attack_Arrange()
{
	_float fRand = m_pGameInstance->Rand_Normal();
	if (fRand < 0.5f)
		m_iState |= ENUM_CLASS(TEST_STATE::MOVE_FORWARD);
}

_bool CMonsterTest::Back()
{
	return m_fFrontDot < 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CMonsterTest::Front()
{
	return m_fFrontDot > 0.f && fabs(m_fFrontDot) > 0.525f;
}

_bool CMonsterTest::Left()
{
	return m_fRightDot < 0.f && fabs(m_fRightDot) > 0.525f;
}

_bool CMonsterTest::Right()
{
	return m_fRightDot > 0.f && fabs(m_fRightDot) > 0.525f;
}

CMonsterTest* CMonsterTest::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonsterTest* pInstance = new CMonsterTest(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MonsterTest");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMonsterTest::Clone(void* pArg)
{
	CMonsterTest* pClone = new CMonsterTest(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MonsterTest (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMonsterTest::Free()
{
	__super::Free();

	for(_uint i = 0; i < ATK_SOCKET::END; ++i)
		Safe_Release(m_pAtkVolumes[i]);

	Safe_Release(m_pGameSystem);
	Safe_Release(m_pParryVolume);
	Safe_Release(m_pBehaviorTreeCom);
	Safe_Release(m_pAnimMachineCom);
}
