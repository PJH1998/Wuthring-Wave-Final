#include "ClientPch.h"
#include "Player.h"
#include "Galbrena.h"
#include "GalbrenaFactory.h"
#include "SpringCamera.h"
#include "Wing.h"
#include "GalbrenaShotGun.h"
#include "GalbrenaDarkWing.h"
#include "Collider.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CGalbrena::CGalbrena(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CGalbrena::CGalbrena(const CGalbrena& Prototype)
    : CCharacter(Prototype)
{
}
 
HRESULT CGalbrena::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CGalbrena::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CCharacter::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eCurLevel;
    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
    Ready_PartObjects(pDesc); // Parts 추가.
	Ready_AttackVolumes();
    Register_AllNotifies(pDesc->strFolderPath);

	CGalbrenaFactory::Register_States(m_pStateMachineCom, this);    // 기본 StateMachineCom
	CGalbrenaFactory::Register_States(m_pFpsStateMachineCom, this); // FPS StateMachineCom
	
	// 비활성화. 
	//PartActivate(PART_FIRSTGUN, false);
	PartActivate(PART_FIRSTGUN, false);
	PartActivate(PART_SECONDGUN, false);
	PartActivate(PART_DARKWING, false);
	PartActivate(PART_LION, false);
	PartActivate(PART_WING, false);
	

	m_IsQTE = false; // QTE
    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1000.f, 0.f, 0.f);
	XMStoreFloat4(&m_vQTEPos, vPos);
	m_pQTEColliderCom->Set_Position(vPos);



	m_pMainAttackVolume->TriggerActivate(false);
    return S_OK;
}

void CGalbrena::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
	m_fMaxDissolveTime = 0.35f;
	// 0. Delayed Action 수행.
	Process_DelayedActions(fTimeDelta);

	// 1. Parts 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Priority_Update(fTimeDelta);
	}

    // 2. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();

	// 3. 몬스터와 타겟간의 거리 계산하기.
	Update_TargetDistance();

	// 4. AttackVolume 몬스터에 바인딩.
	Bind_TargetToVolumes();

	// 5. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Priority_Update(fTimeDelta);
  
	// Dissolve 체크.
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));

	// 6. Dissovle 체크
	if (IsDissolve)
	{
		if (m_fDissolveTimer <= m_fMaxDissolveTime)
			m_fDissolveTimer += fTimeDelta;
		else
		{
			m_isActivate = false;
			Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
		}
	}
}

void CGalbrena::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

	

	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));

	// 3. 상태 머신 갱신
	if (!IsDissolve)
	{
		m_pStateMachineCom->Update(fTimeDelta * m_fStateTimeRate); // 여기서 Weapon이나 Parts의 갱신을 해야함.. => 여기서 Play_Animation 실행됨.

		// 4. Physcics, Camera 업데이트
		Update_Physics(fTimeDelta);
		Update_Camera(fTimeDelta);
	}

	// 2. 파츠 갱신.?
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Update(fTimeDelta);
	}
	

	// 5. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);

}
void CGalbrena::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

   

	// 2. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Late_Update(fTimeDelta);

	// 3. QTE인 경우 Collider 갱신하지 않습니다.?

	if (Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
	{
		m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION)); // 이동이 아닌 위치 재설정/
	}
	else
	{
		// 2. QTE인 경우 Collider 갱신하지 않음.
		if (!m_IsQTE)
			m_pColliderCom->Sync_Position(m_pTransformCom);
		else
			m_pQTEColliderCom->Sync_Position(m_pTransformCom);
	}

	if (m_IsQTEend)
	{
		Notify_HarmonyEnd();
		m_pQTEColliderCom->Set_Position(XMLoadFloat4(&m_vQTEPos));
		m_IsQTEend = false;
	}

	if (m_IsVisible)
	{
		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
			return;

		if (m_IsOutLineVisible)
		{
			if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE, this)))
				return;
		}
		

		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
			return;
	}


	//  파츠 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Late_Update(fTimeDelta);
	}
    
}

void CGalbrena::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();

	// 1. Dissolve 체크.
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		_float fDissolveRate = (m_fDissolveTimer / m_fMaxDissolveTime);
		if (FAILED(m_pShaderCom->Bind_Value("g_fDissolveRate", &fDissolveRate, sizeof(_float))))
			CRASH("Failed Bind Dissolve Rate");

		if (FAILED(m_pShaderCom->Bind_Value("g_vDissolveColor", &m_vDissolveColor, sizeof(_float4))))
			CRASH("Ready EnergyColor");

		if(FAILED(m_pShaderCom->Bind_Value("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
			CRASH("Ready EmissiveIntensity")
	}


    for (_uint i = 0; i < iNumMeshes; i++)
    {
		if (IsBack(i))
			Render_Back(i);
		else if (IsEye(i))
			Render_Eye(i);
		else if (IsSkin(i))
			Render_Skin(i);
		else
			Render_Default(i);
		

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pModelCom->Bind_MorphedResult(m_pShaderCom, i, "g_MorphedVertices")))
			CRASH("Bind Morph Result Failed");

        if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");

		m_pShaderCom->UndBind_All_VS_SRV();
    }

