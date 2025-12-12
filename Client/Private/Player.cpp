#include "ClientPch.h"
#include "Player.h"
#include "Character.h"
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

    m_iCurrentCharacterIdx = ROVER; // 방랑자로 테스트
    //m_iCurrentCharacterIdx = GALBRENA; // 갈브레나로 테스트

	m_pPlayerStatus = m_pGameSystem->Get_PlayerStatus();
	Safe_AddRef(m_pPlayerStatus);

	// 6. Ability 참조 제공
	for (_uint i = CHARACTERTYPE::ROVER; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (nullptr != m_Characters[i])
			m_Characters[i]->Set_Ability(m_pPlayerStatus->Get_Ability(i));
	}


	// 7. 기본 상태 FLIGHT
	m_eUtilityType = UI_TAB_UTILITY::FLIGHT;
	
	// 8. 기본 상태 모두 적용하기.
	for (_uint i = CHARACTERTYPE::ROVER; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (nullptr != m_Characters[i])
			m_Characters[i]->Sync_UtilityType_FromPlayer(m_eUtilityType);
	}

	// 9. 타이머 지정.
	m_fChangeCoolTime = 3.f;
	

    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
	
    m_pInputControllerCom->Update();

	// . PlayerStatus 갱신
	m_pPlayerStatus->Set_CurrentCharIndex(m_iCurrentCharacterIdx);

	// 2. 현재 활성화 캐릭터 이후에 키 입력 확인하기
	Player_KeyInput();

	// 3. 변경이 있다면, 이 프레임 끝에서 처리
	if (m_IsChanage)
	{
		m_IsChanage = false;
		Change_Character(m_eNextCharacter, fTimeDelta);
	}
   
	// 4. 현재 캐릭터에 대한 초기 업데이트
	if (m_iCurrentCharacterIdx != NONE)
		m_Characters[m_iCurrentCharacterIdx]->Priority_Update(fTimeDelta);

	
	// 5. Harmony
	if (m_iHarmonyCharacterIdx != NONE &&
		m_iHarmonyCharacterIdx != m_iCurrentCharacterIdx)
		m_Characters[m_iHarmonyCharacterIdx]->Priority_Update(fTimeDelta);
	else if (m_iPrevCharacterIdx != NONE && m_Characters[m_iPrevCharacterIdx]->IsActivate())
		m_Characters[m_iPrevCharacterIdx]->Priority_Update(fTimeDelta);



	// 6. 현재 비활성화되었든, 활성화되었든 업데이트는 플레이어에서 모두 실행 Update
	if (nullptr != m_pPlayerStatus)
		m_pPlayerStatus->Update(fTimeDelta);


	// 7. Cool Time 갱신
	for (_int i = 0; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (m_ChangeTimers[i] > 0.f)
			m_ChangeTimers[i] -= fTimeDelta;
	}
	
	// 8. PlayerStatus에 Utility Type 바인딩.
	Sync_UtilityType();

	// 9. Previous Position
	m_pTransformCom->Save_PreviousPosition();
}

void CPlayer::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
	if (m_iCurrentCharacterIdx != NONE)
	{
		Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]); // 변경 후에도 동기화 유지.
		Sync_Condition_FromCharacter(m_Characters[m_iCurrentCharacterIdx]); // 컨디션 동기화
		m_Characters[m_iCurrentCharacterIdx]->Update(fTimeDelta);
	}

    // 2. Harmony 
    if (m_iHarmonyCharacterIdx != NONE &&
		m_iHarmonyCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iHarmonyCharacterIdx]->Update(fTimeDelta);
	else if (m_iPrevCharacterIdx != NONE && m_Characters[m_iPrevCharacterIdx]->IsActivate())
		m_Characters[m_iPrevCharacterIdx]->Update(fTimeDelta);


	// 3. Rigidbody Update => Camera 
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
	m_pGrappleRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	Sorting_GrappleTarget(); // Grapple Target Sorting;
	Toggle_Grapple(); 
	Sorting_ThrowTarget();
	Toggle_Throw();
	Sorting_Target(); // 4. Target Sorting
    Toggle_LockOn(); // 5. Lock On
	
	m_GrappleCandidates.clear();
	m_TargetCandidates.clear();
	m_ThrowCandidates.clear();
	//m_TargetTransforms.clear();

	

