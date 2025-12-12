#include "ClientPch.h"
#include "Rover.h"
#include "Player.h"
#include "SpringCamera.h"
#include "RoverSword.h"
#include "RoverDarkWing.h"
#include "RoverDarkScythe.h"

#include "Wing.h"
#include "RoverFactory.h"
#include "Collider.h"

#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CRover::CRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CRover::CRover(const CRover& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CRover::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRover::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CCharacter::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eCurLevel;
    Ready_Components(pDesc);
    Ready_Positions(pDesc);
    Ready_PartObjects(pDesc); // Parts 추가.
	Ready_AttackVolumes();
    Register_AllNotifies(pDesc->strFolderPath);

	CRoverFactory::Register_States(m_pStateMachineCom, this);
	
	
	
	Ready_Variables(pDesc);



    return S_OK;
}

void CRover::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));

	// 1. Dissovle 체크
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

	if (!IsDissolve)
	{
		// 2. Delayed Action 수행.
		Process_DelayedActions(fTimeDelta);

		// 3. 이전 위치 저장
		m_pTransformCom->Save_PreviousPosition();

		// 4. 몬스터가 있다면?
		Update_TargetDistance(fTimeDelta);
	}
	

	// 5. Parts 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Priority_Update(fTimeDelta);
	}

   
	
}

void CRover::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

	
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));


	

	if (!IsDissolve)
	{
		// 특정 상황일 때 TimeLack 감소.
		_float fTimeLack = m_pGameSystem->TimeLack(COLLISIONLAYER::PLAYER);
		
		// 2. 상태 머신 갱신
		m_pStateMachineCom->Update(fTimeDelta * m_fStateTimeRate * fTimeLack); // 여기서 Weapon이나 Parts의 갱신을 해야함.. => 여기서 Play_Animation 실행됨.
		// 3. Physcis 업데이트
		Update_Physics(fTimeDelta);
		// 4. 카메라 업데이트
		Update_Camera(fTimeDelta);
	}

	// 5. 파츠 갱신.?
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Update(fTimeDelta);
	}


	// 6. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);

}
void CRover::Late_Update(_float fTimeDelta)
{
   
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));

	if (!IsDissolve)
	{
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

	// 1. 파츠 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Late_Update(fTimeDelta);
	}
}

void CRover::Render()
{
    Bind_Resources();

	// 1. Dissolve 체크.
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		_float fDissolveRate = (m_fDissolveTimer / m_fMaxDissolveTime);
		if (FAILED(m_pShaderCom->Bind_Value("g_fDissolveRate", &fDissolveRate, sizeof(_float))))
			CRASH("Failed Bind Dissolve Rate");

		if (FAILED(m_pShaderCom->Bind_Value("g_vDissolveColor", &m_vDissolveColor, sizeof(_float4))))
			CRASH("Ready EnergyColor");

		if (FAILED(m_pShaderCom->Bind_Value("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
			CRASH("Ready EmissiveIntensity")
	}

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
		if (IsSkin(i))
			Render_Skin(i);
		else if (IsEye(i))
			Render_Eye(i);
		else if (IsMask(i))
			Render_Mask(i);
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

	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG

}

void CRover::Render_OutLine()
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

void CRover::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::SHADOW));

		m_pModelCom->Render(i);
	}
}


// 캐릭터 전환시 Idle로 상태 전환..
void CRover::TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType, void* pArg)
{
	m_pStateMachineCom->Exit_State();

	switch (eTransitionType)
	{
		case CHARACTER_TRANSITIONTYPE::IDLE:
		{
			// 애니메이션 변경할 값.
			GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
			break;
		}
		case CHARACTER_TRANSITIONTYPE::QTE:
		{
			_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

			// 내 앞에서 생성. (안 곂치게)
			_vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
			
			vPos += vLook * 5.f;
			vPos += XMVectorSet(0.f, 1.f, 0.f, 0.f); // 약간 띄우기.
			m_pColliderCom->Set_Position(vPos);
			m_pColliderCom->IsActivate(true);

			m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.5f, 0.5f);
			// 애니메이션 변경할 값.
			GetStateContextForWrite().m_eQTEType = ERoverQTEType::SKILL_QTE;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::QTE));
			break;
		}
		case CHARACTER_TRANSITIONTYPE::LEVIATAN_QTE:
		{
			if (nullptr == pArg)
				return;

			GetStateContextForWrite().m_eEventType = ERoverEventType::BEHIT_FLY_FALL;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::INTREACTION), ENUM_CLASS(ERoverInteractionState::EVENT), pArg);
		}
		break;

	}

	// 상태 변수 초기화
	m_IsQTE = false;
	m_StateContext.Clear();
}



	// AnimName이 같은걸로 매핑되어있음.
