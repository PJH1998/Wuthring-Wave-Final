#include "ClientPch.h"
#include "Weapon.h"
#include "Character.h"

CWeapon::CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CWeapon::CWeapon(const CPartObject& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CWeapon::Initialize_Prototype()
{
    if (FAILED(CPartObject::Initialize_Prototype()))
        return E_FAIL;
    return S_OK;
}

HRESULT CWeapon::Initialize_Clone(void* pArg)
{
    WEAPON_DESC* pDesc = static_cast<WEAPON_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eWeaponType = pDesc->eWeaponType;
    return S_OK;
}

void CWeapon::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
    CPartObject::Priority_Update(fTimeDelta);
}

void CWeapon::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Update(fTimeDelta);
}

void CWeapon::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Late_Update(fTimeDelta);
}

void CWeapon::Activate(_bool IsActive)
{
    SetActivate(IsActive);

	if (!IsActive)
	{
		m_fTrackPosition = 0.f;
	}

    if (nullptr == m_pRigidbodyCom)
        return;

    if (IsActive)
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
    else
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::ATTACK));
        
}


void CWeapon::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _bool IsLoop)
{
    ASSERT_CRASH(m_pModelCom);


	// 같은 애니메이션 재시작 방지 + 트랙 초기화
	//if (m_strCurrentAnimName != strAnimName)
	//{
	//	m_fTrackPosition = 0.f;
	//	m_strCurrentAnimName = strAnimName;
	//	m_IsAnimationEnd = false;
	//}

    //m_IsAnimationEnd = m_pModelCom->Play_Animation_GPU(
    //    m_pComputeShaderCom, strAnimName, fTimeDelta, &m_fTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
    m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(
        strAnimName, fTimeDelta, &m_fTrackPosition, false, IsRootMotion, fRootMotionRate);
    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
}

void CWeapon::Clear_Animation(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    m_pModelCom->Clear_Animation(strAnimName);
	m_fTrackPosition = 0.f;
	m_strCurrentAnimName.clear();  // 추가
}

#pragma region NOTIFY
void CWeapon::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	if (!IsActive)
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
	else
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::ATTACK));
}
void CWeapon::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	_matrix matWorld = XMLoadFloat4x4(&m_CombinedMatrix);
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}
void CWeapon::Collider_Active(_bool isActive)
{
    if (!isActive)
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
    else 
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::ATTACK));
}
#pragma endregion




void CWeapon::Register_AllNotifies(const _string& strFolderPath)
{
	ASSERT_CRASH(m_pModelCom);
	auto colliderCallback = [this](const _wstring& tag, bool active) {
		this->Collider_Active(tag, active); // 
		};

	auto effectCallBack = [this](const _wstring& tag) {
		this->Effect_Active(tag);
		};

	m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack);
}

void CWeapon::Free()
{
    CPartObject::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pRigidbodyCom);
    m_pParentTransform = { nullptr };
}
