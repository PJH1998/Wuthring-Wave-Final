#include "EditorPch.h"
#include "AnimationActor.h"
#include "Model.h"

CAnimationActor::CAnimationActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CAnimationActor::CAnimationActor(const CAnimationActor& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CAnimationActor::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CAnimationActor::Initialize_Clone(void* pArg)
{
    ANIMATION_ACTOR_DESC* pDesc = static_cast<ANIMATION_ACTOR_DESC*>(pArg);
    if (FAILED(CContainerObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eLevel;

    m_iShaderPath = pDesc->iShaderPath;

    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPostion), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
    
    _float3 vRadian = { 
        XMConvertToRadians(pDesc->vRotation.x),
        XMConvertToRadians(pDesc->vRotation.y),
        XMConvertToRadians(pDesc->vRotation.z) };
    m_pTransformCom->Quaternion(vRadian);

    // Model의 Dat Folder Path
    m_strModelDatPath = pDesc->strModelDatPath;


    if (FAILED(Ready_Components(pDesc)))
    {
        CRASH("Failed Ready_Components");
        return E_FAIL;
    }

    // Default는 0번 애니메이션 실행.
    m_strCurrentAnimation = m_pModelCom->Get_AnimationNames()[0];

    return S_OK;
}

void CAnimationActor::Priority_Update(_float fTimeDelta)
{
    CContainerObject::Priority_Update(fTimeDelta);
}

void CAnimationActor::Update(_float fTimeDelta)
{
    CContainerObject::Update(fTimeDelta);

    m_fTimeDelta = fTimeDelta;

    if (m_IsPlayAnimation)
        m_pModelCom->Play_Animation(m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, false);
}

void CAnimationActor::Late_Update(_float fTimeDelta)
{
    CContainerObject::Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CAnimationActor::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

        //if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
        //    return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_iShaderPath)))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }
    
}

void CAnimationActor::Render_Shadow()
{

}

#ifdef _DEBUG
const vector<_string>& CAnimationActor::Get_AnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_AnimationNames();
}

_float* CAnimationActor::Get_TrackPositionPtr(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_TrackPositionPtr(strAnimName);
}

_float CAnimationActor::Get_Duration(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(strAnimName);
}

// Notify에서 사용할 현재 선택된 애니메이션 이름
const _string& CAnimationActor::Get_CurrentAnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_strCurrentAnimation;
}

// Notify에서 사용할 현재 선택된 애니메이션의 최대 TrackPosition
const _float CAnimationActor::Get_CurrentAnimationDuration() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(m_strCurrentAnimation);
}

HRESULT CAnimationActor::Bind_Bone_to_GUI()
{
	_int iBoneIndex = 0;
    if(FAILED(m_pModelCom->Bind_Bone_to_GUI(iBoneIndex)))
        return E_FAIL;
    return S_OK;
}


// Notify에서 사용할 현재 선택된 애니메이션의 최대 프레임 정보?

void CAnimationActor::Set_TrackPosition(_float fTrackPosition)
{
    ASSERT_CRASH(m_pModelCom);
    // 어차피 현재거 설정하니까 매개변수로 가져올 필요가 없을 듯.
    m_pModelCom->Set_TrackPosition(m_strCurrentAnimation, fTrackPosition);

    // TrackPosition을 설정하면서 만약 Stop인 경우에도 확인할 수 있게 Play Animation을 실행합니다.
    if (!m_IsPlayAnimation)
        m_pModelCom->Play_Animation(m_strCurrentAnimation, m_fTimeDelta, &m_fTrackPosition, false);

}
void CAnimationActor::Set_PlayAnimation(_bool IsPlay)
{
    m_IsPlayAnimation = IsPlay;
}

// 폴더에 존재하는 모든 애니메이션 json을 읽어와서 등록합니다.
void CAnimationActor::Register_AllNotifies(const _string& strFolderPath)
{
    //m_pModelCom->Register_Notify(strFilePath);

    auto colliderCallback = [this](const _wstring& tag, bool active) {
        this->Collider_Active(tag, active); // 'this->'는 생략 가능
    };

    auto effectCallBack = [this]() {
        this->Effect_Active();
    };

    m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack);
    
}
void CAnimationActor::Collider_Active(const _wstring&, _bool)
{

}
void CAnimationActor::Effect_Active()
{
}
#endif

// 1. 행렬 
void CAnimationActor::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

HRESULT CAnimationActor::Ready_Components(const ANIMATION_ACTOR_DESC* pDesc)
{
    // Shader
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strShaderTag,
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
    {
        CRASH("Failed Ready_ComShader");
        return E_FAIL;
    }

    // Model
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(m_eCurLevel), pDesc->strModelTag,
        TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
    {
        CRASH("Failed Ready Com_Model");
        return E_FAIL;
    }
        

    return S_OK;
}

CGameObject* CAnimationActor::Clone(void* pArg)
{
    CAnimationActor* pInstance = new CAnimationActor(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CAnimationActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CAnimationActor* CAnimationActor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimationActor* pInstance = new CAnimationActor(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CAnimationActor");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAnimationActor::Free()
{
    CContainerObject::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);

}
