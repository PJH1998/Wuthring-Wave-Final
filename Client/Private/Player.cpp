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

    m_iCurrentCharacterIdx = AUGUSTA;
    //m_iCurrentCharacterIdx = ROVER; // 방랑자로 테스트

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

	if (m_iEnsembleCharacterIdx != NONE &&
		m_iEnsembleCharacterIdx != m_iCurrentCharacterIdx)
		m_Characters[m_iEnsembleCharacterIdx]->Priority_Update(fTimeDelta);


	// 4. 현재 비활성화되었든, 활성화되었든 업데이트는 플레이어에서 모두 실행 Update
	if (nullptr != m_pPlayerStatus)
		m_pPlayerStatus->Update(fTimeDelta);
}

void CPlayer::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
	if (m_iCurrentCharacterIdx != NONE)
	{
		Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]); // 변경 후에도 동기화 유지.
		m_Characters[m_iCurrentCharacterIdx]->Update(fTimeDelta);
	}

    // 2. Ensemble 
    if (m_iEnsembleCharacterIdx != NONE &&
        m_iEnsembleCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Update(fTimeDelta);

	// 3. Rigidbody Update => Camera 
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);

    Sorting_Target(); // Update => 
    Toggle_LockOn();
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);

    if (m_iCurrentCharacterIdx != NONE)
        m_Characters[m_iCurrentCharacterIdx]->Late_Update(fTimeDelta);

    if (m_iEnsembleCharacterIdx != NONE &&
        m_iEnsembleCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Late_Update(fTimeDelta);

	
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

	return m_Characters[m_iCurrentCharacterIdx]->Get_AbilityCom();
}
#pragma endregion

void CPlayer::Player_KeyInput()
{
	// Character Change
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D1)))
	{
		if (m_iCurrentCharacterIdx != CHARACTERTYPE::ROVER)
		{
			m_IsChanage = true;
			m_eNextCharacter = CHARACTERTYPE::ROVER;
			return;
		}

	}
	else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2)))
	{
		if (m_iCurrentCharacterIdx != CHARACTERTYPE::AUGUSTA)
		{
			m_IsChanage = true;
			m_eNextCharacter = CHARACTERTYPE::AUGUSTA;
			return;
		}
	}
	else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3)))
	{
		if (m_iCurrentCharacterIdx != CHARACTERTYPE::GALBRENA)
		{
			m_IsChanage = true;
			m_eNextCharacter = CHARACTERTYPE::GALBRENA;
			return;
		}
	}

#ifdef _DEBUG
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D4)))
	{
		m_Characters[m_iCurrentCharacterIdx]->Debug_FullCost();
	}
#endif // _DEBUGs
}

void CPlayer::Switch_Skill(CHARACTERTYPE eCharacter)
{
    CCharacter* pCharacter = m_Characters[eCharacter];

    pCharacter->Set_EnsembleEndCallback([this, eCharacter]() {
        this->On_EnsembleEnd(eCharacter);
    });


    switch (eCharacter)
    {
    case CHARACTERTYPE::AUGUSTA:
        pCharacter->Change_State(
            ENUM_CLASS(EStateCategory::GROUND),
            ENUM_CLASS(EAugustaSkillType::SKILLQTE));
        break;

    case CHARACTERTYPE::GALBRENA:
        // Galbrena Ensemble Skill
        break;

    case CHARACTERTYPE::ROVER:
        // Player Ensemble Skill
        pCharacter->Change_State(
            ENUM_CLASS(EStateCategory::GROUND),
            ENUM_CLASS(EAugustaSkillType::SKILLQTE));
        break;
    }
}

void CPlayer::Notify_EnsembleEnd()
{
    if (m_iEnsembleCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iEnsembleCharacterIdx]->SetActivate(false);
        m_iEnsembleCharacterIdx = CHARACTERTYPE::NONE;
    }
}

void CPlayer::Perform_CharacterSwitch(CHARACTERTYPE eNextCharacter)
{

}

