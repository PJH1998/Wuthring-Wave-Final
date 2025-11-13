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

    //m_iCurrentCharacterIdx = AUGUSTA;
    m_iCurrentCharacterIdx = ROVER; // 방랑자로 테스트

	m_pPlayerStatus = m_pGameSystem->Get_PlayerStatus();
	Safe_AddRef(m_pPlayerStatus);

	// 6. Ability 참조 제공
	for (_uint i = CHARACTERTYPE::ROVER; i < CHARACTERTYPE::TYPE_END; ++i)
	{
		if (nullptr != m_Characters[i])
			m_Characters[i]->Set_Ability(m_pPlayerStatus->Get_Ability(i));
	}

    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
	
    m_pInputControllerCom->Update();
    
	// 2. 현재 활성화 캐릭터 이후에 키 입력 확인하기
	Player_KeyInput();

	// 3. 변경이 있다면, 이 프레임 끝에서 처리
	if (m_IsChanage)
	{
		m_IsChanage = false;
		Change_Character(m_eNextCharacter, fTimeDelta);
	}
   
	if (m_iCurrentCharacterIdx != NONE)
		m_Characters[m_iCurrentCharacterIdx]->Priority_Update(fTimeDelta);

	// 2. Harmony
	if (m_iHarmonyCharacterIdx != NONE &&
		m_iHarmonyCharacterIdx != m_iCurrentCharacterIdx)
		m_Characters[m_iHarmonyCharacterIdx]->Priority_Update(fTimeDelta);


	// 4. 현재 비활성화되었든, 활성화되었든 업데이트는 플레이어에서 모두 실행 Update
	if (nullptr != m_pPlayerStatus)
		m_pPlayerStatus->Update(fTimeDelta);

	// 5. 몬스터 사이와의 거리는 Priority Update에서 계산
	if (nullptr != m_pTargetTransform)
		m_fTargetDistance = 0.f;
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


	// 3. Rigidbody Update => Camera 
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

	// 4. Target Sorting
	Sorting_Target();
    
	// 5. Lock On
    Toggle_LockOn();

	m_TargetTransforms.clear();
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

	
}
void CPlayer::Render()
{
    
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
	m_Characters[m_iHarmonyCharacterIdx]->Bind_QTE(true);
	m_Characters[m_iHarmonyCharacterIdx]->Set_QTEEnd(false);
	
	// 그 뭐냐 UI에 캐릭 변경 불가능 상태를 줘야함
	m_IsQTE = true;
	m_pPlayerStatus->Bind_QTE(m_IsQTE);


}
#pragma endregion

void CPlayer::Player_KeyInput()
{
	// Character Change

	if (!m_IsQTE) // QTE 도중이면 플레이어 변경 불가능.
	{
		if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D1)))
		{
			if (m_iCurrentCharacterIdx != CHARACTERTYPE::ROVER)
			{
				m_IsChanage = true;
				m_eNextCharacter = CHARACTERTYPE::ROVER;
				m_pPlayerStatus->Set_CurrentCharIndex(CHARACTERTYPE::ROVER);
				return;
			}

		}
		else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2)))
		{
			if (m_iCurrentCharacterIdx != CHARACTERTYPE::AUGUSTA)
			{
				m_IsChanage = true;
				m_eNextCharacter = CHARACTERTYPE::AUGUSTA;
				m_pPlayerStatus->Set_CurrentCharIndex(CHARACTERTYPE::AUGUSTA);
				return;
			}
		}
		else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3)))
		{
			if (m_iCurrentCharacterIdx != CHARACTERTYPE::GALBRENA)
			{
				m_IsChanage = true;
				m_eNextCharacter = CHARACTERTYPE::GALBRENA;
				m_pPlayerStatus->Set_CurrentCharIndex(CHARACTERTYPE::GALBRENA);
				return;
			}
		}
	}
		

	


	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D4), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Debug_FullCost();
	}
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D5), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Debug_FullCost(true);
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D6), KEYSTATE::UP))
	{
		m_Characters[m_iCurrentCharacterIdx]->Print_Cost();
		m_Characters[m_iCurrentCharacterIdx]->Print_CoolTime();
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_7) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Print_KeySlotinfo();
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_8) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Add_Hp(-10.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_9) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Add_Hp(10.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_0) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Add_HarmonyGauge(10.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_MINUS) == KEYSTATE::UP)
	{
		m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom()->Add_HarmonyGauge(10.f);
	}

	
}