#ifdef _DEBUG
	if (!m_IsQTE)
		m_pColliderCom->Render();
	else
		m_pQTEColliderCom->Render();

	m_pMainAttackVolume->Render();
	Print_LookRay();
#endif // _DEBUG

}

void CGalbrena::Render_OutLine()
{
	Bind_Resources();

	_uint iNumMeshes = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (i == 5)	//Cloths
			continue;

		_bool HasNormal = { false };

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::OUNTLINE))))
			CRASH("Ready Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");
	}
}

void CGalbrena::Render_Shadow()
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


// 캐릭터 전환시 Idle로 상태 전환..
void CGalbrena::TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType)
{
	// 현재 애니메이션 제거.
	m_pStateMachineCom->Exit_State();

	switch (eTransitionType)
	{
		case CHARACTER_TRANSITIONTYPE::IDLE:
		{
			// 애니메이션 변경할 값.
			GetStateContextForWrite().m_eIdleType = EGalbrenaIdleType::STAND2;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::IDLE));
			break;
		}
		case CHARACTER_TRANSITIONTYPE::QTE:
		{
			m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.5f, 0.5f);
			// 애니메이션 변경할 값.
			GetStateContextForWrite().m_eQTEType = EGalbrenaQTEType::SKILL_QTE;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::QTE));
			break;
		}

	}

	// 상태 변수 초기화
	m_IsQTE = false;
	m_StateContext.Clear();
}



	// AnimName이 같은걸로 매핑되어있음.
void CGalbrena::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    switch (iPartType)
    {
	case PART_FIRSTGUN:
		m_pGalbrenaFirstShotGun->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_SECONDGUN:
		m_pGalbrenaSecondShotGun->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_LION:
		break;
	case PART_DARKWING:
		m_pGalbrenaDarkWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_WING:
		m_pWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	
    default:
        break;
    }
}

void CGalbrena::PartActivate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
	case PART_FIRSTGUN:
		m_pGalbrenaFirstShotGun->Activate(IsActive);
		break;
	case PART_SECONDGUN:
		m_pGalbrenaSecondShotGun->Activate(IsActive);
		break;
	case PART_LION:
		break;
	case PART_DARKWING:
		m_pGalbrenaDarkWing->Activate(IsActive);
		break;
	case PART_WING:
		m_pWing->Activate(IsActive);
		break;
    default:
        break;
    }
}

void CGalbrena::Part_VolumeChange(_uint iPartType, _uint iVolumeIdx)
{
	switch (iPartType)
	{
	case PART_FIRSTGUN:
		m_pGalbrenaFirstShotGun->Change_Volume(iVolumeIdx);
		break;
	case PART_SECONDGUN:
		m_pGalbrenaSecondShotGun->Change_Volume(iVolumeIdx);
		break;
	case PART_DARKWING:
		m_pGalbrenaDarkWing->Change_Volume(iVolumeIdx);
		break;
	case PART_LION:
		break;
	}
}

void CGalbrena::Part_VolumeActivate(_uint iPartType, _bool IsActive)
{
	switch (iPartType)
	{
	case PART_FIRSTGUN:
		m_pGalbrenaFirstShotGun->Volume_Activate(IsActive);
		break;
	case PART_SECONDGUN:
		m_pGalbrenaSecondShotGun->Volume_Activate(IsActive);
		break;
	case PART_DARKWING:
		m_pGalbrenaDarkWing->Volume_Activate(IsActive);
		break;
	case PART_LION:
		break;
	}
}

void CGalbrena::Clear_PartAnimation(_uint iPartType, const _string& strAnimName)
{
    switch (iPartType)
    {
	case PART_FIRSTGUN:
		m_pGalbrenaFirstShotGun->Clear_Animation(strAnimName);
		break;
	case PART_SECONDGUN:
		m_pGalbrenaSecondShotGun->Clear_Animation(strAnimName);
		break;
	case PART_LION:
		break;
	case PART_DARKWING:
		m_pGalbrenaDarkWing->Clear_Animation(strAnimName);
		break;
	case PART_WING:
		m_pWing->Clear_Animation(strAnimName);
		break;
    default:
        break;
    }

}

void CGalbrena::Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName)
{
    ASSERT_CRASH(m_pModelCom);
    
    const _float4x4* pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());
    if (nullptr == pSocketMatrix)
        pSocketMatrix = &m_MatrixIdentity;
    
    switch (iPartType)
    {
	case PART_FIRSTGUN:
		break;
	case PART_SECONDGUN:
		break;
	case PART_DARKWING:
		break;
	case PART_LION:
		break;
	case PART_WING:
		m_pWing->Set_SocketMatrix(pSocketMatrix);
		break;
    }
}

