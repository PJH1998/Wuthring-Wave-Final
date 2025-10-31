#include "ClientPch.h"
#include "Player.h"
#include "Character.h"
#include "Augusta.h"
#include "AugustaState_Enum.h"
#include "SpringCamera.h"
#include "PlayerFactory.h"
#include "GameSystem.h"

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
        }
    }

    // 5. Transform
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

    m_iCurrentCharacterIdx = AUGUSTA;
    //m_iCurrentCharacterIdx = ROVER; // 방랑자로 테스트


    return S_OK;
}

void CPlayer::Priority_Update(_float fTimeDelta)
{
    CGameObject::Priority_Update(fTimeDelta);
    
    m_pInputControllerCom->Update();
    
    if (m_iCurrentCharacterIdx != NONE)
        m_Characters[m_iCurrentCharacterIdx]->Priority_Update(fTimeDelta);

    if (m_iEnsembleCharacterIdx != NONE && 
        m_iEnsembleCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Priority_Update(fTimeDelta);
        
	// 캐릭터 체인지 => 추후 구현
	Change_CharacterCheck();

    // 임시.
    if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D4)))
    {
        m_Characters[m_iCurrentCharacterIdx]->Add_UniqueGauge(100.f);
        m_Characters[m_iCurrentCharacterIdx]->Add_BurstGauge(100.f);
    }
    if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D5)))
    {
        m_Characters[m_iCurrentCharacterIdx]->Add_UniqueGauge(-100.f);
        m_Characters[m_iCurrentCharacterIdx]->Add_BurstGauge(-100.f);
    }
    if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D6)))
    {
        m_Characters[m_iCurrentCharacterIdx]->Hit_Judge(nullptr);
    }

}

void CPlayer::Update(_float fTimeDelta)
{
    CGameObject::Update(fTimeDelta);
	if (m_iCurrentCharacterIdx != NONE)
	{
		Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]);
		m_Characters[m_iCurrentCharacterIdx]->Update(fTimeDelta);
	}
        
        
    // 2. Ensemble 
    if (m_iEnsembleCharacterIdx != NONE &&
        m_iEnsembleCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Update(fTimeDelta);

    Sorting_Target(); // Update => 
    Toggle_LockOn();
    
    // 3. Rigidbody Update => Camera 
    m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CPlayer::Late_Update(_float fTimeDelta)
{
    CGameObject::Late_Update(fTimeDelta);

    if (m_iCurrentCharacterIdx != NONE)
        m_Characters[m_iCurrentCharacterIdx]->Late_Update(fTimeDelta);

    if (m_iEnsembleCharacterIdx != NONE &&
        m_iEnsembleCharacterIdx != m_iCurrentCharacterIdx)
        m_Characters[m_iEnsembleCharacterIdx]->Late_Update(fTimeDelta);

	// 맨 끝에.
	//Sync_Transform_FromCharacter(m_Characters[m_iCurrentCharacterIdx]);

	if (m_IsChanage)
	{
		m_IsChanage = false;
		Change_Character(m_eNextCharacter);
	}
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
	{
		m_IsChanage = true;
		m_eNextCharacter = CHARACTERTYPE::AUGUSTA;
	}
    else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2)))
	{
		m_IsChanage = true;
		m_eNextCharacter = CHARACTERTYPE::GALBRENA;
	}
    else if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3)))
	{
		m_IsChanage = true;
		m_eNextCharacter = CHARACTERTYPE::ROVER;
	}
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

void CPlayer::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (ENUM_CLASS(COLLISIONLAYER::ENEMY) != iLayer)
		return;

    CTransform* pTargetTransform = static_cast<CTransform*>(pDesc);
    if (nullptr == pTargetTransform)
    {
        return;
    }
        
    m_TargetTransforms.push_back(pTargetTransform);
}

void CPlayer::Change_Character(CHARACTERTYPE eNextCharacter)
{
    if (eNextCharacter < 0 || eNextCharacter >= TYPE_END)
        return;

    if (m_Characters[eNextCharacter] == nullptr)
        return;

    if (m_iCurrentCharacterIdx == eNextCharacter)
        return;

    CCharacter* pCurrentCharacter = nullptr;
    if (m_iCurrentCharacterIdx != CHARACTERTYPE::NONE)
        pCurrentCharacter = m_Characters[m_iCurrentCharacterIdx];

    _bool bUseEnsemble = false;
    if (pCurrentCharacter && pCurrentCharacter->Is_SwitchGaugeFull())
        bUseEnsemble = true;


	_matrix matPrevWorldMatrix = XMMatrixIdentity();
	_float4 vPrevPosition = {};
	_bool bHasPrevCharacter = false;

	// 이전 캐릭터 비활성화
	CCharacter* pPrevCharacter = m_Characters[m_iCurrentCharacterIdx];
	

	// 다음캐릭 활성화
	CCharacter* pNextCharacter = m_Characters[eNextCharacter];
	pNextCharacter->SetActivate(true);

	// 현재 Matrix를 다음 캐릭터의 트랜스폼에 설정.
	//CTransform* pTransform = dynamic_cast<CTransform*>(pPrevCharacter->Get_Component(L"Com_Transform"));
	//if (nullptr == pTransform)
	//	return;

	_matrix mat = m_pTransformCom->Get_WorldMatrix();
	/*_matrix mat = pTransform->Get_WorldMatrix();*/
	m_Characters[eNextCharacter]->Sync_Transform_FromPlayer(mat);

	// 현재 선택된 캐릭터로 CurrentCharacter Idx 변경.
	m_iPrevCharacterIdx = m_iCurrentCharacterIdx;
	m_iCurrentCharacterIdx = eNextCharacter;

	pPrevCharacter->SetActivate(false);

	// 최근 캐릭터가 존재하고, Ensemble 게이지가 가득찼다면?
	if (bUseEnsemble && pPrevCharacter)
	{
		pPrevCharacter->SetActivate(true);
		//pCurrentCharacter->SetActivate(true);
		Switch_Skill(static_cast<CHARACTERTYPE>(m_iPrevCharacterIdx));
		pPrevCharacter->Reset_SwitchGauge(); // 이전 캐릭터 SwitchGauge 지우기.
	}

    //Perform_CharacterSwitch(eNextCharacter);


}

void CPlayer::Sync_Transform_FromCharacter(CCharacter* pCharacter)
{
	if (nullptr == pCharacter)
		return;
	pCharacter->Sync_Transform_ToPlayer(m_pTransformCom);
}


void CPlayer::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
		return;

    CTransform* pTargetTransform = static_cast<CTransform*>(pDesc);
    if (nullptr == pTargetTransform)
        return;
    m_TargetTransforms.push_back(pTargetTransform);
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
        OnCollide_During(iLayer, pDesc, Manifold);
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
}
