#include "ClientPch.h"
#include "Augusta.h"
#include "Player.h"
#include "SpringCamera.h"
#include "Collider.h"
#include "Ability.h"
#include "AugustaFactory.h"
#include "AugustaState_Enum.h"
#include "AugustaBayonet.h"
#include "AugustaSkillWeapon.h"
#include "AugustaGriffon.h"
#include "AttackVolume.h"
#include "Wing.h"
#include "GameSystem.h"


CAugusta::CAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CAugusta::CAugusta(const CAugusta& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CAugusta::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugusta::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CCharacter::Initialize_Clone(pDesc)))
        return E_FAIL;

	m_DelayedActions = queue<DELAYED_ACTION>();
    m_eCurLevel = pDesc->eCurLevel;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
    Ready_PartObjects(pDesc); // Parts 추가.
	Ready_AttackVolumes();
    Register_AllNotifies(pDesc->strFolderPath);

	//Register_AbilityFiles(pDesc->strAbilityFolderPath);

    CAugustaFactory::Register_States(m_pStateMachineCom, this);
	m_pBayonet->SetActivate(false);
    m_pSkillWeapon->SetActivate(false);
    m_pGriffon->SetActivate(false);
	m_pWing->SetActivate(false);
    
	
	m_IsQTE = false;
    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());
	
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1000.f, 0.f, 0.f);
	XMStoreFloat4(&m_vQTEPos, vPos);
	m_pQTEColliderCom->Set_Position(vPos);

	m_fDodgeableDuration = 0.1f; // Dodge 가능 시간.

	
	
    return S_OK;
}

void CAugusta::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	// 1. Delayed Action 수행.
	Process_DelayedActions(fTimeDelta);

	// 2. Parts 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Priority_Update(fTimeDelta);
	}

    // 3. 이전 위치 저장
	m_pTransformCom->Save_PreviousPosition();

	// 4. 몬스터가 있다면?
	Update_TargetDistance();
	
	// 5. Change Timer 계산. => Dissolve에 사용
	Calc_ChangeTimer(fTimeDelta);
}

void CAugusta::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_DIKeyState(DIK_8) == KEYSTATE::DOWN)
	{
		_float3 vCenterPos = {};
		XMStoreFloat3(&vCenterPos, m_pTransformCom->Get_State(STATE::POSITION));
		m_pGameInstance->Setting_DOF(vCenterPos, 50.f);
	}
#endif

    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;
	
	// 파츠 갱신.
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Update(fTimeDelta);
	}

    // 2. 상태 머신 갱신
    m_pStateMachineCom->Update(fTimeDelta * m_fStateTimeRate); // 여기서 Weapon이나 Parts의 갱신을 해야함.. => 여기서 Play_Animation 실행됨.

	
	// 3. Physcis 업데이트
	Update_Physics(fTimeDelta);
	// 4. 카메라 업데이트
	Update_Camera(fTimeDelta);
	//if (Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::GRABED)))
	//{
	//	if (nullptr != m_PendingCaptureDesc.pSocketMatrix &&
	//		nullptr != m_PendingCaptureDesc.pTransform)
	//	{
	//		
	//		_matrix matFinalWorld = XMLoadFloat4x4(m_PendingCaptureDesc.pSocketMatrix); // 1. 본행렬

	//		_vector vScale{}, vRotQuat{}, vTrans{};
	//		_vector vPlayerScale = XMVectorSet(1.f, 1.f, 1.f, 0.f);
	//		XMMatrixDecompose(&vScale, &vRotQuat, &vTrans, matFinalWorld);
	//		m_pTransformCom->Set_State(STATE::POSITION, vTrans);

	//		_vector vCameraLook = m_pSpringCamera->Get_LookVector_NoPitch(); // Camera Look을 
	//		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	//		vPos += vCameraLook * -3.f;
	//		m_pSpringCamera->Update_Target(vPos, 1.2f); // 카메라는 고정.
	//	}
	//}
	//else
	//{
	//	// 3. 현재 위치 - 1Frame 이전 위치 값 계산'
	//	_vector vVelocity = m_pTransformCom->Get_Velocity();
	//	if (!m_IsQTE)
	//	{
	//		// 4. Collider 갱신 => Jolt 자체에서도 fTimeDelta 값을 적용하고 있기 때문에 
	//		m_pColliderCom->Update(vVelocity / fTimeDelta);

	//		// 5. Camera 갱신 => 위치 따라오게
	//		m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 1.2f);
	//	}
	//	else
	//	{
	//		m_pQTEColliderCom->Update(vVelocity / fTimeDelta);
	//	}
	//	// 6. Land Check
	//	m_IsLand = Is_LandCollider();
	//}
	

	
	// 4. 어택 볼륨 갱신.
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Update(fTimeDelta);
	}
}
void CAugusta::Late_Update(_float fTimeDelta)
{
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

	

	// 3. 
	if (m_IsQTEend)
	{
		Notify_HarmonyEnd();
		m_pQTEColliderCom->Set_Position(XMLoadFloat4(&m_vQTEPos));
		m_IsQTEend = false;
	}

	// 1. 파츠 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Late_Update(fTimeDelta);
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
	
    
}