#ifdef _DEBUG
	GUI_Teleport();
#endif
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);

	
		

    if (m_iCurrentCharacterIdx != NONE)
        m_Characters[m_iCurrentCharacterIdx]->Late_Update(fTimeDelta);

	

    if (m_iHarmonyCharacterIdx != NONE &&
		m_iHarmonyCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iHarmonyCharacterIdx]->Late_Update(fTimeDelta);
	else if (m_iPrevCharacterIdx != NONE && m_Characters[m_iPrevCharacterIdx]->IsActivate())
		m_Characters[m_iPrevCharacterIdx]->Late_Update(fTimeDelta);

	// 채널에서 위치 갱신
	m_pGameInstance->Update_Listener(m_pTransformCom, fTimeDelta);
	

#ifdef _DEBUG
	if (FAILED((m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this))))
		return;
#endif // DEBUG

	
}
void CPlayer::Render()
{
    
#ifdef _DEBUG
	//m_pRigidbodyCom->Render();
	m_pGrappleRigidbodyCom->Render();
#endif // _DEBUG

}

void CPlayer::Render_Shadow()
{

}



#pragma endregion

#pragma region UI Interface
// UI Transfer Current Ability Pointer
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

	// 해당 캐릭터의 Harmony Gauge 초기화
	pAbility->Set_HarmonyGauge(0.f);

	// 캐릭터 상태변경.
	m_iHarmonyCharacterIdx = m_iPrevCharacterIdx;

	// CallBack 제어
	m_Characters[m_iHarmonyCharacterIdx]->Set_HarmonyEndCallback([this, eCharacterType]() {
		this->On_HarmonyEnd(eCharacterType);
		});

	// 이전 캐릭터한테 QTE 정보 알림. => 별개의 Transform으로 움직여야함.
	// Collider도 제어되면안됨.
	m_Characters[m_iHarmonyCharacterIdx]->Activate(true);
	m_Characters[m_iHarmonyCharacterIdx]->Bind_QTE(true);
	m_Characters[m_iHarmonyCharacterIdx]->Set_QTEEnd(false);

	// 그 뭐냐 UI에 캐릭 변경 불가능 상태를 줘야함
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
	//{
	//	_float4x4 identityMatrix = {}; XMStoreFloat4x4(&identityMatrix, (XMMatrixIdentity()));
	//	return identityMatrix;
	//}

	return m_pTransformCom->Get_WorldMatrixPtr();
}

#pragma endregion

