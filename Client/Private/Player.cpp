#include "ClientPch.h"
#include "Player.h"
#include "Character.h"
#include "Augusta.h"
#include "AugustaState_Enum.h"
#include "PlayerFactory.h"

#pragma region 기본 함수
CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer& Prototype)
    : CGameObject(Prototype)
{
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

    // 0. GameObject Clone
    if (FAILED(CGameObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 1. 공유 Components 초기화
    if (FAILED(Ready_Components(pDesc)))
        return E_FAIL;

    // 2. Players 초기화.
    if (FAILED(Ready_Players(pDesc)))
        return E_FAIL;

    // 3. Controller 공유
    for (auto& pCharacter : m_Characters)
    {
        if (nullptr != pCharacter)
            pCharacter->Process_Input(m_pInputControllerCom);
    }


    // 4. Factory 초기화
    CPlayerFactory::Register_KeyInputs(m_pInputControllerCom, this);

    // 5. Transform 초기화
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

    // 기본 NONE => 테스트용도 => 원래는 AUGUSTA로
    m_iCurrentPlayerIdx = AUGUSTA;


    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
    
    // 1. InputController 업데이트.
    m_pInputControllerCom->Update();
    
    // 2. 활성 캐릭터 업데이트
    if (m_iCurrentPlayerIdx != NONE)
        m_Characters[m_iCurrentPlayerIdx]->Priority_Update(fTimeDelta);

    // 3. Ensemble 캐릭터도 업데이트
    if (m_iEnsembleCharacterIdx != NONE && 
        m_iEnsembleCharacterIdx != m_iCurrentPlayerIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Priority_Update(fTimeDelta);
        

}

void CPlayer::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
    // 1. 현재 캐릭터 Update
    if (m_iCurrentPlayerIdx != NONE)
        m_Characters[m_iCurrentPlayerIdx]->Update(fTimeDelta);
        
    // 2. Ensemble 캐릭터도 업데이트
    if (m_iEnsembleCharacterIdx != NONE &&
        m_iEnsembleCharacterIdx != m_iCurrentPlayerIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Update(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);

    // 1. 캐릭터 업데이트
    if (m_iCurrentPlayerIdx != NONE)
        m_Characters[m_iCurrentPlayerIdx]->Late_Update(fTimeDelta);

    // 2. Ensemble 캐릭터 업데이트
    if (m_iEnsembleCharacterIdx != NONE &&
        m_iEnsembleCharacterIdx != m_iCurrentPlayerIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Update(fTimeDelta);

    // 3. 키입력에서 바꾸는 입력이 확인 되었으면?
    Change_CharacterCheck();
}
void CPlayer::Render()
{
    
}

void CPlayer::Render_Shadow()
{

}

#pragma endregion

void CPlayer::Change_CharacterCheck()
{
    if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D1)))
        Change_Character(CHARACTERTYPE::AUGUSTA);
    else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2)))
        Change_Character(CHARACTERTYPE::GALBRENA);
    else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3)))
        Change_Character(CHARACTERTYPE::PLAYER);
}

void CPlayer::Ensemble_Skill(CHARACTERTYPE eCharacter)
{
    // 현재 캐릭터의 Ensemble Skill State로 전환
    CCharacter* pCharacter = m_Characters[eCharacter];

    // Ensemble 종료 조건 Character에 CallBack 등록하기.
    pCharacter->Set_EnsembleEndCallback([this, eCharacter]() {
        this->On_EnsembleEnd(eCharacter);
    });


    switch (eCharacter)
    {
    case CHARACTERTYPE::AUGUSTA:
        // Augusta의 Ensemble Skill State로 전환
        pCharacter->Change_State(
            ENUM_CLASS(EStateCategory::GROUND),
            ENUM_CLASS(ESkillType::SKILLQTE));
        break;

    case CHARACTERTYPE::GALBRENA:
        // Galbrena Ensemble Skill
        break;

    case CHARACTERTYPE::PLAYER:
        // Player Ensemble Skill
        break;
    }
}

void CPlayer::Notify_EnsembleEnd()
{
    // Ensemble Skill 끝났으면 해당 캐릭터 비활성화
    if (m_iEnsembleCharacterIdx != CHARACTERTYPE::NONE)
    {
        // 안보여지게 합니다.
        m_Characters[m_iEnsembleCharacterIdx]->SetActivate(false);
        m_iEnsembleCharacterIdx = CHARACTERTYPE::NONE;
    }
}

