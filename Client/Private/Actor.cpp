#include "ClientPch.h"
#include "Actor.h"
#include "Ability.h"


#pragma region
CActor::CActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{

}

CActor::CActor(const CActor& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CActor::Initialize_Prototype()
{
    if (FAILED(CContainerObject::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CActor::Initialize_Clone(void* pArg)
{
    ACTOR_DESC* pDesc = static_cast<ACTOR_DESC*>(pArg);
    if (FAILED(CContainerObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    return S_OK;
}

void CActor::Priority_Update(_float fTimeDelta)
{
    CContainerObject::Priority_Update(fTimeDelta);
}

void CActor::Update(_float fTimeDelta)
{
    CContainerObject::Update(fTimeDelta);
}

void CActor::Late_Update(_float fTimeDelta)
{
    CContainerObject::Late_Update(fTimeDelta);
}

void CActor::Render()
{

}

SKILL_STATE CActor::Check_Skill(const _string& strSkillName, const _string& strPrevName)
{
	if (nullptr == m_pAbillityCom)
		return SKILL_STATE::NOT_EXIST;

	return m_pAbillityCom->Check_SkillState(strSkillName, strPrevName);
}

SKILL_STATE CActor::Use_Skill(const _string& strSkillName)
{
	if (nullptr == m_pAbillityCom)
		return SKILL_STATE::NOT_EXIST;

	return m_pAbillityCom->TryUseSkill(strSkillName);
}

#ifdef _DEBUG
void CActor::Print_Cost()
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Print_Cost();
}
void CActor::Print_CoolTime()
{
	if (nullptr == m_pAbillityCom)
		return;

	m_pAbillityCom->Print_CoolTime();
}
#endif // _DEBUG


#pragma endregion

void CActor::Register_AllNotifies(const _string& strFolderPath)
{
    ASSERT_CRASH(m_pModelCom);
    auto colliderCallback = [this](const _wstring& tag, bool active) {
        this->Collider_Active(tag, active); 
        };

    auto effectCallBack = [this](const _wstring& tag) {
        this->Effect_Active(tag);
        };

    m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack);
}

// Ability Files 등록.
void CActor::Register_AbilityFiles(const _string& strFolderPath)
{
	ASSERT_CRASH(m_pAbillityCom);
	m_pAbillityCom->Register_AllAbilityFiles(strFolderPath);
}


void CActor::Free()
{
    CContainerObject::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
    Safe_Release(m_pRigidBodyCom);
    Safe_Release(m_pColliderCom);
	Safe_Release(m_pAbillityCom);
}