void CPlayer::Player_KeyInput()
{
	// Scan 키 설정. => T키로 변경 예정.
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::T), KEYSTATE::UP) &&
		UI_TAB_UTILITY::SENSOR == m_eUtilityType) //
	{
		_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);

		_matrix WorldPosMatrix = XMMatrixTranslationFromVector(vPosition);

		m_pGameInstance->Play_Sound(TEXT("ae_ui_but_scan_v3 (SFX)"), ENUM_CLASS(CHANNEL::EFFECT), 1.f);
		m_pGameInstance->Spawn_PoolingObject_ForStatic(TEXT("Pooling_GameObject_Scan"), WorldPosMatrix, nullptr);
	}

	if (!m_IsQTE && // QTE 도중이면 플레이어 변경 불가능.
		!m_IsEventLock) // ANIMSTOP 도중이면 플레이어 변경 불가능.
	{
		if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D1)))
		{
			if (m_iCurrentCharacterIdx != CHARACTERTYPE::ROVER && m_ChangeTimers[CHARACTERTYPE::ROVER] <= 0.f)
			{
				m_IsChanage = true;
				m_eNextCharacter = CHARACTERTYPE::ROVER;
				//m_pPlayerStatus->Set_CurrentCharIndex(CHARACTERTYPE::ROVER);
				m_ChangeTimers[CHARACTERTYPE::ROVER] = m_fChangeCoolTime;
				return;
			}

		}
		else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2)))
		{
			if (m_iCurrentCharacterIdx != CHARACTERTYPE::AUGUSTA && m_ChangeTimers[CHARACTERTYPE::AUGUSTA] <= 0.f)
			{
				m_IsChanage = true;
				m_eNextCharacter = CHARACTERTYPE::AUGUSTA;
				//m_pPlayerStatus->Set_CurrentCharIndex(CHARACTERTYPE::AUGUSTA);
				m_ChangeTimers[CHARACTERTYPE::AUGUSTA] = m_fChangeCoolTime;
				return;
			}
		}
		else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3)))
		{
			if (m_iCurrentCharacterIdx != CHARACTERTYPE::GALBRENA && m_ChangeTimers[CHARACTERTYPE::GALBRENA] <= 0.f)
			{
				m_IsChanage = true;
				m_eNextCharacter = CHARACTERTYPE::GALBRENA;
				//m_pPlayerStatus->Set_CurrentCharIndex(CHARACTERTYPE::GALBRENA);
				m_ChangeTimers[CHARACTERTYPE::GALBRENA] = m_fChangeCoolTime;
				return;
			}
		}

		// Tab을 뗐을 때: UI를 끄고, 선택된 결과를 받아와서 플레이어 상태를 갱신한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_TAB) == KEYSTATE::DOWN)
		{
			m_pGameSystem->Show_TabUtilityUI(ENUM_CLASS(m_eUtilityType));
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_TAB) == KEYSTATE::UP)
		{
			_uint iSelectedUtility = m_pGameSystem->HideNGet_TabUtilityUI();

			if (iSelectedUtility != ENUM_CLASS(UI_TAB_UTILITY::NOTHING)) // NOTHING은 예시
			{
				m_eUtilityType = static_cast<UI_TAB_UTILITY>(iSelectedUtility);

				// 변경 즉시 현재 모든 캐릭터에게도 적용
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

		m_pSpringCamera->Use_Spring(2.5f, 0.1f);
	}
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D5), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Debug_FullCost(true);
		m_Characters[m_iCurrentCharacterIdx]->Clear_CoolTime();


	}


	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D6), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Print_Cost();
		m_Characters[m_iCurrentCharacterIdx]->Print_CoolTime();

		if (nullptr != m_pTransformCom) // 우선 내위치에 켜기?ㅡ
			m_pGameSystem->Summon_SequenceCharacter(m_pTransformCom);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_7) == KEYSTATE::UP)
	{
		//m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Print_KeySlotinfo();
		m_Characters[m_iCurrentCharacterIdx]->Spawn_MotionTrail(3.f, 0.5f, 1.f, { 1.f, 1.f, 1.f, 1.f });

		m_Characters[m_iCurrentCharacterIdx]->Start_Anim();
		LEVI_GRAB Desc{ true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Levi_Grab"), Desc);
		m_Characters[m_iCurrentCharacterIdx]->Attach_ThrowTarget(true);
		// Notify_Event(CHARACTER_EVENT::LEVIATAN_QTE_SUCCESS);

	}

	if (m_pGameInstance->Get_DIKeyState(DIK_8) == KEYSTATE::UP)
	{
		_vector vPos = XMVectorSet(-18.9f, 0.f, 1083.4f, 1.f);
		Notify_Event(CHARACTER_EVENT::TELEPORT, &vPos);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_9) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Remove_Condition_FromPlayer(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE));
		m_Characters[m_iCurrentCharacterIdx]->Throw_AttachTarget();

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