// Hit 판정.
void CGalbrena::Hit_Judge(void* pArg)
{
	if (nullptr == pArg || m_IsHit || m_IsQTE)
		return;

	_uint iFlag = {};
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::DODGE);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::HIT);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE);

	// 컨디션 체크
	if (Check_AnyCondition(iFlag))
		return;

	StateKey eKey = m_pStateMachineCom->Get_CurrentStateKey();
	_uint iCategory = eKey.iCategory;
	_uint iSubState = eKey.iSubState;

	EStateCategory eCategory = static_cast<EStateCategory>(iCategory);

	if (EStateCategory::HIT == eCategory)
		return;

	// 2. 즉시 중복 방지 플래그 세팅
	//m_PendingConditions[HIT] = true;

	// 3. 데이터 저장.
	CCharacter::HIT_DESC* pDesc = static_cast<HIT_DESC*>(pArg);
	m_PendingHitDesc = *pDesc;

	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)); // 회피 가능
	m_fDodgeableHitTimer = m_fDodgeableDuration;


	// 4. 맞았을떄 시간 느리게 하기? => 이때 Attack이라면? 무시. => 다른 스킬 조건들은 Invincible 상태라 예외처리할 필요성 X
	_bool IsAttack = eKey.iCategory == ENUM_CLASS(EStateCategory::GROUND) 
		&& eKey.iSubState == ENUM_CLASS(EGalbrenaGroundState::ATTACK);

	if (!IsAttack)
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.1f, 0.05f); // Dodge 시간 동안 느리게하기? => 0.05로 해야 0.5f?
	else
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.3f, 0.1f);

	
	//m_DelayedActions.push(DELAYED_ACTION(DELAYED_ACTION::TYPE::HIT, pDesc)); => Pro


}

void CGalbrena::Grab_Judge(void* pArg)
{
	if (nullptr == pArg || m_IsHit || m_PendingConditions[QTE])
		return;

	// 1. Grab이 안통하는 상태일때. => Dodge, Grabe, Invincible
	_uint iFlag = {};
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::DODGE);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::GRABED);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE);

	if (Check_AnyCondition(iFlag))
		return;

	// 2. Capture 데이터 캐스팅.
	m_PendingCaptureDesc = *static_cast<CAPTURE_DESC*>(pArg);

	// 데미지 처리.
	m_pAbillityCom->Add_Hp(m_PendingCaptureDesc.fAttack * -1.f);

	// 3. 콜백 함수 내에서는 Jolt에 대한 변경작업을 진행하면 안된다. => Priority Update로 진행 넘기기.
	m_DelayedActions.push({ DELAYED_ACTION::TYPE::GRAB, &m_PendingCaptureDesc });
}

void CGalbrena::Sync_Position()
{
    m_pColliderCom->Sync_Position(m_pTransformCom);
}

void CGalbrena::Bind_QTE(_bool IsQTE)
{
	m_IsQTE = IsQTE;

	if (m_IsQTE)
	{
		// Activate
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

		// 내 앞에서 생성. (안 곂치게)
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
		_vector vUp = XMVectorSet(0.f, 2.f, 0.f, 0.f);
		vPos += vLook * 1.f;
		m_pQTEColliderCom->Set_Position(vPos);
		m_pQTEColliderCom->IsActivate(true);

		SetActivate(true);
		GetStateContextForWrite().m_eQTEType = EGalbrenaQTEType::SKILL_QTE;
		Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EGalbrenaGroundState::QTE));
	}
}

void CGalbrena::Reset_QTECamera()
{
	m_fCameraOffset = m_fCameraOriginOffset;
}

void CGalbrena::Bind_QTECamera()
{
	m_fCameraOriginOffset = m_fCameraOffset;
	m_fCameraOffset = 5.f; // 늘립니다.
}