void CAugusta::Render()
{

    Bind_Resources();

	if (Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE)))
	{
		// Shader에 값 바인딩.. => 나중에 Shader Path 생성 필요,
	}

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

		_bool HasNormal = { false };

		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		_bool HasMask = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, 0)))
			HasMask = true;

		if(FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		if (FAILED(m_pShaderCom->Bind_Value("g_HasSkinMask", &HasMask, sizeof(_bool))))
			CRASH("Ready g_HasSkinMask Failed");

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pModelCom->Bind_MorphedResult(m_pShaderCom, i, "g_MorphedVertices")))
			CRASH("Bind Morph Result Failed");


		if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
			CRASH("Ready Shader Begin Failed");
        //if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
        //    CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");

		m_pShaderCom->UndBind_All_VS_SRV();
    }

#ifdef _DEBUG
	/*if (!m_IsQTE)
		m_pColliderCom->Render();
	else
		m_pQTEColliderCom->Render();*/
	m_pColliderCom->Render();
	//m_pQTEColliderCom->Render();
    
	Print_LookRay();
	
	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG
}

void CAugusta::Render_Shadow()
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

void CAugusta::Render_OutLine()
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

void CAugusta::TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType)
{
	// 현재 애니메이션 제거.
	m_pStateMachineCom->Exit_State();

	// 애니메이션 변경할 값.
	switch (eTransitionType)
	{
		case CHARACTER_TRANSITIONTYPE::IDLE:
		{
			GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION02;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
			break;
		}
		
		case CHARACTER_TRANSITIONTYPE::QTE:
		{
			_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

			// 내 앞에서 생성. (안 곂치게)
			_vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State(STATE::LOOK), 0.f));

			vPos += vLook * -1.f;
			vPos += XMVectorSet(0.f, 1.f, 0.f, 0.f); // 약간 띄우기.
			m_pColliderCom->Set_Position(vPos);
			m_pColliderCom->IsActivate(true);

			m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.5f, 0.5f);

			GetStateContextForWrite().m_eQTEType = EAugustaQTEType::SKILLQTE;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::QTE));
			break;
		}
	}
	
	// 상태 변수 초기화
	m_StateContext.Clear();

	// QTE 플래그 강제 리셋.
	m_IsQTE = false;
}

// AnimName이 같은걸로 매핑되어있음.
void CAugusta::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _bool IsLoop)
{
    switch (iPartType)
    {
    case PART_BAYONET:
		m_pBayonet->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    case PART_SKILLWEAPON:
		m_pSkillWeapon->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    case PART_GRIFFON:
		m_pGriffon->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    case PART_WING:
		m_pWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    }
}

void CAugusta::PartActivate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
    case PART_BAYONET:
        m_pBayonet->Activate(IsActive);
		
        break;
    case PART_SKILLWEAPON:
        m_pSkillWeapon->Activate(IsActive);
        break;
    case PART_GRIFFON:
        m_pGriffon->Activate(IsActive);
        break;
	case PART_WING:
		m_pWing->Activate(IsActive);
		break;
    }
}