void CPlayer::Change_Character(CHARACTERTYPE eNextCharacter, _float fTimeDelta)
{
	// 0. 예외 조건 return;
	if (nullptr == m_Characters[eNextCharacter])
		return;

	// 1. 현재 Idx가 None이 아니라면.
	if (m_iCurrentCharacterIdx != NONE)
	{
		// 이전 캐릭터 비활성화
		m_Characters[m_iCurrentCharacterIdx]->Activate(false);
		m_Characters[m_iCurrentCharacterIdx]->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE)); // 혹시 모르니.
		m_Characters[m_iCurrentCharacterIdx]->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::SELECT));
		//m_Characters[m_iCurrentCharacterIdx]->Collider_Active(TEXT("Body"), false); // 끄기.
		m_iPrevCharacterIdx = m_iCurrentCharacterIdx;
	}

	// 2. 새 캐릭터 활성화
	m_iCurrentCharacterIdx = eNextCharacter;
	m_Characters[m_iCurrentCharacterIdx]->Activate(true);
	
	m_Characters[m_iCurrentCharacterIdx]->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::SELECT)); // 선택된걸 확인하기.
	
	// Change Time 부여를 위한 Condition 추가
	m_Characters[m_iCurrentCharacterIdx]->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE));
	m_Characters[m_iCurrentCharacterIdx]->Bind_ChangeTimer();


	// 3. 새 캐릭터의 위치를 Player의 현재 위치로 동기화
	_fmatrix PlayerWorldMatrix = m_pTransformCom->Get_WorldMatrix();
	m_Characters[m_iCurrentCharacterIdx]->Sync_Transform_FromPlayer(PlayerWorldMatrix, m_Characters[m_iPrevCharacterIdx]->Get_Velocity(), fTimeDelta);

	// 4. 새 캐릭터의 콜라이더 초기화 
	m_Characters[m_iCurrentCharacterIdx]->Sync_Collider(XMVectorZero(), fTimeDelta);  // 속도 0으로 초기화

	// 5. 상태 머신 초기화 (IDLE 상태로 자연스럽게 시작)
	m_Characters[m_iCurrentCharacterIdx]->Set_Gravity(true);  // 중력 활성화 (필요 시)
	m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::IDLE);
	
	m_Characters[m_iCurrentCharacterIdx]->Bind_ChangeEffect();

	// 6. 협주 확인. Ensemble
	// 이전 캐릭터의 협주게이지 확인 => Get_HarmonyGauge
	CHARACTERTYPE ePrevCharacterType = static_cast<CHARACTERTYPE>(m_iPrevCharacterIdx);
	CHARACTERTYPE eCurCharacterType = static_cast<CHARACTERTYPE>(m_iCurrentCharacterIdx);


	// 7. QTE 실행. 가능하면
	if (IsQTEPossible(ePrevCharacterType) && !m_IsEventLock)
	{
		// QTE 실행.
		ExecuteQTE(ePrevCharacterType);

		// 둘다 실행?
		m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::QTE);
	}
	else
	{
		m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
	}
	
	
	//m_pPlayerStatus->Set_CurrentCharIndex(eNextCharacter);

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

	// 1. Parry일경우 우선순위 높음
	CALLBACK_CLIENT pClientDesc = *static_cast<CALLBACK_CLIENT*>(pDesc);
	
	// 추후 다른 어택 판정이 들어오면 그거에 맞는 판정을생성합니다.

	// 충돌하면? => Condition 추가 및 데이터 전달 받기.
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
			m_Characters[m_iCurrentCharacterIdx]->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::LANDSLIDE));
		
		//IsStart가 True면 시작지점(SlideData 있음) false면 끝 지점(SlideData 없음)
		
	}
	else if (COLLISIONLAYER::GRAB == eLayer)
	{
		CCharacter::CAPTURE_DESC GrabDesc{};
		GrabDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		GrabDesc.fAttack = pClientDesc.fAttack;
		GrabDesc.iLayer = iLayer;
		GrabDesc.pSocketMatrix = pClientDesc.pSocketMatrix;
		m_Characters[m_iCurrentCharacterIdx]->Grab_Judge(&GrabDesc);
	}
	else if (iLayer == ENUM_CLASS(COLLISIONLAYER::PARRY))
	{
		CCharacter::PARRY_DESC ParryDesc{};
		// 2. Parry 판정
		ParryDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		ParryDesc.fAttack = pClientDesc.fAttack;
		ParryDesc.iLayer = iLayer;
		m_Characters[m_iCurrentCharacterIdx]->Parry_Judge(&ParryDesc);
	}
	else
	{
		CCharacter::HIT_DESC HitDesc{};
		// 3. Hit 판정.
		HitDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		HitDesc.fAttack = pClientDesc.fAttack;
		HitDesc.iLayer = iLayer;
		HitDesc.IsBack = IsHitBack(HitDesc.pTransform);

		m_Characters[m_iCurrentCharacterIdx]->Hit_Judge(&HitDesc);
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
		Bind_EventLock(true);
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
	}
	else if (CHARACTER_EVENT::LEVIATAN_QTE_SUCCESS == eEvent)
	{
		if (m_iCurrentCharacterIdx == CHARACTERTYPE::ROVER)
		{
			Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]);

			LEVI_GRAB Desc{ true };
			m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Levi_Grab"), Desc);
			m_Characters[m_iCurrentCharacterIdx]->Start_Anim();
			Bind_EventLock(false);
		}
	}
	else if (CHARACTER_EVENT::LEVIATAN_GRAB == eEvent)
	{
		if (m_iCurrentCharacterIdx != CHARACTERTYPE::ROVER)
			return;

		// 1, 2, 3번 도 못누르게 막아야함. QTE도 안되게 하기.?
		//m_Characters[m_iCurrentCharacterIdx]->Stop_Anim(); // Animation Stop
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
    /*sort(m_TargetCandidates.begin(), m_TargetCandidates.end(), [this](const TARGET_INFO& pSrc, const TARGET_INFO& pDst)->_bool {
        _float fSrcDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pSrc.pTransform->Get_State(STATE::POSITION)));
        _float fDstDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pDst.pTransform->Get_State(STATE::POSITION)));
        return fSrcDistance < fDstDistance;
        });*/

	sort(m_TargetCandidates.begin(), m_TargetCandidates.end(), [this](const TARGET_INFO& pSrc, const TARGET_INFO& pDst)->_bool {
		_float fSrcDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - pSrc.pTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - pDst.pTransform->Get_State(STATE::POSITION)));
		return fSrcDistance < fDstDistance;
		});

    if (0 < m_TargetCandidates.size())
    {
        //m_pTargetTransform = m_TargetTransforms[0];
		m_TargetInfo = m_TargetCandidates[0];
		m_TargetInfo.IsActive = true;

		// 몬스터가 탐지되었고, 전투 BGM이 진행 중이라면.
		if (m_pGameSystem->IsModinaryBattle())
			m_IsBattle = true;
    }
	else
	{
		// 몬스터가 탐지되어 있지 않은데 전투 상태라면?
		if (m_IsBattle)
		{
			m_pGameSystem->Engage_Battle(false);
			m_IsBattle = false;
		}
			
	}

}