#pragma region NOTIFY
void CGalbrena::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	_wstring var1, var2, var3;
	wstringstream wss(wStrColliderTag);
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)

	if (var1 == TEXT("Main"))
		m_pMainAttackVolume->TriggerActivate(IsActive);

	if (var1 == TEXT("Galbrena"))
	{
		// 1. Attack Volume Index 설정.
		if (var2 == TEXT("ARROUND"))
			m_iVolumeIdx = VOLUME::VOLUME_ARROUND; // 주변 공격.
		else if (var2 == TEXT("ARROUND_SLASH"))
			m_iVolumeIdx = VOLUME::VOLUME_ARROUND_SLASH;
		else if (var2 == TEXT("AIR_LOOP"))
			m_iVolumeIdx = VOLUME::VOLUME_AIR_LOOP;
		else if (var2 == TEXT("KNOCKBACK"))
			m_iVolumeIdx = VOLUME::VOLUME_KNOCKBACK;
		else if (var2 == TEXT("TARGET"))
			m_iVolumeIdx = VOLUME::VOLUME_TARGET;
		else if (var2 == TEXT("TARGET_BURST"))
			m_iVolumeIdx = VOLUME::VOLUME_TARGET_BURST;
			
		else if (var2 == TEXT("SKILL"))
			m_iVolumeIdx = VOLUME::VOLUME_SKILL;
		else if (var2 == TEXT("DEFAULT_E"))
			m_iVolumeIdx = VOLUME::VOLUME_DEFAULT_E;

		// 2. Main Attack Volume 교체.
		m_pMainAttackVolume->TriggerActivate(false);
		m_pMainAttackVolume = m_AttackVolumes[m_iVolumeIdx];

		// 3. Layer 설정.
		if (var3 == TEXT("ATTACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::SKILL);

		// 4. 활성화.
		m_pMainAttackVolume->TriggerActivate(IsActive);
	}
	else if (var1 == TEXT("SecondAttack"))
	{

	}
	else if (var1 == TEXT("Lion"))
	{
		/*if (var2 == TEXT("ATK"))
			iVolumeIdx = CGalbrenaDarkScythe::VOLUME::VOLUME_ATTACK;

		if (var3 == TEXT("ATTACK"))
			m_pGalbrenaDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pGalbrenaDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pGalbrenaDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pGalbrenaDarkScythe->Volume_Activate(IsActive);*/
	}
	
}

void CGalbrena::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &effecInfo);
}

void CGalbrena::Object_Func(const _wstring& wStrObjectTag)
{
	// 3개의 변수 준비
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);

	// std::getline을 사용하여 L'|' 구분자를 만날 때까지 읽어 변수에 저장합니다.
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)


	// 1. Type Tag
	if (var1 == TEXT("FirstShotGun"))
	{
		// 2. Action Tag
		if (var2 == TEXT("Dissolve"))
		{
			// 3. Dissolve On / Off
			if (var3 == TEXT("On"))
			{
				m_pGalbrenaFirstShotGun->Activate(false);
				return;
			}
		}
	}
	else if (var1 == TEXT("SecondShotGun"))
	{
		// 2. Action Tag
		if (var2 == TEXT("Dissolve"))
		{
			// 3. Dissolve On / Off
			if (var3 == TEXT("On"))
			{
				m_pGalbrenaSecondShotGun->Activate(false);
				return;
			}
		}
	}
	else if (var1 == TEXT("DarkWing"))
	{
		// 2. Action Tag
		if (var2 == TEXT("Dissolve"))
		{
			// 3. Dissolve On / Off
			if (var3 == TEXT("On"))
			{
				m_pGalbrenaDarkWing->Activate(false);
				return;
			}
		}
	}
	else if (var1 == TEXT("StateDelay")) // 애니메이션 State의 속도를 Delay 시킵니다.
	{
		m_fStateTimeRate = stof(var2);
		m_fStateDelayTimer = stof(var3);
		Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::STATE_DELAY));
	}

	// GalbrenaWing|Bone

}
void CGalbrena::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 1. 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::GALBRENA));

	if (nullptr == pAbility)
		return;

	// 2. Burst 상태인지 확인.
	_bool IsBurst = Check_AnyConidtion_FromAbility(ENUM_CLASS(UI_ROVER_CONDITION::BURST_ACTIVE));
	
	switch (m_iVolumeIdx)
	{
	case VOLUME::VOLUME_DEFAULT_E:
		// 기본 E로 타격 시 State가 변하니까 Condition을 바꿔주어야함.
		Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::SKILLHIT));
		pAbility->Add_HarmonyGauge(10.f); // 협주 게이지 채우기.
		pAbility->Add_Cost(COST_TYPE::COST5, 10.f); // 기본 궁극기 게이지

		if(!IsBurst)
			pAbility->Add_Cost(COST_TYPE::COST1, 10.f); // 공명 게이지
		break;
	case VOLUME::VOLUME_KNOCKBACK: // 기본 공격시 협주 게이지와 공명 게이지 채우기
		pAbility->Add_HarmonyGauge(7.f); // 협주 게이지 채우기.
		if (!IsBurst)
			pAbility->Add_Cost(COST_TYPE::COST1, 5.f); // 공명 게이지
		pAbility->Add_Cost(COST_TYPE::COST5, 7.f); // 기본 궁극기 게이지
		break;
	case VOLUME::VOLUME_TARGET_BURST: // 궁극기 사용 시 ?
		pAbility->Add_HarmonyGauge(20.f); // 협주 게이지 채우기.
		if (!IsBurst)
			pAbility->Add_Cost(COST_TYPE::COST1, 30.f); // 공명 게이지(강공격 게이지)
		break;
	default:
		pAbility->Add_HarmonyGauge(5.f); // 협주 게이지 채우기.
		pAbility->Add_Cost(COST_TYPE::COST5, 4.f); // 기본 궁극기 게이지
		if (!IsBurst)
			pAbility->Add_Cost(COST_TYPE::COST1, 3.f); // 공명 게이지(강공격 게이지)
		break;
	}
}