void CRover::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_WING:
		m_pWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	
    default:
        break;
    }
}

void CRover::PartActivate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Activate(IsActive);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Activate(IsActive);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Activate(IsActive);
		break;
	case PART_WING:
		m_pWing->Activate(IsActive);
		break;
    default:
        break;
    }
}

void CRover::Part_VolumeChange(_uint iPartType, _uint iVolumeIdx)
{
	switch (iPartType)
	{
	case PART_SWORD:
		m_pRoverSword->Change_Volume(iVolumeIdx);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Change_Volume(iVolumeIdx);
		break;
	}
}

void CRover::Part_VolumeActivate(_uint iPartType, _bool IsActive)
{
	switch (iPartType)
	{
	case PART_SWORD:
		m_pRoverSword->Volume_Activate(IsActive);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Volume_Activate(IsActive);
		break;
	}
}

void CRover::Clear_PartAnimation(_uint iPartType, const _string& strAnimName)
{
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Clear_Animation(strAnimName);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Clear_Animation(strAnimName);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Clear_Animation(strAnimName);
		break;
	case PART_WING:
		m_pWing->Clear_Animation(strAnimName);
		break;
    default:
        break;
    }

}

void CRover::Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName)
{
    ASSERT_CRASH(m_pModelCom);
    
    const _float4x4* pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());
    if (nullptr == pSocketMatrix)
        pSocketMatrix = &m_MatrixIdentity;
    
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Set_SocketMatrix(pSocketMatrix);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Set_SocketMatrix(pSocketMatrix);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Set_SocketMatrix(pSocketMatrix);
		break;
	case PART_WING:
		m_pWing->Set_SocketMatrix(pSocketMatrix);
		break;
    }
}

// Hit 판정.
void CRover::Hit_Judge(void* pArg)
{
	if (nullptr == pArg || m_IsHit)
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

	// 3. 데이터 저장.
	CCharacter::HIT_DESC* pDesc = static_cast<HIT_DESC*>(pArg);
	m_PendingHitDesc = *pDesc;

	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)); // 회피 가능
	m_fDodgeableHitTimer = m_fDodgeableDuration;


	// 4. 맞았을떄 시간 느리게 하기? => 이때 Attack이라면? 무시. => 다른 스킬 조건들은 Invincible 상태라 예외처리할 필요성 X
	_bool IsAttack = eKey.iCategory == ENUM_CLASS(EStateCategory::GROUND)
		&& eKey.iSubState == ENUM_CLASS(ERoverGroundState::ATTACK);

	_bool IsSpecialAttack = eKey.iCategory == ENUM_CLASS(EStateCategory::GROUND)
		&& eKey.iSubState == ENUM_CLASS(ERoverGroundState::SPECIAL);

	if (!IsAttack && !IsSpecialAttack)
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.3f, 0.1f); // Dodge 시간 동안 느리게하기? => 0.05로 해야 0.5f?
	else
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.3f, 0.1f); // Attack은 살짝만 느려지게

	
	//m_DelayedActions.push(DELAYED_ACTION(DELAYED_ACTION::TYPE::HIT, pDesc));


}

void CRover::Grab_Judge(void* pArg)
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

void CRover::Resolve_PerfectDodge()
{
	// 1. 회피 가능 상태인지 확인.
	if (!Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)))
		return;

	// 2. 조건 플래그 제거.
	Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	// 3. (데미지 무효화)
	while (!m_DelayedActions.empty())
		m_DelayedActions.pop();

	m_PendingHitDesc = {}; // 펜딩된 정보 초기화
	m_PendingConditions[HIT] = false; // 맞고 있다는 사실 취소

	m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.1f, 0.02f); // Time Lack

	// 퍼펙트 닷지가 성공했을 경우에만.
	CAMERA_SHAKE Desc{};
	Desc.fDuration = 0.15f;
	Desc.fFrequency = 20.f;
	Desc.fAmplitude = 0.5f;
	Desc.vRotation = { 0.f, 0.1f, 0.f };
	Desc.fFovKick = 0.f; // 

	m_pGameInstance->OnShake(Desc);

	Spawn_Effect(TEXT("Common_Limit"));
}

void CRover::Sync_Position()
{
    m_pColliderCom->Sync_Position(m_pTransformCom);
}