void CAugusta::Part_VolumeChange(_uint iPartType, _uint iVolumeIdx)
{
	switch (iPartType)
	{
	case PART_BAYONET:
		m_pBayonet->Change_Volume(iVolumeIdx);
		break;
	case PART_SKILLWEAPON:
		m_pSkillWeapon->Change_Volume(iVolumeIdx);
		break;
	case PART_GRIFFON:
		m_pGriffon->Change_Volume(iVolumeIdx);
		break;
	case PART_WING:
		m_pWing->Change_Volume(iVolumeIdx);
		break;
	}
}

void CAugusta::Part_VolumeActivate(_uint iPartType, _bool IsActive)
{
	switch (iPartType)
	{
	case PART_BAYONET:
		m_pBayonet->Volume_Activate(IsActive); // MainVolume 켜기
		break;
	case PART_SKILLWEAPON:
		m_pSkillWeapon->Volume_Activate(IsActive); // MainVolume 켜기
		break;
	case PART_GRIFFON:
		m_pGriffon->Volume_Activate(IsActive); // MainVolume 켜기
		break;
	case PART_WING:
		break;
	}
}

void CAugusta::Clear_PartAnimation(_uint iPartType, const _string& strAnimName)
{
    switch (iPartType)
    {
    case PART_BAYONET:
        m_pBayonet->Clear_Animation(strAnimName);
        break;
    case PART_SKILLWEAPON:
        m_pSkillWeapon->Clear_Animation(strAnimName);
        break;
    case PART_GRIFFON:
        m_pGriffon->Clear_Animation(strAnimName);
        break;
	case PART_WING:
		m_pWing->Clear_Animation(strAnimName);
		break;
    }
}

void CAugusta::Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName)
{
    ASSERT_CRASH(m_pModelCom);
    
    const _float4x4* pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());
    if (nullptr == pSocketMatrix)
        pSocketMatrix = &m_MatrixIdentity;
    
    switch (iPartType)
    {
    case PART_BAYONET:
        m_pBayonet->Set_SocketMatrix(pSocketMatrix);
        break;
    case PART_SKILLWEAPON:
        m_pSkillWeapon->Set_SocketMatrix(pSocketMatrix);
        break;
    case PART_GRIFFON:
        m_pGriffon->Set_SocketMatrix(pSocketMatrix);
        break;
    }
}


// Hit 판정. => QTE 상태면 안맞음.
void CAugusta::Hit_Judge(void* pArg)
{
	if (nullptr == pArg || m_IsHit || m_PendingConditions[QTE])
		return;

	_uint iFlag = {};
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::DODGE);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::HIT);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::INVINCIBLE);
	iFlag |= ENUM_CLASS(CHARACTER_CONDITION::GRABED);

	// 컨디션 체크
	if (Check_AnyCondition(iFlag))
		return;

	StateKey eKey = m_pStateMachineCom->Get_CurrentStateKey();
	_uint iCategory = eKey.iCategory;
	_uint iSubState = eKey.iSubState;

	EStateCategory eCategory = static_cast<EStateCategory>(iCategory);

	// 1. 맞는데 또맞지 않기
	if (EStateCategory::HIT == eCategory)
		return;

	// 2. 즉시 중복 방지 플래그 세팅
	// m_PendingConditions[HIT] = true;


	// 3. 피격 정보 데이터 저장.
	CCharacter::HIT_DESC* pDesc = static_cast<HIT_DESC*>(pArg);
	//m_pAbillityCom->Add_Hp(-pDesc->fAttack);
	m_PendingHitDesc = *pDesc;

	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)); // 회피 가능
	m_fDodgeableHitTimer = m_fDodgeableDuration;

	
	// 4. 맞았을떄 시간 느리게 하기? => 이때 Attack이라면? 무시.
	// => 다른 스킬 조건들은 Invincible 상태라 예외처리할 필요성 X
	_bool IsAttack = eKey.iCategory == ENUM_CLASS(EStateCategory::GROUND) 
		&& eKey.iSubState == ENUM_CLASS(EAugustaGroundState::ATTACK);

	if (!IsAttack)
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.1f, 0.05f); // Dodge 시간 동안 느리게하기? => 0.05로 해야 0.5f?
	else
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.3f, 0.1f);


	
	
}


