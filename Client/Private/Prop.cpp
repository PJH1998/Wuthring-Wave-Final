#include "ClientPch.h"
#include "Prop.h"
#include "Character.h"
#include "AttackVolume.h"

CProp::CProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CProp::CProp(const CPartObject& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CProp::Initialize_Prototype()
{
    if (FAILED(CPartObject::Initialize_Prototype()))
        return E_FAIL;
    return S_OK;
}

HRESULT CProp::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eWeaponType = pDesc->eWeaponType;
    return S_OK;
}

void CProp::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
    CPartObject::Priority_Update(fTimeDelta);

}

void CProp::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Update(fTimeDelta);
}

void CProp::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Late_Update(fTimeDelta);
}

void CProp::Activate(_bool IsActive)
{
    SetActivate(IsActive);
	
	if (!IsActive)
	{
		m_fTrackPosition = 0.f;
	}

    if (nullptr == m_pRigidbodyCom)
        return;
        
}

void CProp::Volume_Activate(_bool IsActive)
{
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->TriggerActivate(IsActive);
}





void CProp::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _bool IsLoop)
{
    ASSERT_CRASH(m_pModelCom);

    //m_IsAnimationEnd = m_pModelCom->Play_Animation_GPU(
    //    m_pComputeShaderCom, strAnimName, fTimeDelta, &m_fTrackPosition, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate, fRootMotionRate);
    m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(
        strAnimName, fTimeDelta, &m_fTrackPosition, false, IsRootMotion, fRootMotionRate);
    m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
}

void CProp::Clear_Animation(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    m_pModelCom->Clear_Animation(strAnimName);
	m_fTrackPosition = 0.f;
	m_strCurrentAnimName.clear();  // 추가
}

#pragma region NOTIFY
void CProp::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	if (!IsActive)
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
	else
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::ATTACK));
}
void CProp::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	_matrix matWorld = XMLoadFloat4x4(&m_CombinedMatrix);
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}
void CProp::Collider_Active(_bool isActive)
{
    if (!isActive)
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
    else 
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::ATTACK));
}
#pragma endregion




void CProp::Register_AllNotifies(const _string& strFolderPath)
{
	ASSERT_CRASH(m_pModelCom);
	auto colliderCallback = [this](const _wstring& tag, bool active) {
		this->Collider_Active(tag, active); // 
		};

	auto effectCallBack = [this](const _wstring& tag) {
		this->Effect_Active(tag);
		};

	auto objectCallBack = [this](const _wstring& tag) {
		this->Object_Func(tag);
		};

	m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack, objectCallBack);
}

void CProp::Free()
{
    CPartObject::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pRigidbodyCom);
	
	// AttackVolumes 제거.
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}
		

	m_AttackVolumes.clear();

	//Safe_Release(m_pMainAttackVolume);
    m_pParentTransform = { nullptr };
}