void CPlayer::Perform_CharacterSwitch(CHARACTERTYPE eNextCharacter)
{
    // 실제 캐릭터 전환 로직 (이전에 작성한 내용)
    _matrix matPrevWorldMatrix = XMMatrixIdentity();
    _float4 vPrevPosition = {};
    _bool bHasPrevCharacter = false;

    
    // 1. 이전 캐릭터 비활성화
    if (m_iCurrentPlayerIdx != CHARACTERTYPE::NONE)
    {
        CCharacter* pPrevCharacter = m_Characters[m_iCurrentPlayerIdx];
        CTransform* pPrevTransform = dynamic_cast<CTransform*>(
            pPrevCharacter->Get_Component(L"Com_Transform"));

        if (pPrevTransform)
        {
            // 위치, 회전, 스케일 저장
            matPrevWorldMatrix = pPrevTransform->Get_WorldMatrix();
            // 현재 위치 저장 (다음 프레임 Velocity 계산용)
            XMStoreFloat4(&vPrevPosition, pPrevTransform->Get_State(STATE::POSITION));

            bHasPrevCharacter = true;
        }

        // 이전 캐릭터 비활성화
        pPrevCharacter->SetActivate(false);
    }

    // 2. 새 캐릭터 활성화
    CCharacter* pNextCharacter = m_Characters[eNextCharacter];
    pNextCharacter->SetActivate(true);

    // 3. Transform 동기화
    CTransform* pNextTransform = dynamic_cast<CTransform*>(
        pNextCharacter->Get_Component(L"Com_Transform"));

    // 4. 위치 동기화
    if (pNextTransform)
    {
        if (m_iCurrentPlayerIdx == CHARACTERTYPE::NONE)
        {
            // NONE에서 전환: Player Transform 사용
            if (m_pTransformCom)
                pNextTransform->Set_WorldMatrix(m_pTransformCom->Get_WorldMatrix());
        }
        else
        {
            // 캐릭터 간 전환: 이전 캐릭터 위치/회전 복사
            pNextTransform->Set_WorldMatrix(matPrevWorldMatrix);
            // PreviousPosition도 동기화 (Velocity 0으로 시작)
            pNextTransform->Save_PreviousPosition();
        }
    }

    // 4. Collider Position 동기화
    CCollider* pNextCollider = dynamic_cast<CCollider*>(
        pNextCharacter->Get_Component(L"Com_Collider"));
    if (pNextCollider && pNextTransform)
        pNextCollider->Sync_Position(pNextTransform);

    // 5. 인덱스 변경
    m_iPrevPlayerIdx = m_iCurrentPlayerIdx;
    m_iCurrentPlayerIdx = eNextCharacter;

    // 6. Player Transform 업데이트
    if (m_pTransformCom && pNextTransform)
        m_pTransformCom->Set_WorldMatrix(pNextTransform->Get_WorldMatrix());

}

// Callback에서 호출될 함수
void CPlayer::On_EnsembleEnd(CHARACTERTYPE eCharacter)
{
    if (m_iEnsembleCharacterIdx != CHARACTERTYPE::NONE)
    {
        m_Characters[m_iEnsembleCharacterIdx]->SetActivate(false);
        m_Characters[m_iEnsembleCharacterIdx]->Clear_EnsembleEndCallback();
        m_iEnsembleCharacterIdx = CHARACTERTYPE::NONE;
    }
}

void CPlayer::Change_Character(CHARACTERTYPE eNextCharacter)
{
    // 0. 유효성 검사
    if (eNextCharacter < 0 || eNextCharacter >= TYPE_END)
        return;

    if (m_Characters[eNextCharacter] == nullptr) // 비어있다면?
        return;

    if (m_iCurrentPlayerIdx == eNextCharacter) // 같은 캐릭터면?
        return;

    CCharacter* pCurrentCharacter = nullptr;
    if (m_iCurrentPlayerIdx != CHARACTERTYPE::NONE)
        pCurrentCharacter = m_Characters[m_iCurrentPlayerIdx];

    // 1. 현재 캐릭터의 Ensemble Energy 체크
    _bool bUseEnsemble = false;
    if (pCurrentCharacter && pCurrentCharacter->Is_EnsembleFull())
        bUseEnsemble = true;

    // 2. 먼저 캐릭터 전환 실행
    Perform_CharacterSwitch(eNextCharacter);

    // 3. 전환 후 이전 캐릭터의 Ensemble Skill 사용.
    if (bUseEnsemble && pCurrentCharacter)
    {
        // 이전 캐릭터를 다시 활성화 (스킬 사용 위해)
        pCurrentCharacter->SetActivate(true);
        Ensemble_Skill(static_cast<CHARACTERTYPE>(m_iPrevPlayerIdx));
        pCurrentCharacter->Reset_EnsembleEnergy();
    }

}

void CPlayer::Sync_Transform()
{
    if (m_iCurrentPlayerIdx != TYPE_END)
    {
        CTransform* pTransform = dynamic_cast<CTransform*>(
            m_Characters[m_iCurrentPlayerIdx]->Get_Component(L"Com_Transform"));
        ASSERT_CRASH(pTransform);
        m_pTransformCom->Set_WorldMatrix(pTransform->Get_WorldMatrix());
    }

        
}

HRESULT CPlayer::Ready_Players(const PLAYER_DESC* pDesc)
{
    ASSERT_CRASH(pDesc);

    // 1. Players 공간 확보
    m_Characters.resize(CHARACTERTYPE::TYPE_END);
    
    CCharacter::CHARACTER_DESC CharacterDesc;
    CCharacter* pPlayer = { nullptr };

    // 2. 캐릭터 별 데이터 초기화
    for (_uint i = 0; i < pDesc->iPlayerCount; ++i)
    {
        switch (i)
        {
        case CHARACTERTYPE::AUGUSTA:
        {
            CharacterDesc = pDesc->PlayerSpecs[CHARACTERTYPE::AUGUSTA].CharacterDesc;
            CharacterDesc.pOwner = this; // Controller Pointer만 전달?
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
        case CHARACTERTYPE::PLAYER:
            break;
        default:
            break;
        }
    }

    // 기본 0번 Augusta
    //m_iCurrentPlayerIdx = CHARACTERTYPE::AUGUSTA;
    // 테스트로 None
    m_iCurrentPlayerIdx = CHARACTERTYPE::NONE;

    return S_OK;
}

HRESULT CPlayer::Ready_Components(const PLAYER_DESC* pDesc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->eCurLevel)
        , pDesc->wStrInputControllerTag, TEXT("Com_InputController"), reinterpret_cast<CComponent**>(&m_pInputControllerCom), nullptr)))
        CRASH("Input Controller");

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

    for (auto& pPlayer : m_Characters)
        Safe_Release(pPlayer);

    Safe_Release(m_pInputControllerCom);
}
