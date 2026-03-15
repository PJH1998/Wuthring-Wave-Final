#include "ClientPch.h"
#include "Player.h"
#include "Augusta.h"
#include "AugustaState_Enum.h"
#include "Rover.h"
#include "RoverState_Enum.h"
#include "SpringCamera.h"
#include "PlayerFactory.h"
#include "Ability.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Event_Leviatan.h"


#pragma region 
CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
    , m_pGameSystem{ CGameSystem::GetInstance() }
{
    Safe_AddRef(m_pGameSystem);
}


CPlayer::CPlayer(const CPlayer& Prototype)
    : CGameObject(Prototype),
    m_pGameSystem { CGameSystem::GetInstance()}
{
    Safe_AddRef(m_pGameSystem);
}

HRESULT CPlayer::Initialize_Prototype()
{
    if (FAILED(CGameObject::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayer::Initialize_Clone(void* pArg)
{
    PLAYER_DESC* pDesc = static_cast<PLAYER_DESC*>(pArg);

    m_eCurLevel = pDesc->eCurLevel;

	// 0. 플레이어 GameSystem에 등록
	m_pGameSystem->Register_Player(this);
	
    if (FAILED(CGameObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Players(pDesc)))
        return E_FAIL;

	

    CPlayerFactory::Register_Camera(LEVEL::STATIC, m_eCurLevel, this, m_pGameInstance, &m_pSpringCamera);
    CPlayerFactory::Register_KeyInputs(m_pInputControllerCom, this);

	m_pGameInstance->SetUp_CameraNF();

    for (auto& pCharacter : m_Characters)
    {
        if (nullptr != pCharacter)
        {
            pCharacter->Set_InputController(m_pInputControllerCom);
            pCharacter->Set_SpringCamera(m_pSpringCamera);
			pCharacter->Set_Collider(m_pColliderCom, m_vColliderOffSet, m_fColliderHeight, m_fColliderRadius);
			pCharacter->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::IDLE);
        }
    }

    // 5. Transform
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

    m_iCurrentCharacterIdx = ROVER; 

	m_pPlayerStatus = m_pGameSystem->Get_PlayerStatus();
	Safe_AddRef(m_pPlayerStatus);

	for (_uint i = CHARACTERTYPE::ROVER; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (nullptr != m_Characters[i])
			m_Characters[i]->Set_Ability(m_pPlayerStatus->Get_Ability(i));
	}


	m_eUtilityType = UI_TAB_UTILITY::FLIGHT;
	
	for (_uint i = CHARACTERTYPE::ROVER; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (nullptr != m_Characters[i])
			m_Characters[i]->Sync_UtilityType_FromPlayer(m_eUtilityType);
	}

	m_fChangeCoolTime = 3.f;

	


    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);

	ApplySwitchRequest(fTimeDelta);
	PreUpdate_Input(fTimeDelta);
	UpdatePlayerStatusIndex();
	Handle_Input();
	PreUpdate_Characters(fTimeDelta);
	PreUpdate_PlayerStatus(fTimeDelta);
	PreUpdate_SwitchCoolDowns(fTimeDelta);
	Sync_UtilityType();
	Save_PreviousPosition();
}

void CPlayer::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
	UpdateCharacters(fTimeDelta);
	UpdateRigidbodies(fTimeDelta);
	Update_Targeting(fTimeDelta);

#ifdef _DEBUG
	GUI_Teleport();
#endif
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);

	if (IsValidCharacterIndex(m_iCurrentCharacterIdx))
		m_Characters[m_iCurrentCharacterIdx]->Late_Update(fTimeDelta);

	CHARACTERTYPE eExtra = GetExtraCharacterForUpdate();
	if (eExtra != NONE)
		m_Characters[eExtra]->Late_Update(fTimeDelta);
	
}
void CPlayer::Render()
{
    
#ifdef _DEBUG
	//m_pRigidbodyCom->Render();
	//m_pGrappleRigidbodyCom->Render();
#endif // _DEBUG

}

void CPlayer::Render_Shadow()
{

}



#pragma endregion

#pragma region UI Interface
CAbility* CPlayer::Get_AbilityCom(CHARACTERTYPE eCharacterType)
{
	if (NONE == eCharacterType)
		return nullptr;

	return m_Characters[eCharacterType]->Get_AbilityCom();
}
_bool CPlayer::IsQTEPossible(CHARACTERTYPE eCharacterType)
{
	if (NONE == eCharacterType)
		return false;

	CAbility* pAbility = Get_AbilityCom(eCharacterType);
	if (nullptr == pAbility)
		return false;

	_float fHarmony = pAbility->Get_Harmony();

	return fHarmony >= pAbility->Get_MaxHarmony();
}

void CPlayer::ExecuteQTE(CHARACTERTYPE eCharacterType)
{
	if (NONE == eCharacterType)
		return;

	CAbility* pAbility = Get_AbilityCom(eCharacterType);
	if (nullptr == pAbility)
		return;

	pAbility->Set_HarmonyGauge(0.f);

	m_iHarmonyCharacterIdx = m_iPrevCharacterIdx;

	m_Characters[m_iHarmonyCharacterIdx]->Set_HarmonyEndCallback([this, eCharacterType]() {
		this->On_HarmonyEnd(eCharacterType);
		});

	m_Characters[m_iHarmonyCharacterIdx]->Activate(true);
	m_Characters[m_iHarmonyCharacterIdx]->Bind_QTE(true);
	m_Characters[m_iHarmonyCharacterIdx]->Set_QTEEnd(false);

	m_IsQTE = true;
	m_pPlayerStatus->Bind_QTE(m_IsQTE);
}
_vector CPlayer::Get_LookVector()
{
	if (nullptr == m_pTransformCom)
		return XMVectorZero();
	return m_pTransformCom->Get_State(STATE::LOOK);
}

