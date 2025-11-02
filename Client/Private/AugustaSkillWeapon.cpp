#include "ClientPch.h"
#include "AugustaSkillWeapon.h"

CAugustaSkillWeapon::CAugustaSkillWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaSkillWeapon::CAugustaSkillWeapon(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaSkillWeapon::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaSkillWeapon::Initialize_Clone(void* pArg)
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

void CAugustaSkillWeapon::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);
}

void CAugustaSkillWeapon::Update(_float fTimeDelta)
{
    CProp::Update(fTimeDelta);

    // Augusta StateMachine

	// Last :  Combined 
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

    _matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);
    //m_pRigidbodyCom->Update_Rigidbody(mat, fTimeDelta);
}

void CAugustaSkillWeapon::Late_Update(_float fTimeDelta)
{


    CProp::Late_Update(fTimeDelta);

    //m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CAugustaSkillWeapon::Render()
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
    m_pRigidbodyCom->Render();
#endif // _DEBUG
}

void CAugustaSkillWeapon::Activate(_bool IsActive)
{
    SetActivate(IsActive);

    // Griffon�� ��쿡�� ��ġ�� �ʱ�ȭ���ش�?
    /*m_fTrackPosition = 0.f;
    _matrix mat = XMMatrixIdentity();
    m_pTransformCom->Set_WorldMatrix(mat);*/

    if (IsActive)
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
    else
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::ATTACK));
}

void CAugustaSkillWeapon::Ready_Components(const PROP_DESC* pDesc)
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

    CRigidbody::CAPSULEBODY_DESC RigidbodyDesc{};
    RigidbodyDesc.fRadius = 0.2f;
    RigidbodyDesc.fHeight = 0.6f;
    RigidbodyDesc.eShape = SHAPE::CAPSULE;
    RigidbodyDesc.vPos = { 0.f, 0.f, 0.f };
    RigidbodyDesc.eType = EMotionType::Kinematic;
    RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ATTACK);
    RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::BODY;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->rigidBodyData.first)
        , pDesc->rigidBodyData.second, TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        CRASH("Rigidbody");
}

void CAugustaSkillWeapon::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CAugustaSkillWeapon::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaSkillWeapon::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaSkillWeapon* CAugustaSkillWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaSkillWeapon* pInstance = new CAugustaSkillWeapon(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaSkillWeapon");
    }
    return pInstance;
}

CGameObject* CAugustaSkillWeapon::Clone(void* pArg)
{
    CAugustaSkillWeapon* pInstance = new CAugustaSkillWeapon(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaSkillWeapon");
    }
    return pInstance;
}

void CAugustaSkillWeapon::Free()
{
    CProp::Free();
}
