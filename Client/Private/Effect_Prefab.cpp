#include "ClientPch.h"
#include "Effect_Prefab.h"
#include "Particle.h"
//#include "Effect_Mesh.h"
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
    m_strBoneTag = pDesc->strBoneTag;

    for (size_t i = 0; i < pDesc->ChildrenCount; i++)
    {
       _wstring strChildrenTag = pDesc->FrameDesc[i].strChildrenTag;
       EFFECT_TYPE eType = pDesc->FrameDesc[i].eChildrenType;

       Add_Children(strChildrenTag, eType, pDesc->CurrentLevel);

       m_vFrames.push_back(pDesc->FrameDesc[i]);
    }
    //m_vLifeTime = pDesc->vLifeTime;
    //프리팹 라이프 타임 필요할까 ?
    m_vLifeTime.y = 10.f;
    m_vLifeTime.x = 0.f;

    m_isActivate = false;
  
    XMStoreFloat4x4(&m_SpawnMatrix, XMMatrixIdentity());

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
            _bool IsActivated = true;
            //자식 활성화
            Get_Children(Frame.strChildrenTag)->Reset(XMLoadFloat4x4(&m_SpawnMatrix), &IsActivated);

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
        Reset_Prefab_Info();
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

void CEffect_Prefab::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
    //여기서 플레이어 월드매트릭스랑, 모델 주소 넘겨받아야함.
    if (CModel* pModel = static_cast<CModel*>(pArg))
    {
        //혹시모를 이전 프리팹 값 있으면 리셋 진행.
        Reset_SpawnMatrix();
        Reset_Prefab_Info();

        _float4x4 PlayerMatrix = {};
        XMStoreFloat4x4(&PlayerMatrix, WorldMatrix);

        //프리팹이 뼈에 붙을 이름을 알고 있게 해줘야함.
        _float4x4 BoneMatrix = *pModel->Get_BoneMatrixPtr(m_strBoneTag.c_str());

        //위에서 꺼낸 본 매트릭스 그때 위치 갱신정보와 모델의 월드매트릭스 전달.
        Set_SpawnMatrix(PlayerMatrix, BoneMatrix);

       

        m_isActivate = true;
    }
}

void CEffect_Prefab::Add_Children(const _wstring& ChildrenTag, EFFECT_TYPE eType, _uint CurrentLevel)
{
    CGameObject* pChildren = {};
    _wstring strChildrenProtoTag = TEXT("Prototype_GameObject_");
    _wstring strChildrenNameTag = ChildrenTag;

    switch (eType)
    {
    case EFFECT_TYPE::PARTICLE:

        strChildrenProtoTag += TEXT("Particle_");
        strChildrenProtoTag += strChildrenNameTag;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
        break;
   /* case EFFECT_TYPE::MESH:
        pMeshDesc = static_cast<CEffect_Mesh::EFFECTMESH_DESC*>(pArg);
        strChildrenTag = pMeshDesc->strMyTag;

        FrameDesc.strChildrenTag = pMeshDesc->strMyTag;
        FrameDesc.eChildrenType = eType;

         strDefaultTag += TEXT("FXMesh_");
        strDefaultTag += strChildrenTag;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"), PROTOTYPE::GAMEOBJECT, pArg));
        break;*/
    case EFFECT_TYPE::TRAIL:
        strChildrenProtoTag += TEXT("TrailMesh_");
        strChildrenProtoTag += strChildrenNameTag;

        pChildren = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(CurrentLevel, strChildrenProtoTag, PROTOTYPE::GAMEOBJECT));
        break;
    case EFFECT_TYPE::END:
        CRASH("Failed Children Desc");
        break;
    }

    if (pChildren == nullptr)
        return;

    m_EffectChildren.emplace(strChildrenNameTag, pChildren);
}

CGameObject* CEffect_Prefab::Get_Children(_wstring ChildrenTag)
{
    auto iter = m_EffectChildren.find(ChildrenTag);

    if (iter == m_EffectChildren.end())
        return nullptr;

    return iter->second;
}

void CEffect_Prefab::Set_SpawnMatrix(_float4x4 PlayerMatrix, _float4x4 BoneMatrix)
{
    // 
    //_vector vPos = XMVectorSet(PlayerMatrix._41, PlayerMatrix._42, PlayerMatrix._43, 1.f);
    //_matrix PlayerPosMatrix = XMMatrixTranslationFromVector(vPos);

    XMStoreFloat4x4(&m_SpawnMatrix,
      XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&PlayerMatrix));
}

void CEffect_Prefab::Reset_SpawnMatrix()
{
    XMStoreFloat4x4(&m_SpawnMatrix, XMMatrixIdentity());
}

void CEffect_Prefab::Reset_Prefab_Info()
{
    for (auto& Frame : m_vFrames)
    {
        Frame.bActivated = false;
    }
    m_fCurrentTime = 0.f;

    m_vLifeTime.x = 0.f;

    _matrix DefaultMat = XMLoadFloat4x4(&m_SpawnMatrix);

    //초기설정으로 되돌리기 처리만
    _bool Activate = false;
    for (auto& Children : m_EffectChildren)
        Children.second->Reset(DefaultMat, &Activate);
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