#pragma endregion

#pragma region 4. EVENT
void CGalbrena::Process_DelayedActions(_float fTimeDelta)
{
	_uint iDodgeableFlag = ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE);

	// 0. 회피 가능창 활성화 되어 있다면?
	if (Check_AnyCondition(iDodgeableFlag))
	{
		m_fDodgeableHitTimer -= fTimeDelta;
		if (m_fDodgeableHitTimer <= 0.f)
		{
			Remove_Condition(iDodgeableFlag); // 회피 가능 상태 제거

			// 저장해뒀던 피격 정보를 사용해 실제 HIT 처리

			// Hit가 되고 있다는 사실은 알고 있어야됨. 그래야 Hit
			m_PendingConditions[HIT] = true;
			m_DelayedActions.push(DELAYED_ACTION(DELAYED_ACTION::TYPE::HIT, &m_PendingHitDesc));
		}
	}

	_uint iDelayFlag = ENUM_CLASS(CHARACTER_CONDITION::STATE_DELAY);
	if (Check_AnyCondition(iDelayFlag))
	{
		m_fStateDelayTimer -= fTimeDelta;
		if (m_fStateDelayTimer <= 0.f)
		{
			Remove_Condition(iDelayFlag);
			m_fStateTimeRate = m_fOriginTimeRate; // 원래 TimeRate로 변경합니다.
			m_fStateDelayTimer = 0.f;
		}

	}


	while (!m_DelayedActions.empty())
	{
		DELAYED_ACTION eAction = m_DelayedActions.front();

		void* pData = eAction.pData;
		switch (eAction.type)
		{
		case DELAYED_ACTION::TYPE::HIT:
		{
			//m_IsHit = true;
			Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // Condition 추가.
			m_pAbillityCom->Add_Hp(-m_PendingHitDesc.fAttack);
			break;
		}
		case DELAYED_ACTION::TYPE::GRAB:
		{
			ActiveCaptureState();
			GetStateContextForWrite().m_eCaptureType = EGalbrenaCaptureType::BEHIT_FLY_START;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::CAPTURED), ENUM_CLASS(EGalbrenaCaptureState::CAPTURE));
			break;
		}

		default:
			break;
		}

		m_DelayedActions.pop();
	}
}
void CGalbrena::Bind_ChangeEffect()
{
	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();

	m_pGameInstance->Spawn_PoolingObject(TEXT("Common_SwapEffect"), matWorld, &effecInfo);
}
void CGalbrena::Bind_DissolveTimer()
{
	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	m_fDissolveTimer = 0.f;	
}
void CGalbrena::Bind_DefaultShaderPath()
{
	// 기본 Shader Path
	for (_uint i = 0; i < MESHTYPE::MESH_END; ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::GALBRENA);

	// 등짝에 문신
	m_ShaderPaths[MESHTYPE::MESH_EYE_OL] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::GALBRENABACK);
}
void CGalbrena::Bind_DissolveShaderPath()
{
	for (_uint i = 0; i < MESHTYPE::MESH_END; ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::DISSOLVE_CHARACTER);
}

void CGalbrena::Activate(_bool IsActivate)
{
	if (false == IsActivate)
	{
		Bind_DissolveTimer();
		Bind_DissolveShaderPath();
		m_IsOutLineVisible = false;
		XMStoreFloat4x4(&m_DissolveWorldMatrix, m_pTransformCom->Get_WorldMatrix());
	}

	if (true == IsActivate)
	{
		m_isActivate = true;
		m_IsOutLineVisible = true;
		Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
		Bind_DefaultShaderPath();
	}
}
#pragma endregion


// 매 프레임 볼륨 타겟 매트릭스 전달.
void CGalbrena::Bind_TargetToVolumes()
{
	// 전달할 TargetMatrix
	const _float4x4* pTargetMatrix = nullptr;

	// LockOn Target 우선.
	if (nullptr != m_pLockOnTargetTransform)
		pTargetMatrix = m_pLockOnTargetTransform->Get_WorldMatrixPtr();
	else if (nullptr != m_pTargetTransform)
		pTargetMatrix = m_pTargetTransform->Get_WorldMatrixPtr();

	// 매프레임 계속 전달.
	m_AttackVolumes[VOLUME_TARGET]->Bind_SocketMatrix(pTargetMatrix);
	m_AttackVolumes[VOLUME_TARGET_BURST]->Bind_SocketMatrix(pTargetMatrix);
}