// 패링 판단.
void CAugusta::Parry_Judge(void* pArg)
{

	if (m_PendingConditions[HIT] || m_PendingConditions[QTE] || m_PendingConditions[PARRY])
		return;

	// 1. 패링 시 ? Layer 변경? => 잠시 무적
	CCharacter::PARRY_DESC* pDesc = static_cast<PARRY_DESC*>(pArg);
}

void CAugusta::Grab_Judge(void* pArg)
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

void CAugusta::Resolove_PerfectDodge()
{
	// 1. 회피 가능 상태인지 확인.
	if (!Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE)))
		return;

	// 2. 조건 플래그 제거.
	Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DODGEABLE));

	// 3. (데미지 무효화)
	// DelayedActions 큐를 비워버리거나, HIT 타입만 제거하는 로직 필요
	while (!m_DelayedActions.empty())
	{
		DELAYED_ACTION eAction = m_DelayedActions.front();
		if (DELAYED_ACTION::TYPE::HIT == eAction.type) // Hit 면 정보 날리기.
			m_DelayedActions.pop();
	}

	m_PendingHitDesc = {}; // 펜딩된 정보 초기화
	m_PendingConditions[HIT] = false; // 맞고 있다는 사실 취소

	m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 1.0f, 0.1f); // 시간 복구
}


void CAugusta::Sync_Position()
{
    m_pColliderCom->Sync_Position(m_pTransformCom);
}

void CAugusta::Bind_QTE(_bool IsQTE)
{
	m_IsQTE = IsQTE;

	if (m_IsQTE)
	{
		// Activate
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

		// 내 앞에서 생성. (안 곂치게)
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
		vPos += vLook * 1.5f;
		vPos += XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP)) * 1.5f;
		m_pQTEColliderCom->Set_Position(vPos);
		m_pQTEColliderCom->IsActivate(true);

		SetActivate(true);
		GetStateContextForWrite().m_eQTEType = EAugustaQTEType::SKILLQTE;
		Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::QTE));
	}
}

void CAugusta::Reset_QTECamera()
{
	m_fCameraOffset = m_fCameraOriginOffset;
}

void CAugusta::Bind_QTECamera()
{
	m_fCameraOriginOffset = m_fCameraOffset;
	m_fCameraOffset = 2.f; // 늘립니다.
}