_vector CPlayer::Get_Position()
{
	if (nullptr == m_pTransformCom)
		return XMVectorZero();
	return m_pTransformCom->Get_State(STATE::POSITION);
}

const _float4x4* CPlayer::Get_PlayerMatrixPtr()
{
	if (nullptr == m_pTransformCom)
		return nullptr;


	return m_pTransformCom->Get_WorldMatrixPtr();
}

#pragma endregion

void CPlayer::Handle_Input()
{
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::T), KEYSTATE::UP) &&
		UI_TAB_UTILITY::SENSOR == m_eUtilityType) //
	{
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);

		_matrix WorldPosMatrix = XMMatrixTranslationFromVector(vPosition);

		m_pGameInstance->Play_Sound(TEXT("ae_ui_but_scan_v3 (SFX)"), ENUM_CLASS(CHANNEL::EFFECT), 1.f);
		m_pGameInstance->Spawn_PoolingObject_ForStatic(TEXT("Pooling_GameObject_Scan"), WorldPosMatrix, nullptr);
	}

	if (!m_IsQTE && 
		!m_IsEventLock)
	{
		for (const auto& val : m_SwitchKeys)
		{
			if (!m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(val.eKey))) continue;

			if (m_iCurrentCharacterIdx == val.eType) return;
			if (m_ChangeTimers[val.eType] > 0.f) return;
			RequestCharacterSwitch(val.eType);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_TAB) == KEYSTATE::DOWN)
			m_pGameSystem->Show_TabUtilityUI(ENUM_CLASS(m_eUtilityType));

		if (m_pGameInstance->Get_DIKeyState(DIK_TAB) == KEYSTATE::UP)
		{
			_uint iSelectedUtility = m_pGameSystem->HideNGet_TabUtilityUI();

			if (iSelectedUtility != ENUM_CLASS(UI_TAB_UTILITY::NOTHING))
			{
				m_eUtilityType = static_cast<UI_TAB_UTILITY>(iSelectedUtility);

				for (_uint i = CHARACTERTYPE::ROVER; i < CHARACTERTYPE::TYPE_END; ++i)
				{
					if (nullptr != m_Characters[i])
						m_Characters[i]->Sync_UtilityType_FromPlayer(m_eUtilityType);
				}
			}
		}
	}

	

#ifdef _DEBUG
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D4), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Debug_FullCost();
		m_Characters[m_iCurrentCharacterIdx]->Clear_CoolTime();

		//m_pSpringCamera->Use_Spring(2.5f, 0.1f);
		//m_pGameInstance->Play_Sound(TEXT("role_slide_loop (SFX)"), ENUM_CLASS(CHANNEL::PLAYER_ACTION), 0.3f);
	}
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D5), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Debug_FullCost(true);
		m_Characters[m_iCurrentCharacterIdx]->Clear_CoolTime();

		m_pGameInstance->Stop_Sound(ENUM_CLASS(CHANNEL::PLAYER_ACTION));
	}


	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D6), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Print_Cost();
		m_Characters[m_iCurrentCharacterIdx]->Print_CoolTime();
		m_Characters[m_iCurrentCharacterIdx]->Clear_CoolTime();

		//if (nullptr != m_pTransformCom) // 우선 내위치에 켜기?ㅡ
		//	m_pGameSystem->Summon_SequenceCharacter(m_pTransformCom);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_7) == KEYSTATE::UP)
	{
		//m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Print_KeySlotinfo();
		//m_Characters[m_iCurrentCharacterIdx]->Spawn_MotionTrail(3.f, 0.5f, 1.f, { 1.f, 1.f, 1.f, 1.f });

		m_Characters[m_iCurrentCharacterIdx]->Set_LeviatanQTE(false);
		m_Characters[m_iCurrentCharacterIdx]->Start_Anim();
		
		//m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::LEVIATAN_QTESUCCESS);
		LEVI_GRAB Desc{ true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Levi_Grab"), Desc);
		m_Characters[m_iCurrentCharacterIdx]->Attach_ThrowTarget(true);
		// Notify_Event(CHARACTER_EVENT::LEVIATAN_QTE_SUCCESS);

		m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 1.f);
		m_pGameSystem->Stop_Action();

	}
	
	if (m_pGameInstance->Get_DIKeyState(DIK_8) == KEYSTATE::UP)
	{
		_float2 vPos = { 500.f, -200.f };
		m_pGameSystem->Play_QTE(vPos, UI_QTE_TYPE::TRIGGER_EXECUTE, UI_QTE_BTN::F);


		//Notify_Event(CHARACTER_EVENT::TELEPORT, &vPos);
	}


	if (m_pGameInstance->Get_DIKeyState(DIK_0) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Add_HarmonyGauge(10.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_MINUS) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Add_HarmonyGauge(10.f);
	}
#endif // _DEBUG
	if (m_pGameInstance->Get_DIKeyState(DIK_9) == KEYSTATE::UP)
	{
		_float2 vPos = { 500.f, -200.f };
		m_pGameSystem->HUD_FadeOut();
	}

	

	
}

void CPlayer::Notify_HarmonyEnd()
{
    if (m_iHarmonyCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iHarmonyCharacterIdx]->Activate(false);
		m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
    }
}