void CGalbrena::Update_TargetDistance()
{
	const _float4x4* pTargetMatrix = nullptr;

	_vector vTargetPos = {};

	// LockOn Target 우선
	if (nullptr != m_pLockOnTargetTransform)
		vTargetPos = m_pLockOnTargetTransform->Get_State(STATE::POSITION);
	// 없으면 Target Transform.
	else if (nullptr != m_pTargetTransform) 
		vTargetPos = m_pTargetTransform->Get_State(STATE::POSITION);

	// 거리 계산. Y제외.
	_vector vDistance = m_pTransformCom->Get_State(STATE::POSITION) - vTargetPos;
	vDistance = XMVectorSetY(vDistance, 0.f);
	m_fTargetDistance = XMVectorGetX(XMVector3Length(vDistance));
}

void CGalbrena::Update_Physics(_float fTimeDelta)
{
	if (Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
	{
		if (nullptr != m_PendingCaptureDesc.pSocketMatrix &&
			nullptr != m_PendingCaptureDesc.pTransform)
		{
			_matrix matFinalWorld = XMLoadFloat4x4(m_PendingCaptureDesc.pSocketMatrix); // 1. 본행렬

			_vector vScale{}, vRotQuat{}, vTrans{};
			_vector vPlayerScale = XMVectorSet(1.f, 1.f, 1.f, 0.f);
			XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, matFinalWorld);
			m_pTransformCom->Set_State(STATE::POSITION, vTrans);
		}
	}
	else
	{
		// 3. 현재 위치 - 1Frame 이전 위치 값 계산'
		_vector vVelocity = m_pTransformCom->Get_Velocity();
		if (!m_IsQTE)
			m_pColliderCom->Update(vVelocity / fTimeDelta);
		else
			m_pQTEColliderCom->Update(vVelocity / fTimeDelta);
		// 6. Land Check
		m_IsLand = Is_LandCollider();
	}
}

void CGalbrena::Update_Camera(_float fTimeDelta)
{
	if (Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
	{
		_vector vCameraLook = m_pSpringCamera->Get_LookVector_NoPitch(); // Camera Look을 
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		vPos += vCameraLook * -3.f;
		m_pSpringCamera->Update_Target(vPos, 1.2f); // 카메라는 고정.
	}
	else if (!m_IsQTE)
	{
		m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 1.2f);
	}
}

void CGalbrena::Render_Default(_uint iMeshIndex)
{
	if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TEXTURETYPE::DIFFUSE, 0)))
		return;

	_bool HasNormal = { false };
	if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", iMeshIndex, TEXTURETYPE::NORMAL, 0)))
		HasNormal = true;

	if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
		CRASH("Ready g_HasNormal Failed");

}

void CGalbrena::Render_Skin(_uint iMeshIndex)
{

	if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TEXTURETYPE::DIFFUSE, 0)))
		return;

	_bool HasNormal = { false };
	if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", iMeshIndex, TEXTURETYPE::NORMAL, 0)))
		HasNormal = true;

	_bool HasSkinMask = { false };
	if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", iMeshIndex, TEXTURETYPE::MASK))) // MaskTexture 배열을 바인딩.
		HasSkinMask = true;

	if (FAILED(m_pShaderCom->Bind_Value("g_HasSkinMask", &HasSkinMask, sizeof(_bool))))
		CRASH("Ready g_HasSkinMask Failed");

	if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
		CRASH("Ready g_HasNormal Failed");
}

void CGalbrena::Render_Back(_uint iMeshIndex)
{
	_bool IsCutScene = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));
	m_iGalbrenaMaskIndex = IsCutScene ? 2 : 1;

	_float4 vEmissiveColor = { 0.7f, 0.2f, 0.3f, 1.f};
	_float fEmissiveIntensity = { 0.5f };

	// 1. MaskTexture 배열 바인딩.
	m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", iMeshIndex, TEXTURETYPE::MASK); // MaskTexture 배열을 바인딩.

	// 2. MaskIndex 바인딩.
	m_pShaderCom->Bind_Value("g_iGalbrenaMaskIndex", &m_iGalbrenaMaskIndex, sizeof(_uint));

	// 3. Emissive 바인딩.
	m_pShaderCom->Bind_Value("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4));
	m_pShaderCom->Bind_Value("g_fEmissiveIntensity", &fEmissiveIntensity, sizeof(_float));

}

void CGalbrena::Render_Eye(_uint iMeshIndex)
{
	_bool IsCutScene = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE));

	if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TEXTURETYPE::DIFFUSE, 0)))
		return;

	_bool HasNormal = { false };
	if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", iMeshIndex, TEXTURETYPE::NORMAL, 0)))
		HasNormal = true;

	if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
		CRASH("Ready g_HasNormal Failed");

	if (IsCutScene)
	{
		m_ShaderPaths[iMeshIndex] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::GALBRENAEYE);
		_float4 vEmissiveColor = { 1.f, 0.1f, 1.0f, 1.f };
		_float fEmissiveIntensity = { 5.f };
		_float fGalbrenaEyeAlpha = 0.6f;
		m_pShaderCom->Bind_Value("g_vEmissiveColor", &vEmissiveColor, sizeof(_float4));
		m_pShaderCom->Bind_Value("g_fEmissiveIntenmmsity", &fEmissiveIntensity, sizeof(_float));
		m_pShaderCom->Bind_Value("g_fGalbrenaEyeAlpha", &fGalbrenaEyeAlpha, sizeof(_float));
	}
	else
		m_ShaderPaths[iMeshIndex] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::GALBRENA);
}