void CPlayer::Toggle_LockOn()
{

	// 1. 락온 키 입력 (상태 전환)
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::WB), KEYSTATE::DOWN))
	{
		m_IsLockOn = !m_IsLockOn;

		if (m_IsLockOn)
		{
			// 현재 타겟을 고정 락온 타겟으로 설정.
			//m_pLockOnTargetTransform = m_pTargetTransform;
			m_LockOnTargetInfo = m_TargetInfo;
		}
		else
		{
			//m_pLockOnTargetTransform = nullptr;
			m_LockOnTargetInfo.Reset();
		}
	}

	// 2. 매프레임 검증
	if (m_IsLockOn)
	{
		// TargetTransform이 없거나.. LockOnTargetTransform이 현재 검색된 Transform 중에 없다면?
		if (nullptr == m_TargetInfo.pTransform || !Is_TargetValid(m_LockOnTargetInfo.pTransform))
		{
			m_IsLockOn = false;
			m_LockOnTargetInfo.pTransform = nullptr;
		}
	}

	// 3. 캐릭터와 카메라에 최종 타겟 정보 전송.
	CTransform* pFinalTarget = nullptr;
	CCharacter* pCurrentCharacter = (m_iCurrentCharacterIdx != NONE) ?
		m_Characters[m_iCurrentCharacterIdx] : nullptr;


	// 4. 락온 해제.
	if (nullptr != m_Characters[m_iCurrentCharacterIdx])
	{
		if (m_Characters[m_iCurrentCharacterIdx]->Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::CUTSCENE)))
		{
			// 락온을 해제해라.
			m_IsLockOn = false;
			m_LockOnTargetInfo.pTransform = nullptr;
		}
	}

	// 5. 락온상태라면?
	if (m_IsLockOn)
	{
		// 하드 락온
		pFinalTarget = m_LockOnTargetInfo.pTransform;
		if (pCurrentCharacter)
			pCurrentCharacter->Set_LockOn(pFinalTarget, m_IsLockOn);

		// LockOn UI 설정.
		Calc_LockOnPos();
		_float3 vPos = {};
		XMStoreFloat3(&vPos, m_pTransformCom->Get_State(STATE::POSITION));
		m_pGameSystem->Attach_LockOnUI(&m_vLockOnPos);

	
	}
	else
	{
		m_pGameSystem->Detach_LockOnUI();
		// 소프트 락온.
		pFinalTarget = m_TargetInfo.pTransform;
		if (pCurrentCharacter)
			pCurrentCharacter->Set_AutoLockOn(pFinalTarget, m_IsLockOn); // Character의 Set_AutoLockOn 호출

	}


	// 6. 카메라 업데이트.
	m_pSpringCamera->Lock_On(pFinalTarget, m_TargetInfo.pSocketMatrix, m_IsLockOn);

	// 7. Target 정보 초기화.
	m_TargetInfo.Reset();
}



