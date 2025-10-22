#include "ClientPch.h"
#include "Player.h"
#include "Character.h"
#include "Augusta.h"

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

    PLAYER_PARTY_DESC* pDesc = static_cast<PLAYER_PARTY_DESC*>(pArg);

    m_eCurLevel = pDesc->eCurLevel;

    // 0. GameObject Clone
    if (FAILED(CGameObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    // 1. Players 초기화.
    if (FAILED(Ready_Players(pDesc)))
        return E_FAIL;

    /*CCharacter::PLAYER_DESC Desc{};
    Desc = PlayerData::GetAugustaCloneData({1.f, 1.f, 1.f}
    , { 0.f, 0.f, 0.f }, { -14.1f, 50.f, -180.f }, m_eCurLevel);

    m_Players.resize(CHARACTERTYPE::TYPE_END);
    m_Players[CHARACTERTYPE::AUGUSTA] = dynamic_cast<CCharacter*>(
        m_pGameInstance->Clone_Prototype(ENUM_CLASS(pDesc->eCurLevel), PlayerData::AUGUSTA_ACTOR_TAG
            , PROTOTYPE::GAMEOBJECT, &Desc));*/




    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
    m_Characters[m_iCurrentPlayerIdx]->Priority_Update(fTimeDelta);

}

void CPlayer::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
    m_Characters[m_iCurrentPlayerIdx]->Update(fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);
    m_Characters[m_iCurrentPlayerIdx]->Late_Update(fTimeDelta);
}

void CPlayer::Render()
{
    
}

void CPlayer::Render_Shadow()
{

}

#pragma endregion

void CPlayer::Ensemble_Skill(CHARACTERTYPE iPlayerType)
{
    switch (iPlayerType)
    {
    case CHARACTERTYPE::AUGUSTA:
        break;
    case CHARACTERTYPE::GALBRENA:
        break;
    case CHARACTERTYPE::PLAYER:
        break;
    }
}

HRESULT CPlayer::Ready_Players(const PLAYER_PARTY_DESC* pDesc)
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
    m_iCurrentPlayerIdx = CHARACTERTYPE::AUGUSTA;

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
}