_bool CGalbrena::IsSkin(_uint iMeshIndex)
{
	if (iMeshIndex == MESH_FACE ||
		iMeshIndex == MESH_UP ||
		iMeshIndex == MESH_DOWN)
		return true;
		

	return false;
}

_bool CGalbrena::IsBack(_uint iMeshIndex)
{
	if (iMeshIndex == MESH_EYE_OL)
		return true;

	return false;
}

_bool CGalbrena::IsEye(_uint iMeshIndex)
{
	if (iMeshIndex == MESH_EYE)
		return true;

	return false;
}

void CGalbrena::Bind_Resources()
{
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		const _float4x4* pWorldMatrix = &m_DissolveWorldMatrix;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", pWorldMatrix))) // Dissolve는 해당 위치에 멈춰서 재생되어야함.
			CRASH("Failed Bind Matrix");
	}
	else
	{
		if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
			CRASH("Failed Bind Matrix");
	}
    

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");

}

void CGalbrena::Ready_Components(const CHARACTER_DESC* pDesc)
{
    // 1. Components
    /*if(FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");*/

    if(FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
        , pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
        CRASH("Compute Shader");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->flyComputeShaderData.first)
		, pDesc->flyComputeShaderData.second, TEXT("Com_ComputeShaderFly"), reinterpret_cast<CComponent**>(&m_pFlyComputeShaderCom), nullptr)))
		CRASH("Com_ComputeShaderFly");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->facialComputeShaderData.first)
		, pDesc->facialComputeShaderData.second, TEXT("Com_ComputeShaderFacial"), reinterpret_cast<CComponent**>(&m_pFacialComputeShaderCom), nullptr)))
		CRASH("Com_ComputeShaderFly");

#ifdef _DEBUG
	cout << "Galbrena Model Clone : " << endl;
#endif // _DEBUG

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
        , pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->stateMachineData.first)
        , pDesc->stateMachineData.second, TEXT("Com_StateMachine"), reinterpret_cast<CComponent**>(&m_pStateMachineCom), nullptr)))
        CRASH("StateMachine");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->stateMachineData.first)
		, pDesc->stateMachineData.second, TEXT("Com_FpsStateMachine"), reinterpret_cast<CComponent**>(&m_pFpsStateMachineCom), nullptr)))
		CRASH("StateMachine");

	
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.vPos = pDesc->vPosition;
	ColliderDesc.vOffset = { 0.f, 0.67f, 0.f };
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::QTE);
	ColliderDesc.fHeight = 0.4f;
	ColliderDesc.fRadius = 0.5f;
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Collider"), TEXT("Com_QTECollider"), reinterpret_cast<CComponent**>(&m_pQTEColliderCom), &ColliderDesc)))
		CRASH("Collider");

}

void CGalbrena::Ready_Variables(const CHARACTER_DESC* pDesc)
{

	m_fDodgeableDuration = 0.1f; // Dodge 가능 시간.
	m_fCameraOriginOffset = 1.2f;
	m_fCameraOffset = 1.2f;

    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::GALBRENA);

	m_ShaderPaths[MESH_EYE_OL] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::GALBRENABACK);
	
	// Shader Value 추가.
	m_vEmissiveColor = {};
	m_vMaskEmssiveColor = {};

	m_fDissolveTimer = 0.f;
	m_fMaxDissolveTime = 0.35f;
	m_vDissolveColor = { 0.407f, 0.619f, 1.f, 1.f };
	m_fEmissiveIntensity = 3.f;
}

void CGalbrena::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}