void CPlayer::Notify_HarmonyEnd()
{
    if (m_iHarmonyCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iHarmonyCharacterIdx]->SetActivate(false);
		m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
    }
}


// Callback
void CPlayer::On_HarmonyEnd(CHARACTERTYPE eCharacter)
{
    if (m_iHarmonyCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iHarmonyCharacterIdx]->SetActivate(false);
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
		m_Characters[m_iCurrentCharacterIdx]->SetActivate(false);
		m_Characters[m_iCurrentCharacterIdx]->Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE)); // 혹시 모르니.
		//m_Characters[m_iCurrentCharacterIdx]->Collider_Active(TEXT("Body"), false); // 끄기.
		m_iPrevCharacterIdx = m_iCurrentCharacterIdx;
	}

	// 2. 새 캐릭터 활성화
	m_iCurrentCharacterIdx = eNextCharacter;
	m_Characters[m_iCurrentCharacterIdx]->SetActivate(true);
	
	// Change Time 부여를 위한 Condition 추가
	m_Characters[m_iCurrentCharacterIdx]->Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::CHANGE));
	m_Characters[m_iCurrentCharacterIdx]->Bind_ChangeTimer();
	
	


	// 3. 새 캐릭터의 위치를 Player의 현재 위치로 동기화 (Character.cpp의 Sync_Transform_FromPlayer 사용)
	//    - Player의 WorldMatrix는 이전 캐릭터로부터 이미 동기화되어 있음 (Sync_Transform_FromCharacter에서).
	_fmatrix PlayerWorldMatrix = m_pTransformCom->Get_WorldMatrix();
	m_Characters[m_iCurrentCharacterIdx]->Sync_Transform_FromPlayer(PlayerWorldMatrix, m_Characters[m_iPrevCharacterIdx]->Get_Velocity(), fTimeDelta);

	// 4. 새 캐릭터의 콜라이더 초기화 (속도 0으로 리셋, 중력 등 상태 복원)
	//    - CCharacter::m_pColliderCom->Update(XMVectorZero())로 속도 초기화.
	//    - 필요 시 Set_Gravity(true) 등으로 물리 상태 재설정.
	m_Characters[m_iCurrentCharacterIdx]->Sync_Collider(XMVectorZero(), fTimeDelta);  // 속도 0으로 초기화

	// 5. 상태 머신 초기화 (IDLE 상태로 자연스럽게 시작) => 새 캐릭터.
//    - 새 캐릭터의 StateContext 초기화 (e.g., IdleType 설정). => 모두 고정.
	m_Characters[m_iCurrentCharacterIdx]->Set_Gravity(true);  // 중력 활성화 (필요 시)
	m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::IDLE);
	

	m_Characters[m_iCurrentCharacterIdx]->Bind_ChangeEffect();

	// 6. 협주 확인. Ensemble
	// 이전 캐릭터의 협주게이지 확인 => Get_HarmonyGauge
	CHARACTERTYPE eCharacterType = static_cast<CHARACTERTYPE>(m_iPrevCharacterIdx);


	// 7. QTE 실행. 가능하면 ㄴ
	if (IsQTEPossible(eCharacterType))
	{
		// QTE 실행.
		ExecuteQTE(eCharacterType);
	}
	else
	{
		m_iHarmonyCharacterIdx = CHARACTERTYPE::NONE;
	}

	/*m_pPlayerStatus->;*/

	

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


void CPlayer::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// Detect Body 탐지용
	if (ENUM_CLASS(COLLISIONLAYER::ENEMY) != iLayer) 
		return;

	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	// CallBack Client Transform에 이상한 값이 들어가 있음.
    CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform); 
    if (nullptr == pTargetTransform)
        return;
	
	{
		
		lock_guard<mutex> lock(m_Mutex);
		// 캐스팅 타입이 안맞아서 터질 수 있으므로 정확한 Rule을 지켜서 Desc을 설정해야함.
		// Vector 컨테이너에 넣어줄 거면 
		m_TargetTransforms.push_back(pTargetTransform);
	}
}



void CPlayer::OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 공격과 스킬이 아니라면 호출하지 않습니다.
	if (ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::PARRY))
		return;

	if (nullptr == m_Characters[m_iCurrentCharacterIdx])
		return;

#ifdef _DEBUG
	cout << "Player Crash" << endl;
