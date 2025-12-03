#include "ClientPch.h"
#include "Levi_Augusta.h"
#include "AttackVolume.h"
#include "GameSystem.h"

CLevi_Augusta::CLevi_Augusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CLevi_Augusta::CLevi_Augusta(const CPartObject& Prototype)
    : CPartObject(Prototype )
{
}

HRESULT CLevi_Augusta::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CLevi_Augusta::Initialize_Clone(void* pArg)
{
    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

	LEVIAUG_DESC* pDesc = static_cast<LEVIAUG_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
	Ready_AttackVolumes(pDesc);

#ifdef _DEBUG
	m_vOffsetPos = pDesc->vOffsetPos;
	m_vOffsetRot = pDesc->vOffsetRadian;
#else
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(pDesc->vOffsetRadian.x, pDesc->vOffsetRadian.y, pDesc->vOffsetRadian.z),
		XMVectorSetW(XMLoadFloat3(&pDesc->vOffsetPos), 1.f));
	XMStoreFloat4x4(&m_OffsetMatrix, matOffset);
#endif // _DEBUG
	m_vBaseColor = _float4(0.1f, 0.1f, 0.1f, 1.f);
	m_pModelCom->Play_Animation_CPU("Burst01", 0.f, nullptr, false, false, false, false);
    return S_OK;
}

void CLevi_Augusta::Priority_Update(_float fTimeDelta)
{
	if (nullptr != m_pAttackVolume)
		m_pAttackVolume->Priority_Update(fTimeDelta);
	
}

void CLevi_Augusta::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(m_vOffsetRot.x, m_vOffsetRot.y, m_vOffsetRot.z), XMVectorSetW(XMLoadFloat3(&m_vOffsetPos), 1.f));
#else
	_matrix matOffset = XMLoadFloat4x4(&m_OffsetMatrix);
#endif // _DEBUG

	XMStoreFloat4x4(&m_CombinedMatrix, matOffset *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_CombinedMatrix));

	if (nullptr != m_pAttackVolume)
		m_pAttackVolume->Update(fTimeDelta);
	
}

void CLevi_Augusta::Late_Update(_float fTimeDelta)
{
	if (nullptr != m_pAttackVolume)
		m_pAttackVolume->Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CLevi_Augusta::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

		_bool HasNormal = { false };

		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }

#ifdef _DEBUG
	if (m_pAttackVolume->IsActivate())
		m_pAttackVolume->Render();
#endif // _DEBUG
}

void CLevi_Augusta::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	//LEVI_AUG_RESET* pDesc = static_cast<LEVI_AUG_RESET*>(pArg);
	m_pAttackVolume->TriggerActivate(false);
	//m_pAttackVolume->Change_Layer(pDesc->eLayer);
}

void CLevi_Augusta::Change_Volume(COLLISIONLAYER eLayer)
{
	if (m_pAttackVolume != nullptr)
		m_pAttackVolume->Change_Layer(eLayer);
}

void CLevi_Augusta::Attack_Active(_bool isActive)
{
	m_pAttackVolume->TriggerActivate(isActive);
}

void CLevi_Augusta::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
#ifdef _DEBUG
		cout << "On Hit! (Levi Augusta)" << endl;
#endif // _DEBUG
	}

}

void CLevi_Augusta::Ready_Components(const LEVIAUG_DESC* pDesc)
{
    // 1. Components
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->eLevel)
        , TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->eLevel)
        , TEXT("Prototype_Component_Model_Augusta_SkillWeapon"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");
}

void CLevi_Augusta::Ready_Variables(const LEVIAUG_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;
}

void CLevi_Augusta::Ready_AttackVolumes(const LEVIAUG_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.vExtent = _float3(1.7f, 0.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(1.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = pDesc->fAttackDmg;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::DARK;
	TriggerDesc.eDir = ATTACKVOULME_DIR::DEFAULT;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	m_pAttackVolume = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	m_pAttackVolume->TriggerActivate(false);
}

void CLevi_Augusta::Bind_Resources()
{
	//if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
	//	CRASH("Failed Bind Matrix");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
		CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
	if (FAILED(m_pShaderCom->Bind_Value("g_vBaseColor", &m_vBaseColor, sizeof(_float4))))
		CRASH("Failed Base Color");
}

CLevi_Augusta* CLevi_Augusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevi_Augusta* pInstance = new CLevi_Augusta(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CLevi_Augusta");
    }
    return pInstance;
}

CGameObject* CLevi_Augusta::Clone(void* pArg)
{
    CLevi_Augusta* pInstance = new CLevi_Augusta(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CLevi_Augusta");
    }
    return pInstance;
}

void CLevi_Augusta::Free()
{
    __super::Free();
	Safe_Release(m_pAttackVolume);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
