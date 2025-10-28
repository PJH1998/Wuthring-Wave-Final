#include "ClientPch.h"
#include "AugustaBayonet.h"

CAugustaBayonet::CAugustaBayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CWeapon{ pDevice, pContext }
{
}

CAugustaBayonet::CAugustaBayonet(const CPartObject& Prototype)
    : CWeapon(Prototype )
{
}

HRESULT CAugustaBayonet::Initialize_Prototype()
{
    if (FAILED(CWeapon::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaBayonet::Initialize_Clone(void* pArg)
{
    WEAPON_DESC* pDesc = static_cast<WEAPON_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);

    return S_OK;
}

void CAugustaBayonet::Priority_Update(_float fTimeDelta)
{
    CWeapon::Priority_Update(fTimeDelta);
}

void CAugustaBayonet::Update(_float fTimeDelta)
{
    CWeapon::Update(fTimeDelta);

    // Augusta의 StateMachine에서 애니메이션실행?

    // Last :  Combined 행렬 초기화
    XMStoreFloat4x4(&m_CombinedMatrix,
        m_pTransformCom->Get_WorldMatrix() *
        XMLoadFloat4x4(m_pSocketMatrix) *
        m_pParentTransform->Get_WorldMatrix());

    //m_pRigidbodyCom->Update_Rigidbody(mat, fTimeDelta);
}

void CAugustaBayonet::Late_Update(_float fTimeDelta)
{
    CWeapon::Late_Update(fTimeDelta);

    //m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CAugustaBayonet::Render()
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

void CAugustaBayonet::Ready_Components(const WEAPON_DESC* pDesc)
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

    /*CRigidbody::RIGIDBODY_DESC RigidbodyDesc{};
    RigidbodyDesc.eShape = SHAPE::CAPSULE;
    RigidbodyDesc.vPos = { 0.f, 0.f, 0.f };
    RigidbodyDesc.eType = EMotionType::Kinematic;
    RigidbodyDesc.iLayer = 1;
    RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::BODY;
    

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->rigidBodyData.first)
        , pDesc->rigidBodyData.second, TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        CRASH("Rigidbody");*/
}

void CAugustaBayonet::Ready_Variables(const WEAPON_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CAugustaBayonet::Ready_Positions(const WEAPON_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaBayonet::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaBayonet* CAugustaBayonet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaBayonet* pInstance = new CAugustaBayonet(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaBayonet");
    }
    return pInstance;
}

CGameObject* CAugustaBayonet::Clone(void* pArg)
{
    CAugustaBayonet* pInstance = new CAugustaBayonet(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaBayonet");
    }
    return pInstance;
}

void CAugustaBayonet::Free()
{
    CWeapon::Free();
}