#pragma region NOTIFY
void CAugusta::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);


	_wstring var1, var2, var3;
	wstringstream wss(wStrColliderTag);
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)
	_uint iVolumeIdx = {  };

	if (var1 == TEXT("Main"))
		m_pMainAttackVolume->TriggerActivate(IsActive);

	// Main Attack Volume의 TriggerActivate
	if (var1 == TEXT("Bayonet"))
	{
		
		if (var2 == TEXT("ATK"))
			iVolumeIdx = CAugustaBayonet::VOLUME::VOLUME_ATTACK;
		if (var2 == TEXT("STRATK"))
			iVolumeIdx = CAugustaBayonet::VOLUME::VOLUME_STRONG_ATTACK;

		m_pBayonet->Change_Volume(iVolumeIdx);
		if (var3 == TEXT("ATTACK"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if(var3 == TEXT("KNOCKBACK"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pBayonet->Volume_Activate(IsActive);
	}
	else if (var1 == TEXT("SkillWeapon"))
	{
		if (var2 == TEXT("ULTI"))
			iVolumeIdx = CAugustaSkillWeapon::VOLUME::VOLUME_ULTI;
		else if (var2 == TEXT("SWORD_ATTACK"))
			iVolumeIdx = CAugustaSkillWeapon::VOLUME::VOLUME_SWORD_ATTACK;
		else if (var2 == TEXT("SWORD_ULTI"))
			iVolumeIdx = CAugustaSkillWeapon::VOLUME::VOLUME_SWORD_ULTI;

		if (var3 == TEXT("ATTACK"))
			m_pSkillWeapon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pSkillWeapon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pSkillWeapon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pSkillWeapon->Volume_Activate(IsActive);
	}
	else if (var1 == TEXT("Griffon"))
	{
		if (var2 == TEXT("STRIKE"))
			iVolumeIdx = CAugustaGriffon::VOLUME::VOLUME_STRIKE;

		m_pGriffon->Change_Volume(iVolumeIdx);
		if (var3 == TEXT("ATTACK"))
			m_pGriffon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pGriffon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pGriffon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pGriffon->Volume_Activate(IsActive);
	}
	else if (var1 == TEXT("Augusta"))
	{
		if (var2 == TEXT("RISE_ZERO"))
			m_iVolumeIdx = VOLUME::VOULME_RISE_ZERO;
		else if (var2 == TEXT("RISE"))
			m_iVolumeIdx = VOLUME::VOLUME_RISE;
		else if (var2 == TEXT("HACKDOWN"))
			m_iVolumeIdx = VOLUME::VOLUME_HACKDOWN;

		m_pMainAttackVolume->TriggerActivate(false); // 교체.
		m_pMainAttackVolume = m_AttackVolumes[m_iVolumeIdx];

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var3 == TEXT("ATTACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("SKILL"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::SKILL);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::KNOCKBACK);

		m_pMainAttackVolume->TriggerActivate(IsActive);
	}

}
void CAugusta::Effect_Active(const _wstring& wStrEffectTag)
{
    if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
        return;

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

    _matrix matWorld = m_pTransformCom->Get_WorldMatrix();
    m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &effecInfo);
}
void CAugusta::Object_Func(const _wstring& wStrObjectTag)
{

	// 3개의 변수 준비
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);

	// std::getline을 사용하여 L'|' 구분자를 만날 때까지 읽어 변수에 저장합니다.
	getline(wss, var1, L'|');
	if (var1 == TEXT("HITSTOP"))
		Process_HitStop(wStrObjectTag);
	else if (var1 == TEXT("CAMERA"))
		Process_CameraAction(wStrObjectTag);
	else if (var1 == TEXT("StateDelay")) // 애니메이션 State의 속도를 Delay 시킵니다.
	{
		m_fStateTimeRate = stof(var2);
		m_fStateDelayTimer = stof(var3);
		Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::STATE_DELAY));
	}
	//else
	//	Process_VolumeChange(wStrObjectTag);

	return;
}

void CAugusta::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{

}


#pragma endregion

#pragma region 4. EVENT

// 지연 처리 작업
void CAugusta::Process_DelayedActions(_float fTimeDelta)
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
			m_DelayedActions.push({ DELAYED_ACTION::TYPE::HIT, &m_PendingHitDesc });
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
	
	// Dodge가 아닐때만 추가되므로.
	while (!m_DelayedActions.empty())
	{
		DELAYED_ACTION eAction = m_DelayedActions.front();

		void* pData = eAction.pData;
		switch (eAction.type)
		{
			// 여기서 깎으면 된다. => Skill 도중엔 Dodge가 안되니까?
			case DELAYED_ACTION::TYPE::HIT:
			{
				//m_IsHit = true;
				Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::HIT)); // Condition 추가.
				m_pAbillityCom->Add_Hp(-m_PendingHitDesc.fAttack);
				//m_pAbillityCom->Add_Hp(-10.f);
				break;
			}
			case DELAYED_ACTION::TYPE::GRAB:
			{
				ActiveCaptureState();
				GetStateContextForWrite().m_eCaptureType = EAugustaCaptureType::BEHIT_FLY_START;
				m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::CAPTURED), ENUM_CLASS(EAugustaCaptureState::CAPTURE));
				break;
			}
			
		default:
			break;
		}

		m_DelayedActions.pop();
	}
}
void CAugusta::Calc_ChangeTimer(_float fTimeDelta)
{
	if (m_fChangeTimer > 0.f)
		m_fChangeTimer -= fTimeDelta; // Dissolve 변수 값.
	else
	{
		m_fChangeTimer = 0.f;
		Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE));
	}
		
}