#endif // _DEBUG



	// 1. Parry일경우 우선순위 높음
	CALLBACK_CLIENT pClientDesc = *static_cast<CALLBACK_CLIENT*>(pDesc);
	CCharacter::HIT_DESC HitDesc{};
	CCharacter::PARRY_DESC ParryDesc{};
	

	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PARRY))
	{
		// 2. Parry 판정
		ParryDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		ParryDesc.fAttack = pClientDesc.fAttack;
		ParryDesc.iLayer = iLayer;
		m_Characters[m_iCurrentCharacterIdx]->Parry_Judge(&ParryDesc);
	}
	else
	{
		// 3. Hit 판정
		HitDesc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
		HitDesc.fAttack = pClientDesc.fAttack;
		HitDesc.iLayer = iLayer;
		m_Characters[m_iCurrentCharacterIdx]->Hit_Judge(&HitDesc);
	}
	

}

_bool CPlayer::Is_TargetValid(CTransform* pTarget)
{
	if (nullptr == pTarget)
		return false;


	auto iter = find(m_TargetTransforms.begin(), m_TargetTransforms.end(), pTarget);
	if (iter == m_TargetTransforms.end())
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

void CPlayer::Sorting_Target()
{
    sort(m_TargetTransforms.begin(), m_TargetTransforms.end(), [this](CTransform* pSrcTransform, CTransform* pDstTransform)->_bool {
        _float fSrcDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pSrcTransform->Get_State(STATE::POSITION)));
        _float fDstDistance = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - pDstTransform->Get_State(STATE::POSITION)));
        return fSrcDistance < fDstDistance;
        });

    if (0 < m_TargetTransforms.size())
    {
        m_pTargetTransform = m_TargetTransforms[0];
    }

    //m_TargetTransforms.clear();
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
			m_pLockOnTargetTransform = m_pTargetTransform;
		}
		else
		{
			m_pLockOnTargetTransform = nullptr;
		}
	}

	// 2. 매프레임 검증
	if (m_IsLockOn)
	{
		if (nullptr == m_pLockOnTargetTransform || !Is_TargetValid(m_pLockOnTargetTransform))
		{
			m_IsLockOn = false;
			m_pLockOnTargetTransform = nullptr;
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
			m_pLockOnTargetTransform = nullptr;
		}
	}

	// 5. 락온상태라면?
	if (m_IsLockOn)
	{
		// 하드 락온
		pFinalTarget = m_pLockOnTargetTransform;
		if (pCurrentCharacter)
			pCurrentCharacter->Set_LockOn(pFinalTarget, m_IsLockOn);
	}
	else
	{
		// 소프트 락온.
		pFinalTarget = m_pTargetTransform;
		if (pCurrentCharacter)
			pCurrentCharacter->Set_AutoLockOn(pFinalTarget, m_IsLockOn); // Character의 Set_AutoLockOn 호출
	}


	// 6. 카메라 업데이트.
	m_pSpringCamera->Lock_On(pFinalTarget, m_IsLockOn);

	/* if (nullptr == m_pTargetTransform)
	 {
		 if (m_IsLockOn)
		 {
			 m_IsLockOn = false;
			 m_pSpringCamera->Lock_On(nullptr, false);
			 if (m_iCurrentCharacterIdx !=NONE)
				 m_Characters[m_iCurrentCharacterIdx]->Set_LockOn(nullptr, false);

		 }
		 return;
	 }

	 if (nullptr != m_Characters[m_iCurrentCharacterIdx])
	 {
		 m_Characters[m_iCurrentCharacterIdx]->Set_AutoLockOn(m_pTargetTransform, m_IsLockOn);
	 }
	 m_pSpringCamera->Lock_On(pFinalTarget, m_IsLockOn);
	 */

    //m_pTargetTransform = nullptr;
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
    RigidbodyDesc.vExtent = _float3(200.f, 100.f, 200.f);
    XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

    if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
        TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        CRASH("Rigidbody");

    m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_During(iLayer, pDesc, Manifold);
    });


	//m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
	//	OnCollider_Enter(iLayer, pDesc, Manifold);
	//	});

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

    for (auto& pPlayer : m_Characters)
        Safe_Release(pPlayer);

    Safe_Release(m_pSpringCamera);
    Safe_Release(m_pInputControllerCom);
    Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pPlayerStatus);
}