// Callback
void CPlayer::On_HarmonyEnd(CHARACTERTYPE eCharacter)
{
    if (m_iHarmonyCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iHarmonyCharacterIdx]->Activate(false);
        m_Characters[m_iHarmonyCharacterIdx]->Clear_HarmonyEndCallback();
		m_Characters[m_iHarmonyCharacterIdx]->Bind_QTE(false); // 시점이 잘못됌. => PriorityUpdate에서 처리해주던가? => 적어도 OnExit에서는 처리하면 안됌

		m_IsQTE = false; 
		m_pPlayerStatus->Bind_QTE(m_IsQTE); // 캐릭 변경 가능.

		m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
    }
}


void CPlayer::Change_Character(CHARACTERTYPE eNext, _float fTimeDelta)
{
	// 0. 예외 조건 return;
	if (!IsValidCharacter(eNext))
		return;

	const CHARACTERTYPE ePrev = static_cast<CHARACTERTYPE>(m_iCurrentCharacterIdx);

	DeactivatePrevCharacter(ePrev);
	ActivateNextCharacter(eNext);
	SyncNextCharacterFromPlayer(ePrev, eNext, fTimeDelta);
	ResetNextCharacterCollider(eNext, fTimeDelta);
	InitNextCharacterState(eNext);
	Bind_SwitchVFX(eNext);
	HandleQTEOnSwitch(ePrev, eNext);
	PlaySwitchSFX();
}

void CPlayer::Sync_Transform_FromCharacter(CCharacter* pCharacter)
{
	if (nullptr == pCharacter)
		return;
	pCharacter->Sync_Transform_ToPlayer(m_pTransformCom); // 현재 캐릭터의 Transform을 Player와 동기화
}

void CPlayer::Sync_Condition_FromCharacter(CCharacter* pCharacter)
{
	if (nullptr == pCharacter)
		return;

	pCharacter->Sync_Condition_ToPlayer(&m_iCondition);
}

// 플레이어 전체 공통이므로 전달.
void CPlayer::Sync_InteractionType_ToCharacter(CCharacter* pCharacter)
{
	if (nullptr == pCharacter)
		return;

	pCharacter->Sync_UtilityType_FromPlayer(m_eUtilityType);
}




// During 사이에 탐지하기.
void CPlayer::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 둘다 아닌 경우에만.
	if ((ENUM_CLASS(COLLISIONLAYER::ENEMY) != iLayer))
		return;

	// Detect Body 탐지용 => switch
	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(iLayer);
	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	switch (eLayer)
	{
	case COLLISIONLAYER::ENEMY:
		Process_CollideEnemy(pcallDesc);
		break;
	}
}

// Grapple 전용 .
void CPlayer::OnCollider_GrappleDuring(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if ((ENUM_CLASS(COLLISIONLAYER::GRAPPLE) != iLayer) &&
		(ENUM_CLASS(COLLISIONLAYER::INTERACT_THROW) != iLayer))
		return;

	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(iLayer);
	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	switch (eLayer)
	{
	case COLLISIONLAYER::GRAPPLE:
		Process_CollideGrapple(pcallDesc);
		break;
	case COLLISIONLAYER::INTERACT_THROW:
		Process_CollideThrow(pcallDesc);
		break;
	}
	
}


void CPlayer::OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 공격과 스킬이 아니라면 호출하지 않습니다.
	if (ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::ENEMY_HARDATTACK) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::GRAB) != iLayer &&
		ENUM_CLASS(COLLISIONLAYER::PARRY) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::SLIDE) != iLayer)
		return;

	if (nullptr == m_Characters[m_iCurrentCharacterIdx])
		return;


	COLLISIONLAYER eLayer = static_cast<COLLISIONLAYER>(iLayer);

	CALLBACK_CLIENT pClientDesc = *static_cast<CALLBACK_CLIENT*>(pDesc);
	if (COLLISIONLAYER::SLIDE == eLayer)
	{
		if (pClientDesc.IsStart)
		{
			// 1. Condition 추가.
			m_Characters[m_iCurrentCharacterIdx]->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE_READY));

			// 2. Data 전달.
			m_Characters[m_iCurrentCharacterIdx]->Reserve_LandSlide(pClientDesc.eSlideData);
		}
		else
			m_Characters[m_iCurrentCharacterIdx]->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE));
	}
	else if (COLLISIONLAYER::GRAB == eLayer)
	{
		CAPTURE_DESC GrabDesc{};
		GrabDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		GrabDesc.fAttack = pClientDesc.fAttack;
		GrabDesc.iLayer = iLayer;
		GrabDesc.pSocketMatrix = pClientDesc.pSocketMatrix;
		m_Characters[m_iCurrentCharacterIdx]->Grab_Judge(GrabDesc);
	}
	else
	{
		HIT_DESC HitDesc{};
		HitDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		HitDesc.fAttack = pClientDesc.fAttack;
		HitDesc.iLayer = iLayer;
		HitDesc.IsBack = IsHitBack(HitDesc.pTransform);
		m_Characters[m_iCurrentCharacterIdx]->Hit_Judge(HitDesc);
	}
}


_bool CPlayer::Is_TargetValid(CTransform* pTarget)
{
	if (nullptr == pTarget)
		return false;


	auto iter = find_if(m_TargetCandidates.begin(), m_TargetCandidates.end(), [pTarget](const TARGET_INFO& info) {
			return info.pTransform == pTarget;
		});

	if (iter == m_TargetCandidates.end())
		return false;

	const _float fMaxLockOnDistance = 30.f;
	_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = pTarget->Get_State(STATE::POSITION);
	_float fDistance = XMVectorGetX(XMVector3Length(vMyPos - vTargetPos));

	// 타겟이 너무 멀어짐
	if (fDistance > fMaxLockOnDistance)
		return false; 

	return true;
}



