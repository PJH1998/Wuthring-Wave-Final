#include "ClientPch.h"
#include "PlayerController.h"
#include "Player.h"
#include "PlayerAugusta.h"

#pragma region 기본 함수
CPlayerController::CPlayerController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CPlayerController::CPlayerController(const CPlayerController& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CPlayerController::Initialize_Prototype()
{
    if (FAILED(CGameObject::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayerController::Initialize_Clone(void* pArg)
{

    PLAYER_CONTROLLER_DESC* pDesc = static_cast<PLAYER_CONTROLLER_DESC*>(pArg);

    m_eCurLevel = pDesc->eCurLevel;

    // 0. GameObject Clone
    if (FAILED(CGameObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 1. Players 초기화.
    //if (FAILED(Ready_Players(pDesc)))
    //    return E_FAIL;

    CPlayer::PLAYER_DESC Desc{};
    Desc = PlayerData::GetAugustaCloneData({1.f, 1.f, 1.f}
    , { 0.f, 0.f, 0.f }, { -14.1f, 50.f, -180.f }, m_eCurLevel);

    m_Players.resize(PLAYERTYPE::TYPE_END);
    m_Players[PLAYERTYPE::AUGUSTA] = dynamic_cast<CPlayer*>(
        m_pGameInstance->Clone_Prototype(ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_GameObject_Actor_Augusta")
            , PROTOTYPE::GAMEOBJECT, &Desc));




    return S_OK;
}

void CPlayerController::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
    m_Players[m_iCurrentPlayerIdx]->Priority_Update(fTimeDelta);

    //m_pAugusta->Priority_Update(fTimeDelta);
    
}

void CPlayerController::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
    m_Players[m_iCurrentPlayerIdx]->Update(fTimeDelta);
    //m_pAugusta->Update(fTimeDelta);
}

void CPlayerController::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);
    m_Players[m_iCurrentPlayerIdx]->Late_Update(fTimeDelta);
    //m_pAugusta->Late_Update(fTimeDelta);
}

void CPlayerController::Render()
{
    
}

void CPlayerController::Render_Shadow()
{

}

#pragma endregion

void CPlayerController::Ensemble_Skill(PLAYERTYPE iPlayerType)
{
    switch (iPlayerType)
    {
    case PLAYERTYPE::AUGUSTA:
        break;
    case PLAYERTYPE::GALBRENA:
        break;
    case PLAYERTYPE::PLAYER:
        break;
    }
}

HRESULT CPlayerController::Ready_Players(const PLAYER_CONTROLLER_DESC* pDesc)
{
    ASSERT_CRASH(pDesc);

    // 1. Players 공간 확보
    m_Players.resize(PLAYERTYPE::TYPE_END);

    
    CPlayer::PLAYER_DESC PlayerDesc;
    CPlayer* pPlayer = { nullptr };

    // 2. 캐릭터 별 데이터 초기화
    for (_uint i = 0; i < pDesc->iPlayerCount; ++i)
    {
        switch (i)
        {
        case PLAYERTYPE::AUGUSTA:
        {
            PlayerDesc = pDesc->PlayerSpecs[PLAYERTYPE::AUGUSTA].PlayerDesc;
            PlayerDesc.pController = this; // Controller Pointer만 전달?
            pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Clone_Prototype(
                ENUM_CLASS(m_eCurLevel),
                pDesc->PlayerSpecs[i].strActorTag,
                PROTOTYPE::GAMEOBJECT,
                &PlayerDesc));

            ASSERT_CRASH(pPlayer);

            /*if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel)
                , TEXT("Layer_Augusta"), pPlayer)))
                CRASH("Augusta");*/
            m_Players[i] = pPlayer;
            Safe_AddRef(pPlayer);
        }
            break;
        case PLAYERTYPE::GALBRENA:
            break;
        case PLAYERTYPE::PLAYER:
            break;
        default:
            break;
        }
    }

    // 기본 0번 Augusta
    m_iCurrentPlayerIdx = PLAYERTYPE::AUGUSTA;

    return S_OK;
}

CPlayerController* CPlayerController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPlayerController* pInstance = new CPlayerController(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CPlayerController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CPlayerController::Clone(void* pArg)
{
    CPlayerController* pInstance = new CPlayerController(*this);
    
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CPlayerController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPlayerController::Free()
{
    CGameObject::Free();

    for (auto& pPlayer : m_Players)
        Safe_Release(pPlayer);
}
