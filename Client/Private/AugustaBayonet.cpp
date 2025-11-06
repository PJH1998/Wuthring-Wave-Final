#include "ClientPch.h"
#include "AugustaBayonet.h"
#include "AttackVolume.h"

CAugustaBayonet::CAugustaBayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaBayonet::CAugustaBayonet(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaBayonet::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaBayonet::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
	Ready_AttackVolumes();
	Register_AllNotifies(pDesc->strFolderPath);

    return S_OK;
}

void CAugustaBayonet::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);

	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Priority_Update(fTimeDelta);
}

void CAugustaBayonet::Update(_float fTimeDelta)
{
	// 호출 순서. Character Update -> Activate 상태라면-> WingUpdate(행렬 및 RigidBody 갱신) -> StateMachine Update 
	// -> m_pSocketMatrix에 뼈 행렬 포인터 전달. -> Animation 실행. -> 캐릭터 Update  종료
    CProp::Update(fTimeDelta);

    _matrix matWorld = XMLoadFloat4x4(&m_CombinedMatrix);
    m_pRigidbodyCom->Update_Rigidbody(matWorld, fTimeDelta);

	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);
}

void CAugustaBayonet::Late_Update(_float fTimeDelta)
{
	// Combined Matrix 
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

    CProp::Late_Update(fTimeDelta);


	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Late_Update(fTimeDelta);

    //m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
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
    m_pRigidbodyCom->Render();
	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG
}

void CAugustaBayonet::Activate(_bool IsActivate)
{
	CProp::Activate(IsActivate);
}

void CAugustaBayonet::Change_Volume(_uint iVolumeIdx)
{
	if (m_AttackVolumes[iVolumeIdx] == nullptr)
		return;

	m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];
}

void CAugustaBayonet::Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer)
{
}


void CAugustaBayonet::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{

}

void CAugustaBayonet::Ready_Components(const PROP_DESC* pDesc)
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
    RigidbodyDesc.fRadius = 0.3f;
    RigidbodyDesc.fHeight = 0.5f;
    RigidbodyDesc.eShape = SHAPE::CAPSULE;
    RigidbodyDesc.vPos = { 0.f, 0.f, 0.f };
    RigidbodyDesc.eType = EMotionType::Kinematic;
    RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ATTACK);
    RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::BODY;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->rigidBodyData.first)
        , pDesc->rigidBodyData.second, TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        CRASH("Rigidbody");
}

void CAugustaBayonet::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CAugustaBayonet::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaBayonet::Ready_AttackVolumes()
{
	// size 설정
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(2.f, 2.f, 2.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 200.f;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
			this->OnHitEnter(iLayer, pOther, Manifold);
	};

	// Attack용 만들기.
	m_AttackVolumes[VOLUME_ATTACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
		, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	// 끄고 켜기
	m_AttackVolumes[VOLUME_ATTACK]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_ATTACK]; // 기본.
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
    CProp::Free();

}