#pragma region GameSystem 연계함수.
void CPlayer::Notify_GrabVisible(_bool IsVisible)
{
	m_Characters[m_iCurrentCharacterIdx]->Set_Visible(IsVisible);
}
void CPlayer::Notify_EscapeGrabReady()
{
	m_Characters[m_iCurrentCharacterIdx]->Bind_GrabEscapePossible();
	m_IsLockOn = true;
}
void CPlayer::Notify_EscapeGrabExecute()
{
	m_Characters[m_iCurrentCharacterIdx]->Bind_GrabEscapeExecute();
	m_IsLockOn = false;
}

void CPlayer::Notify_Event(CHARACTER_EVENT eEvent, void* pArg)
{
	// 1. 어떤 캐릭터 였건 Rover로 변경하기.
	if (CHARACTER_EVENT::LEVIATAN_QTE == eEvent)
	{
		// LockOn 해제.
		m_IsLockOn = false;

		Bind_EventLock(true);

		// 협주 중이였다면?
		if (m_iHarmonyCharacterIdx != CHARACTERTYPE::NONE)
		{
			// => 협주 중지
			m_Characters[m_iHarmonyCharacterIdx]->Set_QTEEnd(true);
			// => 협주 인덱스를 제거하기.
			m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
		}
			

		m_IsQTE = false;

		// 2. Rover로 변경.
		if (m_iCurrentCharacterIdx != CHARACTERTYPE::ROVER)
			Change_Character(CHARACTERTYPE::ROVER, 0.f);


		// 3. 작업
		// => Rover 위치 변경 (위치는 안변경되는거 같기도하고..)
		// => Rover State 변경. (Leviatan 전용 QTE로)
		// => Rover 시간 멈춤 (State Machine만)
		m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(
			CHARACTER_TRANSITIONTYPE::LEVIATAN_QTE, pArg
		);
		
		// 4. Levi Cap
		m_Characters[m_iCurrentCharacterIdx]->Play_Action(TEXT("Action_Levi_Capture")
			,true, false);
	}
	else if (CHARACTER_EVENT::LEVIATAN_QTE_SUCCESS == eEvent)
	{
		m_IsLockOn = false;
		if (m_iCurrentCharacterIdx == CHARACTERTYPE::ROVER)
		{
			Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]);

			LEVI_GRAB Desc{ true };
			m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Levi_Grab"), Desc);
			m_Characters[m_iCurrentCharacterIdx]->Start_Anim();
			Bind_EventLock(false);
			m_pGameSystem->Stop_Action();
		}
	}
	else if (CHARACTER_EVENT::LEVIATAN_PREV_EXECUTE == eEvent) // 레비아탄 위치도 고정시켜야할 것 같은데?..
	{
		m_IsLockOn = false;

		if (m_iHarmonyCharacterIdx != CHARACTERTYPE::NONE)
		{
			m_Characters[m_iHarmonyCharacterIdx]->Set_QTEEnd(true);
			m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
		}
		

		// 1. Rover가 아니면 Rover로변경 (얘가 메인 캐릭터로)
		if (m_iCurrentCharacterIdx != CHARACTERTYPE::ROVER)
			Change_Character(CHARACTERTYPE::ROVER, 0.f);

		// 2. Rover의 상태를 변경.
		m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(
			CHARACTER_TRANSITIONTYPE::LEVIATAN_PREV_EXECUTE, pArg
		);


		// 3. Galbrena 활성화. => 보스의
		m_iEventCharacterIdx = CHARACTERTYPE::GALBRENA;

		m_Characters[m_iEventCharacterIdx]->TransitionState_FromPlayer(
			CHARACTER_TRANSITIONTYPE::LEVIATAN_PREV_EXECUTE, pArg
		);

		
		m_Characters[m_iCurrentCharacterIdx]->Play_Action(TEXT("Action_Levi_Execute"), true, false);

		// 카메라 액션 시작하면서 실행?
		//m_Characters[m_iCurrentCharacterIdx]->Spawn_Effect(TEXT("Pooling_Excute_Prefab"));
	}
	else if (CHARACTER_EVENT::LEVIATAN_EXECUTE_SUCCESS == eEvent)
	{
		m_IsLockOn = false;

		m_Characters[m_iCurrentCharacterIdx]->Start_Anim();
		m_Characters[m_iEventCharacterIdx]->Start_Anim();
		
		m_pGameSystem->Stop_Action();

		// 이벤트 예약? => 1.5f 뒤에 Leviatan 죽음 이벤트를 실행하라.
		m_Event = [this]() {
			m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 1.f); // 몬스터 TimeRatio 정상화.
			LEVI_EXECUTE Desc{ true };
			m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Levi_Execute"), Desc);
			m_iEventCharacterIdx = CHARACTERTYPE::NONE; // Event 캐릭 해제.
		}; 

		m_fEventMaxTime = { 2.5f };

		m_IsEvent = true;
		
	}
	else if (CHARACTER_EVENT::TELEPORT == eEvent)
	{
		// TELEPORT
		if (nullptr == pArg)
			return;

		_float4 vPos = *static_cast<_float4*>(pArg);
		m_Characters[m_iCurrentCharacterIdx]->Execute_Telport(XMLoadFloat4(&vPos));
	}
	
}
void CPlayer::Bind_EventLock(_bool IsLock)
{
	m_IsEventLock = IsLock;
}

void CPlayer::Lock_Input(_bool IsLock)
{
	if (nullptr == m_pInputControllerCom)
		return;

	m_pInputControllerCom->Set_BlockInput(IsLock);
}
void CPlayer::Bind_Gravity(_bool IsGravity)
{
	if (nullptr == m_pColliderCom)
		return;

	m_pColliderCom->Set_Gravity(IsGravity);
}
void CPlayer::Use_Spring(_float fDestination, _float fDuration)
{
	if (nullptr == m_pSpringCamera)
		return;

	m_pSpringCamera->Use_Spring(fDestination, fDuration);
}
#pragma endregion



