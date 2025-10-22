#include "ClientPch.h"
#include "PlayerParty.h"
#include "Player.h"
#include "PlayerAugusta.h"

#pragma region 기본 함수
CPlayerParty::CPlayerParty(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CPlayerParty::CPlayerParty(const CPlayerParty& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CPlayerParty::Initialize_Prototype()
{
    if (FAILED(CGameObject::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayerParty::Initialize_Clone(void* pArg)
{

    PLAYER_PARTY_DESC* pDesc = static_cast<PLAYER_PARTY_DESC*>(pArg);

    m_eCurLevel = pDesc->eCurLevel;

    // 0. GameObject Clone
    if (FAILED(CGameObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 1. Players 초기화.
    if (FAILED(Ready_Players(pDesc)))
        return E_FAIL;

    /*CPlayer::PLAYER_DESC Desc{};
    Desc = PlayerData::GetAugustaCloneData({1.f, 1.f, 1.f}
    , { 0.f, 0.f, 0.f }, { -14.1f, 50.f, -180.f }, m_eCurLevel);

    m_Players.resize(PLAYERTYPE::TYPE_END);
    m_Players[PLAYERTYPE::AUGUSTA] = dynamic_cast<CPlayer*>(
        m_pGameInstance->Clone_Prototype(ENUM_CLASS(pDesc->eCurLevel), PlayerData::AUGUSTA_ACTOR_TAG
            , PROTOTYPE::GAMEOBJECT, &Desc));*/




    return S_OK;
}

void CPlayerParty::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
    m_Players[m_iCurrentPlayerIdx]->Priority_Update(fTimeDelta);

}

void CPlayerParty::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
    m_Players[m_iCurrentPlayerIdx]->Update(fTimeDelta);
}

void CPlayerParty::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);
    m_Players[m_iCurrentPlayerIdx]->Late_Update(fTimeDelta);
}

void CPlayerParty::Render()
{
    
}

void CPlayerParty::Render_Shadow()
{

}

#pragma endregion

void CPlayerParty::Ensemble_Skill(PLAYERTYPE iPlayerType)
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

HRESULT CPlayerParty::Ready_Players(const PLAYER_PARTY_DESC* pDesc)
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
            m_Players[i] = pPlayer;
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

CPlayerParty* CPlayerParty::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPlayerParty* pInstance = new CPlayerParty(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CPlayerParty");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CPlayerParty::Clone(void* pArg)
{
    CPlayerParty* pInstance = new CPlayerParty(*this);
    
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CPlayerParty");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPlayerParty::Free()
{
    CGameObject::Free();

    for (auto& pPlayer : m_Players)
        Safe_Release(pPlayer);
}
