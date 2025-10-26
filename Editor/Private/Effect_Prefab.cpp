#include "Editorpch.h"
#include "Effect_Prefab.h"
#include "Particle.h"
#include "Effect_Mesh.h"
#include "Trail_Mesh.h"

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
    PREFAB_DESC* pDesc = static_cast<PREFAB_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    m_strMyTag = pDesc->strPrefabTag;
    //m_vLifeTime = pDesc->vLifeTime;

    //일단 프리팹 라이프타임 15초로
    m_vLifeTime.y = 10.f;

   // Root_Test();

    m_isActivate = false;

    return S_OK;
}

void CEffect_Prefab::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_fCurrentTime += fTimeDelta;

    for (auto& Frame : m_vFrames)
    {
        if (Frame.fActivateTime <= m_fCurrentTime && !Frame.bActivated)
        {
            //자식 활성화
            Get_Children(Frame.strChildrenTag)->SetActivate(true);

            Frame.bActivated = true;
        }
    }

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Priority_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    if (m_vLifeTime.x >= m_vLifeTime.y)
    {
        m_isActivate = false;
    }
    else
        m_vLifeTime.x += fTimeDelta;

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Update(fTimeDelta);
    }
}

void CEffect_Prefab::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    for (auto& Children : m_EffectChildren)
    {
        if (Children.second->IsActivate())
            Children.second->Late_Update(fTimeDelta);
    }
}

void CEffect_Prefab::Render()
{

}

void CEffect_Prefab::Add_Children(void* pArg, EFFECT_TYPE eType)
{
    CGameObject* pChildren = {};
    _wstring strChildrenTag = {};
    
    //설정할 자식들 Desc 미리 선언
    CParticle::PARTICLE_DESC* pParticleDesc = {};
    CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc = {};
    CTrail_Mesh::TRAILMESH_DESC* pTrailDesc = {};

    //프리팹 프레임에 미리 추가.
    FRAME_DESC FrameDesc = {};

    switch (eType)
    {
    case EFFECT_TYPE::PARTICLE:
        pParticleDesc = static_cast<CParticle::PARTICLE_DESC*>(pArg);
        strChildrenTag = pParticleDesc->strMyTag;

        FrameDesc.strChildrenTag = pParticleDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        if (pParticleDesc->IsRootOn)
            pParticleDesc->RootMatrix = m_pRootMatirx;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::MESH:
        pMeshDesc = static_cast<CEffect_Mesh::EFFECTMESH_DESC*>(pArg);
        strChildrenTag = pMeshDesc->strMyTag;

        FrameDesc.strChildrenTag = pMeshDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        if (pMeshDesc->IsRootOn)
            pMeshDesc->RootMatrix = m_pRootMatirx;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::TRAIL:
        pTrailDesc = static_cast<CTrail_Mesh::TRAILMESH_DESC*>(pArg);
        strChildrenTag = pTrailDesc->strMyTag;

        FrameDesc.strChildrenTag = pTrailDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

        if (pTrailDesc->IsRootOn)
            pTrailDesc->RootMatrix = m_pRootMatirx;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_TrailMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;
    case EFFECT_TYPE::END:
        CRASH("Failed Children Desc");
        break;
    }

    if (pChildren == nullptr)
        return;

    //활성화 한번
    Reset_Prefab_Info();
    _matrix DefaultMat = {};
    pChildren->Reset(DefaultMat, nullptr);

    m_EffectChildren.emplace(strChildrenTag, pChildren);
    m_vFrames.push_back(FrameDesc);

}

void CEffect_Prefab::Remove_Children(_wstring& ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return;

    Safe_Release(iter->second);
    m_EffectChildren.erase(iter);

    for (auto iterFrame = m_vFrames.begin(); iterFrame != m_vFrames.end(); )
    {
        if (iterFrame->strChildrenTag == ChildrenTag)
            iterFrame = m_vFrames.erase(iterFrame);
        else
            ++iterFrame;
    }
}

void CEffect_Prefab::Root_Test()
{
   CModel* pModel = static_cast<CModel*>(m_pGameInstance->Get_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Layer_Actor"), 0, TEXT("Com_Model")));

   m_pRootMatirx = pModel->Get_BoneMatrixPtr("Bone_Skirt051_M");
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

void CEffect_Prefab::Set_FrameDesc(FRAME_DESC* pFrameDesc)
{
    for (auto& Frame : m_vFrames)
    {
        if (pFrameDesc->strChildrenTag == Frame.strChildrenTag)
        {
            Frame.fActivateTime = pFrameDesc->fActivateTime;
            return;
        }
    }
}

void CEffect_Prefab::Reset_Prefab_Info()
{
    for (auto& Frame : m_vFrames)
    {
        Frame.bActivated = false;
    }
    m_fCurrentTime = 0.f;

    //일단처리
    m_vLifeTime.x = 0.f;
    m_isActivate = true;

    _matrix DefaultMat = {};

    for (auto& Children : m_EffectChildren)
        Children.second->Reset(DefaultMat, nullptr);
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

    for (auto& Children : m_EffectChildren)
        Safe_Release(Children.second);

    m_EffectChildren.clear();
}