void CPlayer::Sorting_Target()
{
	sort(m_TargetCandidates.begin(), m_TargetCandidates.end(), [this](const TARGET_INFO& pSrc, const TARGET_INFO& pDst)->_bool {
		_float fSrcDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - pSrc.pTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - pDst.pTransform->Get_State(STATE::POSITION)));
		return fSrcDistance < fDstDistance;
		});

    if (0 < m_TargetCandidates.size())
    {
		m_TargetInfo = m_TargetCandidates[0];
		m_TargetInfo.IsActive = true;

		if (m_pGameSystem->IsModinaryBattle())
			m_IsBattle = true;
    }
	else
	{
		if (m_IsBattle)
		{
			m_pGameSystem->Engage_Battle(false);
			m_IsBattle = false;
		}
			
	}

}

void CPlayer::Toggle_LockOn()
{

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::WB), KEYSTATE::DOWN))
	{
		m_IsLockOn = !m_IsLockOn;

		if (m_IsLockOn)
		{
			m_LockOnTargetInfo = m_TargetInfo;
		}
		else
		{
			m_LockOnTargetInfo.Reset();
		}
	}

	if (m_IsLockOn)
	{
		if (nullptr == m_TargetInfo.pTransform || !Is_TargetValid(m_LockOnTargetInfo.pTransform))
		{
			m_IsLockOn = false;
			m_LockOnTargetInfo.pTransform = nullptr;
		}
	}

	CTransform* pFinalTarget = nullptr;
	CCharacter* pCurrentCharacter = (m_iCurrentCharacterIdx != NONE) ?
		m_Characters[m_iCurrentCharacterIdx] : nullptr;


	if (nullptr != m_Characters[m_iCurrentCharacterIdx])
	{
		if (m_Characters[m_iCurrentCharacterIdx]->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE)))
		{
			m_IsLockOn = false;
			m_LockOnTargetInfo.pTransform = nullptr;
		}
	}

	if (m_IsLockOn)
	{
		pFinalTarget = m_LockOnTargetInfo.pTransform;
		if (pCurrentCharacter)
			pCurrentCharacter->Set_LockOn(pFinalTarget, m_IsLockOn);

		Calc_LockOnPos();
		_float3 vPos = {};
		XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
		m_pGameSystem->Attach_LockOnUI(&m_vLockOnPos);

	
	}
	else
	{
		m_pGameSystem->Detach_LockOnUI();
		pFinalTarget = m_TargetInfo.pTransform;
		if (pCurrentCharacter)
			pCurrentCharacter->Set_AutoLockOn(pFinalTarget, m_IsLockOn); // Character의 Set_AutoLockOn 호출

	}


	m_pSpringCamera->Lock_On(pFinalTarget, m_TargetInfo.pSocketMatrix, m_IsLockOn);

	m_TargetInfo.Reset();
}



void CPlayer::Sorting_GrappleTarget()
{
	sort(m_GrappleCandidates.begin(), m_GrappleCandidates.end(), [this](const GRAPPLE_INFO& src, const GRAPPLE_INFO& dst)->_bool {
		CTransform* pSrcTransform = static_cast<CTransform*>(src.pTransform);
		CTransform* pDstTransform = static_cast<CTransform*>(dst.pTransform);

		_float fSrcDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION)
			- pSrcTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION)
			- pDstTransform->Get_State(STATE::POSITION)));
		return fSrcDistance < fDstDistance;
		});

	if (0 < m_GrappleCandidates.size())
	{
		m_TargetGrappleInfo = m_GrappleCandidates[0];
		m_TargetGrappleInfo.IsActive = true;
	}
		
}


void CPlayer::Toggle_Grapple()
{
	if (m_eUtilityType == UI_TAB_UTILITY::GRAPPLE)
		m_Characters[m_iCurrentCharacterIdx]->Bind_GrappleTarget(
			m_TargetGrappleInfo
		);
}

void CPlayer::Sorting_ThrowTarget()
{
	sort(m_ThrowCandidates.begin(), m_ThrowCandidates.end(), [this](const THROW_INFO& src, const THROW_INFO& dst)->_bool {
		CTransform* pSrcTransform = static_cast<CTransform*>(src.pTransform);
		CTransform* pDstTransform = static_cast<CTransform*>(dst.pTransform);

		_float fSrcDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION)
			- pSrcTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION)
			- pDstTransform->Get_State(STATE::POSITION)));
		return fSrcDistance < fDstDistance;
		});

	if (0 < m_ThrowCandidates.size())
	{
		m_TargetThrowInfo = m_ThrowCandidates[0];
		m_TargetThrowInfo.IsActive = true;
	}
		
}
void CPlayer::Toggle_Throw()
{
	// 1. 현재 T에 들어가 있는 키가 Grapple 이라면?
	if (m_eUtilityType == UI_TAB_UTILITY::LEVITATOR)
		m_Characters[m_iCurrentCharacterIdx]->Bind_ThrowTarget(
			m_TargetThrowInfo
		);
}
void CPlayer::Process_CollideEnemy(const CALLBACK_CLIENT* pcallDesc)
{
	// CallBack Client Transform에 이상한 값이 들어가 있음.
	CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform);
	if (nullptr == pTargetTransform)
		return;
	{

		lock_guard<mutex> lock(m_Mutex);
		
		m_TargetCandidates.push_back({ pTargetTransform, pcallDesc->pSocketMatrix });

		m_TargetInfo.Reset();
	}
}