void CAugusta::Bind_ChangeEffect()
{
	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();

	m_pGameInstance->Spawn_PoolingObject(TEXT("Common_SwapEffect"), matWorld, &effecInfo);
}
void CAugusta::Render_Damage(const HIT_DESC* pDesc)
{
	_float4 vTextPosition = {};
	XMStoreFloat4(&vTextPosition, m_pTransformCom->Get_State(STATE::POSITION));
	//m_pGameSystem->Render_Damage(vTextPosition, static_cast<_int>(pDesc->fAttack), TEXT_COLOR_TYPE::TT_PROGRESS);
	m_pGameSystem->Render_Damage(vTextPosition, static_cast<_int>(pDesc->fAttack), TEXT_COLOR_TYPE::ELEC);
}
#pragma endregion


#pragma region HELPER 함수
void CAugusta::Process_HitStop(const _wstring& wStrObjectTag)
{
	wstringstream wss(wStrObjectTag);
	// 4개의 변수 준비
	_wstring var1, var2, var3, var4;

	getline(wss, var1, L'|'); // HITSTOP
	getline(wss, var2, L'|'); // Layer Tag
	getline(wss, var3, L'|'); // Rate
	getline(wss, var4, L'|'); // Duration

	_float fRate = stof(var3);
	_float fDuration = stof(var4);

	if (var2 == TEXT("ALL"))
	{
		// 캐릭터의 경우 전체 시간 감소.
		m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), fRate, fDuration);
	}

}
void CAugusta::Process_CameraAction(const _wstring& wStrObjectTag)
{
	_wstring Tag;
	_wstring Duration;
	_wstring Frequency; 
	_wstring Amplitude;
	_wstring FovKick;
	_wstring Intensity; // 강도
	_wstring Dir; // UD, LR

	/* CAMERA | Duration | Frequency | Amplitude | Intensity | FovKick | Dir*/
	wstringstream wss(wStrObjectTag);
	getline(wss, Tag, L'|');
	getline(wss, Duration, L'|'); 
	getline(wss, Frequency, L'|');
	getline(wss, Amplitude, L'|');
	getline(wss, Intensity, L'|');
	getline(wss, FovKick, L'|');
	getline(wss, Dir, L'|');

	CAMERA_SHAKE ShakeDesc = {};
	ShakeDesc.fDuration = stof(Duration);
	ShakeDesc.fFrequency = stof(Frequency);
	ShakeDesc.fAmplitude = stof(Amplitude);
	_float fIntensity = stof(Intensity);
	if (Dir == TEXT("UD"))
		ShakeDesc.vRotation = { fIntensity, 0.f, 0.f};
	else if (Dir == TEXT("LR"))
		ShakeDesc.vRotation = { 0.f, fIntensity ,0.f };
	else if (Dir == TEXT("UDLR"))
		ShakeDesc.vRotation = { fIntensity, fIntensity ,0.f };

	ShakeDesc.fFovKick = stof(FovKick);


	// 흔든다.
	m_pGameInstance->OnShake(ShakeDesc);
	return;
}
void CAugusta::Process_VolumeChange(const _wstring& wStrObjectTag)
{
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)

	_uint iVolumeIdx = stoul(var3);

	/* BAYONET|ATTACK|0*/
	// 1. 어떤 무기인가?
	if (var1 == TEXT("BAYONET"))
	{
		// 볼륨 인덱스로 볼륨 변경. (ATTACK (0), STRONG_ATTACK(1), ULTI(2) )
		m_pBayonet->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
		{
			// 3. 볼륨 레이어 변경
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		}
		else if (var2 == TEXT("SKILL"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);


	}
	else if (var1 == TEXT("GRIFFON"))
	{
		// 볼륨 인덱스로 볼륨 변경. (STRIKE (0))
		m_pGriffon->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
			m_pGriffon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK); // 3. 볼륨 레이어 변경
		else if (var2 == TEXT("SKILL"))
			m_pGriffon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pGriffon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
	}
	else if (var1 == TEXT("SKILLWEAPON"))
	{
		// 볼륨 인덱스로 볼륨 변경. (VOLUME_ULTI (0) VOLUME_SWORD_ATTACK(1), VOLUME_SWORD_ULTI(2) )
		m_pSkillWeapon->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
			m_pSkillWeapon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK); // 3. 볼륨 레이어 변경
		else if (var2 == TEXT("SKILL"))
			m_pSkillWeapon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pSkillWeapon->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
	}
	else if (var1 == TEXT("AUGUSTA"))
	{
		// 볼륨 인덱스로 볼륨 변경. (VOLUME_RISE (0), VOLUME_HACKDOWN(1))
		if (nullptr == m_AttackVolumes[iVolumeIdx] || nullptr == m_pMainAttackVolume)
			return;

		m_pMainAttackVolume->TriggerActivate(false); // 교체.
		m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::ATTACK);
		else if (var2 == TEXT("SKILL"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::KNOCKBACK);
	}
}