void CPlayer::Sorting_GrappleTarget()
{
	// 거리순으로 정렬해서 넣어줍니다.


	sort(m_GrappleCandidates.begin(), m_GrappleCandidates.end(), [this](const GRAPPLE_INFO& src, const GRAPPLE_INFO& dst)->_bool {
		CTransform* pSrcTransform = static_cast<CTransform*>(src.pTransform);
		CTransform* pDstTransform = static_cast<CTransform*>(dst.pTransform);

		/*_float fSrcDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos())
			- pSrcTransform->Get_State(STATE::POSITION)));
		_float fDstDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos())
			- pDstTransform->Get_State(STATE::POSITION)));*/
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
	// 1. 현재 T에 들어가 있는 키가 Grapple 이라면?
	if (m_eUtilityType == UI_TAB_UTILITY::GRAPPLE)
		m_Characters[m_iCurrentCharacterIdx]->Bind_GrappleTarget(
			m_TargetGrappleInfo
		);
}
void CPlayer::Sorting_ThrowTarget()
{
	// 거리순으로 정렬해서 넣어줍니다.
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
		// 캐스팅 타입이 안맞아서 터질 수 있으므로 정확한 Rule을 지켜서 Desc을 설정해야함.
		
		// Vector 컨테이너에 넣어줄 거면 
		m_TargetCandidates.push_back({ pTargetTransform, pcallDesc->pSocketMatrix });
		//m_TargetTransforms.push_back(pTargetTransform);

		//m_pTargetTransform = nullptr;
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

	//_vector vSocektPos, vSocketRot, vSocketScale;
	//XMMatrixDecompose(&vSocektPos, &vSocketRot, &vSocketScale, matSocket);
	//
	//// Scale 1.f로 고정
	//matSocket = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vSocketRot, vSocektPos);

	_matrix matCombined = matSocket * matTarget;

	XMStoreFloat3(&m_vLockOnPos, matCombined.r[3]); // Position 저장.
	
	
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

	// Collider 추가했고.
	m_vColliderOffSet = { 0.f, 0.67f, 0.f };
	m_fColliderRadius = 0.4f;
	m_fColliderHeight = 0.5f;
	
	// Collider를 Player가 소유하고 Character들은 AddRef로 참조
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


	// 몬스터 탐지용 콜백으로 받을 Desc - LJH => 탐지는 하나의 Transform만 설정.
	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = 700.f;
	m_CallBack.pCondition = &m_iCondition;
	m_pColliderCom->Set_Desc(&m_CallBack);

	//m_pColliderCom->Set_Desc(m_pTransformCom);

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
