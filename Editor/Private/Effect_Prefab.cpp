#include "Editorpch.h"
#include "Effect_Prefab.h"
#include "Particle.h"

CEffect_Prefab::CEffect_Prefab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Prefab::CEffect_Prefab(const CEffect_Prefab& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Prefab::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Prefab::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    return S_OK;
}

void CEffect_Prefab::Priority_Update(_float fTimeDelta)
{
    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Priority_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Update(_float fTimeDelta)
{
    //자식들 설정값에 따라 Activate 활성화 해줘야함.

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Update(fTimeDelta);
    }
}

void CEffect_Prefab::Late_Update(_float fTimeDelta)
{
    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Late_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Render()
{
    //랜더 없어도 될듯
}

void CEffect_Prefab::Add_Children(void* pArg)
{
    //자식들 추가 (파티클이면 파티클 Desc필요)
    //자식 추가할 때 파티클인지 뭔지 알아야할거 같은데?
    //일단 임시로 파티클 고정

    CGameObject* pChildren = {};
    CParticle::PARTICLE_DESC* pDesc = static_cast<CParticle::PARTICLE_DESC*>(pArg);

    pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"), PROTOTYPE::GAMEOBJECT, pArg));

    if (pChildren == nullptr)
        return;

    m_EffectChildren.emplace(pDesc->strMyTag, pChildren);
}

void CEffect_Prefab::Remove_Children(_wstring& ChildrenTag)
{
   auto iter = m_EffectChildren.find(ChildrenTag);

   //잘못된 키값
   if (iter == m_EffectChildren.end())
       return;

   Safe_Release(iter->second);
   m_EffectChildren.erase(iter);
}

_int CEffect_Prefab::Get_Children_Count()
{
    if (m_EffectChildren.empty())
        return 0;

    return m_EffectChildren.size();
}

_wstring CEffect_Prefab::Get_Children_Tag(_int iIndex)
{
    _int iCheckIndex = 0;

    for (auto iter = m_EffectChildren.begin(); iter != m_EffectChildren.end();)
    {
        if (iCheckIndex == iIndex)
        {
            return iter->first;
        }
        else
        {
            ++iter;
            ++iCheckIndex;
        }
    }
}

CGameObject* CEffect_Prefab::Get_Children(_wstring ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return nullptr;

    return iter->second;
}

CEffect_Prefab* CEffect_Prefab::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Prefab* pInstance = new CEffect_Prefab(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Prefab");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Prefab::Clone(void* pArg)
{
    CEffect_Prefab* pInstance = new CEffect_Prefab(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CParticle");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Prefab::Free()
{
    __super::Free();

    //자식 삭제
    for (auto& Children : m_EffectChildren)
        Safe_Release(Children.second);

    m_EffectChildren.clear();
}