void CAugusta::Update_TargetDistance()
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

void CAugusta::Update_Physics(_float fTimeDelta)
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


void CAugusta::Update_Camera(_float fTimeDelta)
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



#pragma endregion




void CAugusta::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

void CAugusta::Ready_Components(const CHARACTER_DESC* pDesc)
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

void CAugusta::Ready_Variables(const CHARACTER_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::AUGUSTA);
}

void CAugusta::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}


void CAugusta::Ready_PartObjects(const CHARACTER_DESC* pDesc)
{

    _float3 vScale = {};
    _float3 vRotation = {};
    _float3 vPosition = {};

    for (_uint i = 0; i < PARTTYPE::TYPE_END; ++i)
    {
        _wstring strPartName = pDesc->PartPrototypes[i].first;
        _wstring strPrototypeName = pDesc->PartPrototypes[i].second;

        CProp::PROP_DESC Desc{};
		CAttackVolume::ATKVOLUME_DESC VolumeDesc{};

        switch (i)
        {
        case PARTTYPE::PART_BAYONET:
            
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaBayonetCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);
            

			// PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pBayonet = dynamic_cast<CAugustaBayonet*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pBayonet);
            Safe_AddRef(m_pBayonet);
            break;

        case PARTTYPE::PART_SKILLWEAPON:
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaSkillWeaponCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pSkillWeapon = dynamic_cast<CAugustaSkillWeapon*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pSkillWeapon);
            Safe_AddRef(m_pSkillWeapon);
            break;
        case PARTTYPE::PART_GRIFFON:
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaGriffonCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pGriffon = dynamic_cast<CAugustaGriffon*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pGriffon);
            Safe_AddRef(m_pGriffon);
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

void CAugusta::Ready_AttackVolumes()
{
	// size 설정
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::BONE; // 뼈
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Root");
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(4.f, 4.f, 8.f); // 
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 700.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::ELEC;
	TriggerDesc.eDir = ATTACKVOULME_DIR::UPPER;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
	};


	m_AttackVolumes[VOULME_RISE_ZERO] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOULME_RISE_ZERO]);
	m_AttackVolumes[VOULME_RISE_ZERO]->TriggerActivate(false);


	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(7.f, 7.f, 10.f); // 
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.eDir = ATTACKVOULME_DIR::DEFAULT;

	// Attack용 만들기.
	m_AttackVolumes[VOLUME_RISE] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_RISE]);
	m_AttackVolumes[VOLUME_RISE]->TriggerActivate(false);


	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(6.f, 6.f, 2.f); // y작게? x, z 평면 크게.
	TriggerDesc.eDir = ATTACKVOULME_DIR::UPPER;
	m_AttackVolumes[VOLUME_HACKDOWN] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_HACKDOWN]);
	m_AttackVolumes[VOLUME_HACKDOWN]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_RISE]; // Main Attack Volume 설정.
	m_pMainAttackVolume->TriggerActivate(false); // 꺼놓기.
}

CAugusta* CAugusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugusta* pInstance = new CAugusta(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAugusta::Clone(void* pArg)
{
    CAugusta* pInstance = new CAugusta(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAugusta::Free()
{
    CCharacter::Free();
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}

	m_AttackVolumes.clear();

    Safe_Release(m_pBayonet);
    Safe_Release(m_pSkillWeapon);
    Safe_Release(m_pGriffon);
	Safe_Release(m_pWing);
}