void CRover::Bind_QTE(_bool IsQTE)
{
	m_IsQTE = IsQTE;

	if (m_IsQTE)
	{
		// Activate
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

		// 내 앞에서 생성. (안 곂치게)
		_vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));
		_vector vUp = XMVectorSet(0.f, 2.f, 0.f, 0.f);
		vPos += vLook * 5.f;
		m_pQTEColliderCom->Set_Position(vPos);
		m_pQTEColliderCom->IsActivate(true);

		SetActivate(true);
		GetStateContextForWrite().m_eQTEType = ERoverQTEType::SKILL_QTE;
		Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::QTE));
	}
}

void CRover::Reset_QTECamera()
{
	m_fCameraOffset = m_fCameraOriginOffset;
}

void CRover::Bind_QTECamera()
{
	m_fCameraOriginOffset = m_fCameraOffset;
	m_fCameraOffset = 2.f; // 늘립니다.
}

void CRover::Attach_ThrowTarget(_bool isAttach)
{
	if (!m_ThrowInfo.IsActive) // 객체가 활성화 되어있지 않은 객체라면?
		return;

	if (isAttach)
	{
		const _float4x4* pBoneMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp01");
		const _float4x4* pWorldMatrix = m_pTransformCom->Get_WorldMatrixPtr();

		*m_ThrowInfo.ppRefBoneMatrix = pBoneMatrix;
		*m_ThrowInfo.ppRefWorldMatrix = pWorldMatrix;
		*m_ThrowInfo.pGrabbed = true;
		*m_ThrowInfo.pThrow = false;
	}
	else
	{
		*m_ThrowInfo.pGrabbed = false;
		*m_ThrowInfo.pThrow = false;
	}
}

void CRover::Throw_AttachTarget()
{
	if (!m_ThrowInfo.IsActive)
		return;

	*m_ThrowInfo.pGrabbed = false;
	*m_ThrowInfo.pThrow = true;
}



#pragma region NOTIFY
void CRover::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
  /*  if (wStrColliderTag == TEXT("Body"))
    {
		m_pColliderCom->IsActivate(IsActive);
    }*/

	_wstring var1, var2, var3;
	wstringstream wss(wStrColliderTag);
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)
	_uint iVolumeIdx = {  };

	if (var1 == TEXT("Main"))
		m_pMainAttackVolume->TriggerActivate(IsActive);


    if (var1 == TEXT("Sword"))
    {
		if (var2 == TEXT("ATK"))
			iVolumeIdx = CRoverSword::VOLUME::VOLUME_ATTACK;

		if (var3 == TEXT("ATTACK"))
			m_pRoverSword->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pRoverSword->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pRoverSword->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pRoverSword->Volume_Activate(IsActive);
    }
	else if (var1 == TEXT("Scythe"))
	{
		if (var2 == TEXT("ATK"))
			iVolumeIdx = CRoverDarkScythe::VOLUME::VOLUME_ATTACK;

		if (var3 == TEXT("ATTACK"))
			m_pRoverDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pRoverDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pRoverDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pRoverDarkScythe->Volume_Activate(IsActive);
	}
	else if (var1 == TEXT("DarkWing"))
	{

		if (var2 == TEXT("ATK"))
			iVolumeIdx = CRoverDarkScythe::VOLUME::VOLUME_ATTACK;

		if (var3 == TEXT("ATTACK"))
			m_pRoverDarkWing->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pRoverDarkWing->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pRoverDarkWing->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pRoverDarkWing->Volume_Activate(IsActive);
	}
	else if (var1 == TEXT("Rover"))
	{
		if (var2 == TEXT("KNOCKBACK"))
			iVolumeIdx = VOLUME::VOLUME_KNOCKBACK;
		else if (var2 == TEXT("SKILL"))
			iVolumeIdx = VOLUME::VOLUME_SKILL;

		m_pMainAttackVolume->TriggerActivate(false); // 교체.
		m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];

		if (var3 == TEXT("ATTACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::SKILL);

		m_pMainAttackVolume->TriggerActivate(IsActive);
	}
}