void CPlayer::Process_CollideGrapple(const CALLBACK_CLIENT* pcallDesc)
{
	{
		CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform);
		if (nullptr == pTargetTransform)
			return;

		// 매프레임 초기화.
		m_TargetGrappleInfo.Reset();

		lock_guard<mutex> lock(m_Mutex);

		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vTargetPos = pTargetTransform->Get_State(STATE::POSITION);
		_bool IsinWorldSpace = m_pGameInstance->IsIn_WorldSpace(vTargetPos, 0.f);
		_float fLength = XMVectorGetX(XMVector3Length(vPos - vTargetPos));

		
		// Camera View Space 안에 있으면 넣기.
		if (IsinWorldSpace && !(pcallDesc->eObjectType == OBJECTTYPE::ROPE_UI))
			m_GrappleCandidates.push_back({ pTargetTransform, pcallDesc->eObjectType, pcallDesc->pCondition });

		
	}
}

void CPlayer::Process_CollideThrow(const CALLBACK_CLIENT* pcallDesc)
{
	{
		CTransform* pTransform = static_cast<CTransform*>(pcallDesc->pTransform);
		if (nullptr == pTransform)
			return;

		lock_guard<mutex> lock(m_Mutex);

		THROW_INFO ThrowInfo = {
			pTransform,
			pcallDesc->eObjectType,
			pcallDesc->IsGrab,
			pcallDesc->IsThrow,
			pcallDesc->ppRefBoneMatrix,
			pcallDesc->ppRefWorldMatrix
		};

		m_ThrowCandidates.push_back(ThrowInfo);

		// 매프레임 초기화.
		m_TargetThrowInfo.Reset();
	}
}

void CPlayer::Process_QTEEvent(CHARACTER_EVENT eEvent, void* pArg)
{
}

void CPlayer::Process_Timer(_float fTimeDelta)
{
	if (m_IsEvent)
	{
		if (m_fEventTimer <= m_fEventMaxTime)
		{
			m_fEventTimer += fTimeDelta;
		}
		else
		{
			m_fEventTimer = 0.f;
			m_IsEvent = false;

			if (nullptr != m_Event)
				m_Event();
		}
	}

}

void CPlayer::Manage_Condition()
{

}

void CPlayer::Sync_UtilityType()
{
	if (nullptr == m_pPlayerStatus)
		return;

	m_pPlayerStatus->Bind_UtilityType(m_eUtilityType);
}

_bool CPlayer::IsHitBack(CTransform* pTransform)
{
	if (nullptr == pTransform)
		return false;

	_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vAttackerPos = pTransform->Get_State(STATE::POSITION);

	_vector vDirToAttacker = vAttackerPos - vPlayerPos;
	vDirToAttacker = XMVectorSetY(vDirToAttacker, 0.f); // 높이 차이로 인한 오차 제거

	if (XMVectorGetX(XMVector3Length(vDirToAttacker)) < 0.001f) // 거리가 너무적다면. 그냥 false 리턴.
		return false;

	vDirToAttacker = XMVector3Normalize(vDirToAttacker);

	_vector vPlayerLook = m_pTransformCom->Get_State(STATE::LOOK);
	vPlayerLook = XMVectorSetY(vPlayerLook, 0.f);
	vPlayerLook = XMVector3Normalize(vPlayerLook);

	_float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vDirToAttacker));
	if (fDot < 0.f)
		return true;

	return false;
}

void CPlayer::Calc_LockOnPos()
{
	if (nullptr == m_LockOnTargetInfo.pTransform ||
		nullptr == m_LockOnTargetInfo.pSocketMatrix)
		return;

	_matrix matTarget = m_LockOnTargetInfo.pTransform->Get_WorldMatrix();
	_matrix matSocket = XMLoadFloat4x4(m_LockOnTargetInfo.pSocketMatrix);
	_matrix matCombined = matSocket * matTarget;
	XMStoreFloat3(&m_vLockOnPos, matCombined.r[3]); // Position 저장.
	
	
}

void CPlayer::RequestCharacterSwitch(CHARACTERTYPE eType)
{
	m_SwitchRequest = { true, eType };
	m_ChangeTimers[eType] = m_fChangeCoolTime;
}

_bool CPlayer::IsValidCharacter(CHARACTERTYPE eType) const
{
	return IsValidCharacterIndex(ENUM_CLASS(eType));
}

_bool CPlayer::IsValidCharacterIndex(_int iIndex) const
{
	if (iIndex <= NONE || iIndex >= CHARACTERTYPE::TYPE_END) return false;
	if (nullptr == m_Characters[iIndex]) return false;

	return true;
}


void CPlayer::DeactivatePrevCharacter(CPlayer::CHARACTERTYPE ePrev)
{
	if (ePrev == NONE) return;

	auto* pPrev = m_Characters[ePrev];
	pPrev->Activate(false);
	pPrev->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::CHANGE));
	pPrev->Remove_Flag(ENUM_CLASS(CHARACTER_CONDITION::SELECT));
	m_iPrevCharacterIdx = ePrev;
}


void CPlayer::ActivateNextCharacter(CPlayer::CHARACTERTYPE eNext)
{
	m_iCurrentCharacterIdx = eNext;

	auto* pNext = m_Characters[eNext];
	pNext->Activate(true);
	pNext->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::SELECT));
	pNext->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE));
	pNext->Bind_ChangeTimer();
}

void CPlayer::SyncNextCharacterFromPlayer(CPlayer::CHARACTERTYPE ePrev, CPlayer::CHARACTERTYPE eNext, _float fTimeDelta)
{
	_fmatrix PlayerWorldMatrix = m_pTransformCom->Get_WorldMatrix();
	_vector prevVel = XMVectorZero();

	if (IsValidCharacter(ePrev))
		prevVel = m_Characters[ePrev]->Get_Velocity();

	m_Characters[eNext]->Sync_Transform_FromPlayer(PlayerWorldMatrix, prevVel, fTimeDelta);
}

