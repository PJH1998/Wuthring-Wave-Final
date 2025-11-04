#include "ClientPch.h"
#include "AugustaGriffon.h"
#include "Client_Debug.h"

CAugustaGriffon::CAugustaGriffon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaGriffon::CAugustaGriffon(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaGriffon::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaGriffon::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
    return S_OK;
}

void CAugustaGriffon::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);

#ifdef _DEBUG
    ClientDebug::Edit_TransformRotate(m_pTransformCom);
#endif // _DEBUG
}

void CAugustaGriffon::Update(_float fTimeDelta)
{
    CProp::Update(fTimeDelta);

    XMStoreFloat4x4(&m_CombinedMatrix,
        m_pTransformCom->Get_WorldMatrix() *
        XMLoadFloat4x4(m_pSocketMatrix) *
        m_pParentTransform->Get_WorldMatrix());
}

void CAugustaGriffon::Late_Update(_float fTimeDelta)
{
    CProp::Late_Update(fTimeDelta);

    //m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);

    /*if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;*/
    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CAugustaGriffon::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();


    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

        m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0);

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }

#ifdef _DEBUG
    //m_pRigidbodyCom->Render();
#endif // _DEBUG
}

void CAugustaGriffon::Activate(_bool IsActive)
{
	CProp::Activate(IsActive);
	// 한번 실행시킨다. => 1Frame 위에서 놀고있게
	CProp::Play_Animation("SA1Shouwangjiu_Fly_Loop", 0.f, nullptr);
  
}

void CAugustaGriffon::Ready_Components(const PROP_DESC* pDesc)
{
    // 1. Components
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
        , pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
        CRASH("Compute Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
        , pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");

    //CRigidbody::CAPSULEBODY_DESC RigidbodyDesc{};
    //RigidbodyDesc.fRadius = 0.3f;
    //RigidbodyDesc.fHeight = 0.5f;
    //RigidbodyDesc.eShape = SHAPE::CAPSULE;
    //RigidbodyDesc.vPos = { 0.f, 0.f, 0.f };
    //RigidbodyDesc.eType = EMotionType::Kinematic;
    //RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ATTACK);
    //RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::BODY;

    //if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->rigidBodyData.first)
    //    , pDesc->rigidBodyData.second, TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
    //    CRASH("Rigidbody");
}

void CAugustaGriffon::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CAugustaGriffon::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaGriffon::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaGriffon* CAugustaGriffon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaGriffon* pInstance = new CAugustaGriffon(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaGriffon");
    }
    return pInstance;
}

CGameObject* CAugustaGriffon::Clone(void* pArg)
{
    CAugustaGriffon* pInstance = new CAugustaGriffon(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaGriffon");
    }
    return pInstance;
}

void CAugustaGriffon::Free()
{
    CProp::Free();
}