void CRover::Effect_Active(const _wstring& wStrEffectTag)
{
    if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
        return;

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &effecInfo);
}
void CRover::Object_Func(const _wstring& wStrObjectTag)
{
	// 3개의 변수 준비
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);

	// std::getline을 사용하여 L'|' 구분자를 만날 때까지 읽어 변수에 저장합니다.
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)

	// GalbrenaWing|Bone
	// 자르는거야.
	if (var1 == TEXT("StateDelay")) // 애니메이션 State의 속도를 Delay 시킵니다.
	{
		m_fStateTimeRate = stof(var2);
		m_fStateDelayTimer = stof(var3);
		Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::STATE_DELAY));
	}

	if (var1 == TEXT("DarkScythe"))
	{
		if (var2 == TEXT("Activate"))
		{
			if (var3 == TEXT("false"))
				PartActivate(PARTTYPE::PART_DARKSCYTHE, false);
			else if(var3 == TEXT("true"))
				PartActivate(PARTTYPE::PART_DARKSCYTHE, true);
		}
	}

	if (var1 == TEXT("DarkWing"))
	{
		if (var2 == TEXT("Activate"))
		{
			if (var3 == TEXT("false"))
				PartActivate(PARTTYPE::PART_DARKWING, false);
			else if (var3 == TEXT("true"))
				PartActivate(PARTTYPE::PART_DARKWING, true);
		}
	}
	else if (var1 == TEXT("Throw"))
		Throw_AttachTarget(); // 던지기.
	else if (var1 == TEXT("Sound"))
		Process_PlaySound(wStrObjectTag); // Character 함수.
	else if (var1 == TEXT("MotionTrail"))
		Process_MotionTrail(wStrObjectTag);

	

}
void CRover::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 1. 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::ROVER));

	if (nullptr == pAbility)
		return;

	switch (m_iVolumeIdx)
	{
	case VOLUME::VOLUME_KNOCKBACK: // 기본 공격시 공명 게이지와 궁게이지 채우기
		pAbility->Add_HarmonyGauge(7.f); // 공명 게이지 채우기.
		pAbility->Add_Cost(COST_TYPE::COST1, 5.f); // 궁 ULTI
		pAbility->Add_Cost(COST_TYPE::COST5, 5.f); // 궁 ULTI
		break;

	default:
		pAbility->Add_Cost(COST_TYPE::COST5, 5.f); // 궁 ULTI
		break;
	}
}

#pragma endregion

#pragma region 4. EVENT
void CRover::Process_DelayedActions(_float fTimeDelta)
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
			m_pAbillityCom->Add_Hp(-10.f); // 최소 피해량.
			break;
		}
		case DELAYED_ACTION::TYPE::GRAB:
		{
			ActiveCaptureState();
			GetStateContextForWrite().m_eCaptureType = ERoverCaptureType::BEHIT_FLY_START;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::CAPTURED), ENUM_CLASS(ERoverCaptureState::CAPTURE));
			break;
		}

		default:
			break;
		}

		m_DelayedActions.pop();
	}
}
void CRover::Bind_ChangeEffect()
{
	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();

	m_pGameInstance->Spawn_PoolingObject(TEXT("Common_SwapEffect"), matWorld, &effecInfo);
}
void CRover::Bind_DissolveTimer()
{
	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	m_fDissolveTimer = 0.f;
}
void CRover::Bind_DefaultShaderPath()
{
	// 기본 Shader Path
	for (_uint i = 0; i < MESHTYPE::MESH_END; ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::ROVER);

	m_ShaderPaths[MESH_MASK] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::ROVERMASK);
}
void CRover::Bind_DissolveShaderPath()
{
	for (_uint i = 0; i < MESHTYPE::MESH_END; ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::DISSOLVE_CHARACTER);
}
void CRover::Activate(_bool IsActivate)
{
	//m_isActivate = IsActivate;
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



void CRover::Update_Physics(_float fTimeDelta)
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

void CRover::Update_Camera(_float fTimeDelta)
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

void CRover::Update_TargetDistance(_float fTimeDelta)
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

void CRover::Render_Default(_uint iMeshIndex)
{
	if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TEXTURETYPE::DIFFUSE, 0)))
		return;

	_bool HasNormal = { false };
	if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", iMeshIndex, TEXTURETYPE::NORMAL, 0)))
		HasNormal = true;

	if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
		CRASH("Ready g_HasNormal Failed");
}

void CRover::Render_Skin(_uint iMeshIndex)
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

void CRover::Render_Eye(_uint iMeshIndex)
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
		m_ShaderPaths[iMeshIndex] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::ROVER);
}

void CRover::Render_Mask(_uint iMeshIndex)
{
	if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TEXTURETYPE::DIFFUSE, 0)))
		return;

	_bool HasNormal = { false };
	if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
		CRASH("Ready g_HasNormal Failed");
}



_bool CRover::IsSkin(_uint iMeshIndex)
{
	if (iMeshIndex == MESH_FACE ||
		iMeshIndex == MESH_UP)
		return true;

	return false;
}

_bool CRover::IsEye(_uint iMeshIndex)
{
	if (iMeshIndex == MESH_EYE)
		return true;

	return false;
}