void CPlayer::ResetNextCharacterCollider(CPlayer::CHARACTERTYPE eNext, _float fTimeDelta)
{
	m_Characters[eNext]->Sync_Collider(XMVectorZero(), fTimeDelta);
}

void CPlayer::InitNextCharacterState(CPlayer::CHARACTERTYPE eNext)
{
	auto* pNext = m_Characters[eNext];
	pNext->Set_Gravity(true);
	pNext->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::IDLE);
}

void CPlayer::HandleQTEOnSwitch(CPlayer::CHARACTERTYPE ePrev, CPlayer::CHARACTERTYPE eNext)
{
	if (ePrev == NONE)
	{
		m_iHarmonyCharacterIdx = NONE;
		return;
	}

	if (IsQTEPossible(ePrev) && !m_IsEventLock)
	{
		ExecuteQTE(ePrev);
		m_Characters[eNext]->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::QTE);
	}
	else
		m_iHarmonyCharacterIdx = NONE;
}

void CPlayer::Bind_SwitchVFX(CPlayer::CHARACTERTYPE eNext)
{
	m_Characters[eNext]->Bind_ChangeEffect();
}

void CPlayer::PlaySwitchSFX()
{
	m_pGameInstance->Play_Sound(TEXT("ui_ia_com_tick (SFX)"), ENUM_CLASS(CHANNEL::PLAYER_UI), 0.25f);
}

void CPlayer::UpdateCharacters(_float fTimeDelta)
{
	// 1. 현재 캐릭터
	if (IsValidCharacterIndex(m_iCurrentCharacterIdx))
	{
		Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]);
		Sync_Condition_FromCharacter(m_Characters[m_iCurrentCharacterIdx]);
		m_Characters[m_iCurrentCharacterIdx]->Update(fTimeDelta);
	}

	// 2. 추가 업데이트 대상 결정 (QTE > 이벤트 > 이전 캐릭터(Dissolve 처리 상태))
	CHARACTERTYPE eExtra = GetExtraCharacterForUpdate();

	if (eExtra != CHARACTERTYPE::NONE)
		m_Characters[eExtra]->Update(fTimeDelta);
}

void CPlayer::UpdateRigidbodies(_float fTimeDelta)
{
	const _fmatrix WorldMatrix = m_pTransformCom->Get_WorldMatrix();
	m_pRigidbodyCom->Update_Rigidbody(WorldMatrix, fTimeDelta);
	m_pGrappleRigidbodyCom->Update_Rigidbody(WorldMatrix, fTimeDelta);
}

void CPlayer::Update_Targeting(_float fTimeDelta)
{
	Sorting_GrappleTarget();
	Toggle_Grapple();
	Sorting_ThrowTarget();
	Toggle_Throw();
	Sorting_Target();
	Toggle_LockOn();

	m_GrappleCandidates.clear();
	m_TargetCandidates.clear();
	m_ThrowCandidates.clear();
}

CPlayer::CHARACTERTYPE CPlayer::GetExtraCharacterForUpdate() const
{
	if (IsValidCharacterIndex(m_iHarmonyCharacterIdx) &&
		m_iHarmonyCharacterIdx != m_iCurrentCharacterIdx)
		return static_cast<CHARACTERTYPE>(m_iHarmonyCharacterIdx);

	if (IsValidCharacterIndex(m_iEventCharacterIdx))
		return static_cast<CHARACTERTYPE>(m_iEventCharacterIdx);

	if (IsValidCharacterIndex(m_iPrevCharacterIdx) &&
		m_Characters[m_iPrevCharacterIdx]->IsActivate())
		return static_cast<CHARACTERTYPE>(m_iPrevCharacterIdx);

	return NONE;
}

void CPlayer::PreUpdate_Input(_float fTimeDelta)
{
	Process_Timer(fTimeDelta);
	m_pInputControllerCom->Update();
}

void CPlayer::UpdatePlayerStatusIndex()
{
	if (nullptr != m_pPlayerStatus)
		m_pPlayerStatus->Set_CurrentCharIndex(m_iCurrentCharacterIdx);
}

// 변경 요청을 받았다면 교체
void CPlayer::ApplySwitchRequest(_float fTimeDelta)
{
	if (m_SwitchRequest.isSwitching)
	{
		m_SwitchRequest.isSwitching = false;
		Change_Character(m_SwitchRequest.eType, fTimeDelta);
	}
}

void CPlayer::PreUpdate_Characters(_float fTimeDelta)
{
	if (IsValidCharacterIndex(m_iCurrentCharacterIdx))
		m_Characters[m_iCurrentCharacterIdx]->Priority_Update(fTimeDelta);

	CHARACTERTYPE eExtra = GetExtraCharacterForUpdate();
	if (eExtra != CHARACTERTYPE::NONE)
		m_Characters[eExtra]->Priority_Update(fTimeDelta);
}

void CPlayer::PreUpdate_PlayerStatus(_float fTimeDelta)
{
	if (nullptr != m_pPlayerStatus)
		m_pPlayerStatus->Update(fTimeDelta);
}

void CPlayer::PreUpdate_SwitchCoolDowns(_float fTimeDelta)
{
	for (_int i = 0; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (m_ChangeTimers[i] > 0.f)
			m_ChangeTimers[i] -= fTimeDelta;
	}
}

void CPlayer::Save_PreviousPosition()
{
	if (nullptr == m_pTransformCom)
		return;

	m_pTransformCom->Save_PreviousPosition();
}