void CGalbrena::Ready_PartObjects(const CHARACTER_DESC* pDesc)
{

    _float3 vScale = {};
    _float3 vRotation = {};
    _float3 vPosition = {};

    for (_uint i = 0; i < PARTTYPE::TYPE_END; ++i)
    {
        _wstring strPartName = pDesc->PartPrototypes[i].first;
        _wstring strPrototypeName = pDesc->PartPrototypes[i].second;

        CProp::PROP_DESC Desc{};
        switch (i)
        {
        case PARTTYPE::PART_FIRSTGUN:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetGalbrenaFirstShotGunCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);

			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("PART_FIRSTGUN");

			m_pGalbrenaFirstShotGun = dynamic_cast<CGalbrenaShotGun*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pGalbrenaFirstShotGun);
			Safe_AddRef(m_pGalbrenaFirstShotGun);
			break;
		case PARTTYPE::PART_SECONDGUN:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetGalbrenaSecondShotGunCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);

			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("PART_SECONDSHOTGUN");

			m_pGalbrenaSecondShotGun = dynamic_cast<CGalbrenaShotGun*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pGalbrenaSecondShotGun);
			Safe_AddRef(m_pGalbrenaSecondShotGun);
			break;

		case PARTTYPE::PART_LION:
			break;
		case PARTTYPE::PART_DARKWING:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetGalbrenaDarkWingCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);

			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("PART_DARKWING");

			m_pGalbrenaDarkWing = dynamic_cast<CGalbrenaDarkWing*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pGalbrenaDarkWing);
			Safe_AddRef(m_pGalbrenaDarkWing);
			break;

		case PARTTYPE::PART_WING:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetWingCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);

			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("Wing");

			m_pWing = dynamic_cast<CWing*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pWing);
			Safe_AddRef(m_pWing);
			break;
		}
    }
}

void CGalbrena::Ready_AttackVolumes()
{
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::BONE; // 뼈
	TriggerDesc.eDir = ATTACKVOULME_DIR::DEFAULT;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bip001");
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::KNOCKBACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 1.f); // (x, z, y)임 x, z 크게 y작게 
	TriggerDesc.vOffsetPos = _float3(0.0f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 250.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::FUSI;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
	};

	m_AttackVolumes[VOLUME_ARROUND] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_ARROUND]);
	m_AttackVolumes[VOLUME_ARROUND]->TriggerActivate(false);

	TriggerDesc.vExtent = _float3(3.f, 3.f, 1.f); // (x, z, y)임 x, z 크게 y작게 
	m_AttackVolumes[VOLUME_ARROUND_SLASH] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_ARROUND_SLASH]);
	m_AttackVolumes[VOLUME_ARROUND_SLASH]->TriggerActivate(false);

	_wstring strEffectTag = TEXT("GALBRENA_SLASH_EFFECT"); // 임시.
	m_AttackVolumes[VOLUME_ARROUND_SLASH]->Cange_EffectTag(strEffectTag);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Root");
	TriggerDesc.vExtent = _float3(4.f, 4.f, 6.f); // (x, z, y)
	m_AttackVolumes[VOLUME_AIR_LOOP] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_AIR_LOOP]);
	m_AttackVolumes[VOLUME_AIR_LOOP]->TriggerActivate(false);


	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Bip001_R_Knee_B");
	TriggerDesc.vExtent = _float3(2.f, 2.f, 1.f); // x, z 크게 y작게
	TriggerDesc.fAttackDmg = 600.f;
	m_AttackVolumes[VOLUME_DEFAULT_E] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_DEFAULT_E]);
	m_AttackVolumes[VOLUME_DEFAULT_E]->TriggerActivate(false);

	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Root");
	TriggerDesc.vExtent = _float3(3.f, 3.f, 0.5f); // x, z 크게 y작게
	TriggerDesc.fAttackDmg = 400.f;
	m_AttackVolumes[VOLUME_KNOCKBACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_KNOCKBACK]);
	m_AttackVolumes[VOLUME_KNOCKBACK]->TriggerActivate(false);


	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 2.f); // x, z 크게 y작게
	m_AttackVolumes[VOLUME_SKILL] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_SKILL]);
	m_AttackVolumes[VOLUME_SKILL]->TriggerActivate(false);
	

	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비.
	TriggerDesc.vExtent = _float3(2.f, 2.f, 1.f); // x, z 크게 y작게
	TriggerDesc.fAttackDmg = 250.f;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Root"); // 기본은 Root?
	m_AttackVolumes[VOLUME_TARGET] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_TARGET]);
	m_AttackVolumes[VOLUME_TARGET]->TriggerActivate(false);


	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비.
	TriggerDesc.vExtent = _float3(8.f, 8.f, 5.f); // 궁극기 => 타겟 주위 강력한 범위형 장판 데미지
	TriggerDesc.fAttackDmg = 1570.f;
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Root"); // 기본은 Root?
	m_AttackVolumes[VOLUME_TARGET_BURST] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_TARGET_BURST]);
	m_AttackVolumes[VOLUME_TARGET_BURST]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_ARROUND];
	m_pMainAttackVolume->TriggerActivate(false);
}

CGalbrena* CGalbrena::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGalbrena* pInstance = new CGalbrena(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CGalbrena");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CGalbrena::Clone(void* pArg)
{
    CGalbrena* pInstance = new CGalbrena(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CGalbrena");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGalbrena::Free()
{
    CCharacter::Free();
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}
	m_AttackVolumes.clear();
	Safe_Release(m_pWing);
	Safe_Release(m_pGalbrenaFirstShotGun);
	Safe_Release(m_pGalbrenaSecondShotGun);
	Safe_Release(m_pGalbrenaDarkWing);
}