_bool CRover::IsMask(_uint iMeshIndex)
{
	if (iMeshIndex == MESHTYPE::MESH_MASK)
		return true;
	return false;
}

void CRover::Process_MotionTrail(const _wstring& wStrObjectTag)
{
	_wstring var1, var2, var3, var4, var5;
	wstringstream wss(wStrObjectTag);

	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|');
	getline(wss, var4, L'|');
	getline(wss, var5, L'|');

	_float fDuration = stof(var2);
	_float fInterval = stof(var3);
	_float fMotionLifeTime = stof(var4);
	_uint iShaderPath = stoul(var5);

	// Color는 고정?
	Spawn_MotionTrail(fDuration, fInterval, fMotionLifeTime, m_vMotionTrailColor, iShaderPath);
}

void CRover::Bind_Resources()
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

void CRover::Ready_Components(const CHARACTER_DESC* pDesc)
{
    // 1. Components
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
	cout << "Rover Model Clone : " << endl;
#endif // _DEBUG

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
        , pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->stateMachineData.first)
        , pDesc->stateMachineData.second, TEXT("Com_StateMachine"), reinterpret_cast<CComponent**>(&m_pStateMachineCom), nullptr)))
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

void CRover::Ready_Variables(const CHARACTER_DESC* pDesc)
{
	m_fDodgeableDuration = 0.1f; // Dodge 가능 시간.

    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::ROVER);

	// Shader Vlaue 추가
	m_fDissolveTimer = 0.f;
	m_fMaxDissolveTime = 0.35f;
	m_vDissolveColor = { 0.693f, 0.481f, 1.f, 1.f };
	m_fEmissiveIntensity = 1.5f;

	m_ShaderPaths[MESH_MASK] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::ROVERMASK);
	m_vMotionTrailColor = {0.1f, 0.1f, 0.1f, 0.7f};


	// 비활성화. 
	PartActivate(PART_SWORD, false);
	PartActivate(PART_DARKWING, false);
	PartActivate(PART_DARKSCYTHE, false);
	PartActivate(PART_WING, false);


	m_IsQTE = false; // QTE
	XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());

	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1000.f, 0.f, 0.f);
	XMStoreFloat4(&m_vQTEPos, vPos);
	m_pQTEColliderCom->Set_Position(vPos);
}

void CRover::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

}


void CRover::Ready_PartObjects(const CHARACTER_DESC* pDesc)
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
        case PARTTYPE::PART_SWORD:

            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetRoverWeaponCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


            // PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

			m_pRoverSword = dynamic_cast<CRoverSword*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pRoverSword);
            Safe_AddRef(m_pRoverSword);
            break;
		case PARTTYPE::PART_DARKWING:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetRoverDarkWingCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("DarkWing");

			m_pRoverDarkWing = dynamic_cast<CRoverDarkWing*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pRoverDarkWing);
			Safe_AddRef(m_pRoverDarkWing);
			break;
		case PARTTYPE::PART_DARKSCYTHE:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetRoverDarkScytheCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("DarkScythe");

			m_pRoverDarkScythe = dynamic_cast<CRoverDarkScythe*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pRoverDarkScythe);
			Safe_AddRef(m_pRoverDarkScythe);
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
				CRASH("Weapon");

			m_pWing = dynamic_cast<CWing*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pWing);
			Safe_AddRef(m_pWing);
			break;
		}
    }
}

void CRover::Ready_AttackVolumes()
{
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::BONE; // 뼈
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp01");
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::KNOCKBACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(1.f, 1.f, 1.f); // x, z 크게 y작게
	TriggerDesc.vOffsetPos = _float3(0.0f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 400.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::DARK;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};


	m_AttackVolumes[VOLUME_KNOCKBACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_KNOCKBACK]);
	m_AttackVolumes[VOLUME_KNOCKBACK]->TriggerActivate(false);


	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(20.f, 20.f, 20.f); // x, z 크게 y작게
	m_AttackVolumes[VOLUME_SKILL] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_SKILL]);
	m_AttackVolumes[VOLUME_SKILL]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_KNOCKBACK];
	m_pMainAttackVolume->TriggerActivate(false);
}

CRover* CRover::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRover* pInstance = new CRover(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CRover::Clone(void* pArg)
{
    CRover* pInstance = new CRover(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CRover::Free()
{
    CCharacter::Free();
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}

	m_AttackVolumes.clear();
    Safe_Release(m_pRoverSword);
    Safe_Release(m_pRoverDarkWing);
    Safe_Release(m_pRoverDarkScythe);
	Safe_Release(m_pWing);
}