#ifdef _DEBUG
void CPlayer::GUI_Teleport()
{
	ImGui::Begin("Player Teleport");

	ImGui::Text("[Position]");
	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&m_vDebugTeleportPos));

	if (ImGui::Button("Apply"))
	{
		_vector vChagePos = XMVectorSetW(XMLoadFloat3(&m_vDebugTeleportPos), 1.f);
		m_pTransformCom->Set_State(STATE::POSITION, vChagePos);
		m_pColliderCom->Set_Position(vChagePos);
	}
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_char szPos[MAX_PATH] = {};
	sprintf_s(szPos, "X : %.2f / Y : %.2f / Z : %.2f", vPos.m128_f32[0], vPos.m128_f32[1], vPos.m128_f32[2]);
	ImGui::Text(szPos);

	ImGui::End();
}
#endif
HRESULT CPlayer::Ready_Players(const PLAYER_DESC* pDesc)
{
    ASSERT_CRASH(pDesc);

    // 1. Players
    m_Characters.resize(CHARACTERTYPE::TYPE_END);
    
    CCharacter::CHARACTER_DESC CharacterDesc;
    CCharacter* pPlayer = { nullptr };

    // 2
    for (_uint i = 0; i < pDesc->iPlayerCount; ++i)
    {
        switch (i)
        {
		case CHARACTERTYPE::ROVER:
			CharacterDesc = pDesc->PlayerSpecs[CHARACTERTYPE::ROVER].CharacterDesc;
			CharacterDesc.pOwner = this;
			pPlayer = dynamic_cast<CCharacter*>(m_pGameInstance->Clone_Prototype(
				ENUM_CLASS(m_eCurLevel),
				pDesc->PlayerSpecs[i].strActorTag,
				PROTOTYPE::GAMEOBJECT,
				&CharacterDesc));

			ASSERT_CRASH(pPlayer);
			m_Characters[i] = pPlayer;
			break;
		case CHARACTERTYPE::AUGUSTA:
		{
			CharacterDesc = pDesc->PlayerSpecs[CHARACTERTYPE::AUGUSTA].CharacterDesc;
			CharacterDesc.pOwner = this;
			pPlayer = dynamic_cast<CCharacter*>(m_pGameInstance->Clone_Prototype(
				ENUM_CLASS(m_eCurLevel),
				pDesc->PlayerSpecs[i].strActorTag,
				PROTOTYPE::GAMEOBJECT,
				&CharacterDesc));

			ASSERT_CRASH(pPlayer);
			m_Characters[i] = pPlayer;

		}
		break;
        case CHARACTERTYPE::GALBRENA:
		{
			CharacterDesc = pDesc->PlayerSpecs[CHARACTERTYPE::GALBRENA].CharacterDesc;
			CharacterDesc.pOwner = this;
			pPlayer = dynamic_cast<CCharacter*>(m_pGameInstance->Clone_Prototype(
				ENUM_CLASS(m_eCurLevel),
				pDesc->PlayerSpecs[i].strActorTag,
				PROTOTYPE::GAMEOBJECT,
				&CharacterDesc));

			ASSERT_CRASH(pPlayer);
			m_Characters[i] = pPlayer;
		}
            break;
	
        default:
            break;
        }
    }

    //m_iCurrentCharacterIdx = CHARACTERTYPE::AUGUSTA;
    m_iCurrentCharacterIdx = CHARACTERTYPE::NONE;

    return S_OK;
}

HRESULT CPlayer::Ready_Components(const PLAYER_DESC* pDesc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->eCurLevel)
        , pDesc->wStrInputControllerTag, TEXT("Com_InputController"), reinterpret_cast<CComponent**>(&m_pInputControllerCom), nullptr)))
        CRASH("Input Controller");


    CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
    RigidbodyDesc.eBodyType = CRigidbody::BODY;
    RigidbodyDesc.eShape = SHAPE::BOX;
    RigidbodyDesc.eType = EMotionType::Kinematic;
    RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
    RigidbodyDesc.vExtent = _float3(30.f, 30.f, 30.f);
    XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

    if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
        TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        CRASH("Rigidbody");

    m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_During(iLayer, pDesc, Manifold);
    });

	RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = _float3(15.f, 15.f, 15.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_GrappleRigidbody"), reinterpret_cast<CComponent**>(&m_pGrappleRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pGrappleRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_GrappleDuring(iLayer, pDesc, Manifold);
	});

	m_vColliderOffSet = { 0.f, 0.67f, 0.f };
	m_fColliderRadius = 0.4f;
	m_fColliderHeight = 0.5f;
	
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.vPos = pDesc->vPosition;
	ColliderDesc.vOffset = m_vColliderOffSet;
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	ColliderDesc.fHeight = m_fColliderHeight;
	ColliderDesc.fRadius = m_fColliderRadius;
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Collider"), TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		CRASH("Collider");


	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = 700.f;
	m_CallBack.pCondition = &m_iCondition;
	m_pColliderCom->Set_Desc(&m_CallBack);

	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_Enter(iLayer, pDesc, Manifold);
		});

    return S_OK;
}

CPlayer* CPlayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPlayer* pInstance = new CPlayer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CPlayer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CPlayer::Clone(void* pArg)
{
    CPlayer* pInstance = new CPlayer(*this);
    
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CPlayer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPlayer::Free()
{
    CGameObject::Free();
    Safe_Release(m_pGameSystem);

    for (auto& pCharacter : m_Characters)
        Safe_Release(pCharacter);

    Safe_Release(m_pSpringCamera);
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pRigidbodyCom);
    Safe_Release(m_pGrappleRigidbodyCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pPlayerStatus);
}