// Callback
void CPlayer::On_EnsembleEnd(CHARACTERTYPE eCharacter)
{
    if (m_iEnsembleCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iEnsembleCharacterIdx]->SetActivate(false);
        m_Characters[m_iEnsembleCharacterIdx]->Clear_EnsembleEndCallback();
        m_iEnsembleCharacterIdx = CHARACTERTYPE::NONE;
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
		m_Characters[m_iCurrentCharacterIdx]->Collider_Active(TEXT("Body"), false); // 끄기.
		m_iPrevCharacterIdx = m_iCurrentCharacterIdx;
	}

	// 2. 새 캐릭터 활성화
	m_iCurrentCharacterIdx = eNextCharacter;
	m_Characters[m_iCurrentCharacterIdx]->SetActivate(true);
	m_Characters[m_iCurrentCharacterIdx]->Collider_Active(TEXT("Body"), true); // 콜라이더 활성화

	// 3. 새 캐릭터의 위치를 Player의 현재 위치로 동기화 (Character.cpp의 Sync_Transform_FromPlayer 사용)
	//    - Player의 WorldMatrix는 이전 캐릭터로부터 이미 동기화되어 있음 (Sync_Transform_FromCharacter에서).
	_fmatrix PlayerWorldMatrix = m_pTransformCom->Get_WorldMatrix();
	m_Characters[m_iCurrentCharacterIdx]->Sync_Transform_FromPlayer(PlayerWorldMatrix, m_Characters[m_iPrevCharacterIdx]->Get_Velocity(), fTimeDelta);

	// 4. 새 캐릭터의 콜라이더 초기화 (속도 0으로 리셋, 중력 등 상태 복원)
	//    - CCharacter::m_pColliderCom->Update(XMVectorZero())로 속도 초기화.
	//    - 필요 시 Set_Gravity(true) 등으로 물리 상태 재설정.
	m_Characters[m_iCurrentCharacterIdx]->Sync_Collider(XMVectorZero(), fTimeDelta);  // 속도 0으로 초기화

	// 5. 상태 머신 초기화 (IDLE 상태로 자연스럽게 시작)
	//    - 새 캐릭터의 StateContext 초기화 (e.g., IdleType 설정). => 모두 고정.
	m_Characters[m_iCurrentCharacterIdx]->Set_Gravity(true);  // 중력 활성화 (필요 시)
	m_Characters[m_iCurrentCharacterIdx]->TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE::IDLE);
	

	// 6. 카메라/입력 컨트롤러 재설정 (캐릭터 변경 후 카메라가 새 위치 따라가도록)

	// 7. (옵션) 애니메이션 초기화: 등장 애니메이션 재생 

	// 8. UI/게이지 동기화 (캐릭터 스탯 유지)

}

void CPlayer::Sync_Transform_FromCharacter(CCharacter* pCharacter)
{
	if (nullptr == pCharacter)
		return;
	pCharacter->Sync_Transform_ToPlayer(m_pTransformCom); // 현재 캐릭터의 Transform을 Player와 동기화
}


void CPlayer::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (ENUM_CLASS(COLLISIONLAYER::ENEMY) != iLayer)
		return;

    CTransform* pTargetTransform = static_cast<CTransform*>(pDesc);
    if (nullptr == pTargetTransform)
        return;
    m_TargetTransforms.push_back(pTargetTransform);
}



void CPlayer::OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 공격과 스킬이 아니라면 호출하지 않습니다.
	if (ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK) != iLayer && 
		ENUM_CLASS(COLLISIONLAYER::ENEMY_SKILL) != iLayer)
		return;

	if (nullptr == m_Characters[m_iCurrentCharacterIdx])
		return;

	CALLBACK_CLIENT pClientDesc = *static_cast<CALLBACK_CLIENT*>(pDesc);

	CCharacter::HIT_DESC Desc{};
	Desc.pTransform = static_cast<CTransform*>(pClientDesc.pTransform);
	Desc.fAttack = pClientDesc.fAttack;
	Desc.iLayer = iLayer;

	
	// Hit 판정 전달.
	m_Characters[m_iCurrentCharacterIdx]->Hit_Judge(&Desc);
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

    m_TargetTransforms.clear();
}

void CPlayer::Toggle_LockOn()
{
    if (nullptr == m_pTargetTransform)
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
        m_Characters[m_iCurrentCharacterIdx]->Set_LockOn(m_pTargetTransform, m_IsLockOn);
    }


    if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::WB), KEYSTATE::DOWN))
    {
        m_IsLockOn = !m_IsLockOn;
    }

   // if (m_IsLockOn)
   // {
        m_pSpringCamera->Lock_On(m_pTargetTransform, m_IsLockOn);
      //  return;
    //}

    m_pTargetTransform = nullptr;
    
}


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
            break;
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
    RigidbodyDesc.vExtent = _float3(1000.f, 400.f, 1000.f);
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
	m_pColliderCom->Set_Desc(m_pTransformCom);
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
