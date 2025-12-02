#include "ClientPch.h"
#include "AugustaBurstWeapon.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CAugustaBurstWeapon::CAugustaBurstWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaBurstWeapon::CAugustaBurstWeapon(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaBurstWeapon::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaBurstWeapon::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
	Ready_AttackVolumes();

    return S_OK;
}

void CAugustaBurstWeapon::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);

	if (m_IsAnimationEnd)
		m_isActivate = false;

	/*if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Priority_Update(fTimeDelta);*/
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Priority_Update(fTimeDelta);
	}
}

void CAugustaBurstWeapon::Update(_float fTimeDelta)
{
    CProp::Update(fTimeDelta);

    // Augusta StateMachine

	// Last :  Combined 
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

    _matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);

	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Update(fTimeDelta);
	}
}

void CAugustaBurstWeapon::Late_Update(_float fTimeDelta)
{
    CProp::Late_Update(fTimeDelta);

	//if (nullptr != m_pMainAttackVolume)
	//	m_pMainAttackVolume->Late_Update(fTimeDelta);


	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Late_Update(fTimeDelta);
	}

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CAugustaBurstWeapon::Render()
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
	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG
}

void CAugustaBurstWeapon::Activate(_bool IsActivate)
{
    SetActivate(IsActivate);

	m_pModelCom->Clear_Animation(m_strCurrentAnimName); // 애니메이션 클리어

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	if (false == IsActivate)
	{
		_matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Common_Weapon"), mat, &effecInfo);
		m_pMainAttackVolume->TriggerActivate(false); // 비활성화
	}
}

void CAugustaBurstWeapon::Change_Volume(_uint iVolumeIdx)
{
	if ((m_AttackVolumes[iVolumeIdx] == nullptr) || (m_pMainAttackVolume == nullptr))
		return;

	// 교체.
	m_pMainAttackVolume->TriggerActivate(false);
	m_iVolumeIdx = iVolumeIdx;
	m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];
}

void CAugustaBurstWeapon::Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer)
{
	if (m_AttackVolumes[iVolumeIdx] != nullptr)
		m_AttackVolumes[iVolumeIdx]->Change_Layer(eLayer);
}

void CAugustaBurstWeapon::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::AUGUSTA));

	if (nullptr == pAbility)
		return;
}

void CAugustaBurstWeapon::Ready_Components(const PROP_DESC* pDesc)
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
}

void CAugustaBurstWeapon::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

	for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON);
}

void CAugustaBurstWeapon::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaBurstWeapon::Ready_AttackVolumes()
{
	m_AttackVolumes.resize(VOLUME_END);


	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 3.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 200.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::ELEC;
	TriggerDesc.eDir = ATTACKVOULME_DIR::DEFAULT;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	// Burst 궁 켰을때 평타.
	m_AttackVolumes[VOLUME_SWORD_ATTACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(4.f, 4.f, 2.f);

	// 궁극기용도.
	ASSERT_CRASH(m_AttackVolumes[VOLUME_SWORD_ATTACK])
		m_AttackVolumes[VOLUME_SWORD_ATTACK]->TriggerActivate(false);

	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(30.f, 30.f, 30.f); // 3차원 크으게
	TriggerDesc.fAttackDmg = 1500.f;
	m_AttackVolumes[VOLUME_SWORD_ULTI] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_SWORD_ULTI])
		m_AttackVolumes[VOLUME_SWORD_ULTI]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_SWORD_ATTACK]; // 기본.
}

void CAugustaBurstWeapon::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaBurstWeapon* CAugustaBurstWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaBurstWeapon* pInstance = new CAugustaBurstWeapon(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaBurstWeapon");
    }
    return pInstance;
}

CGameObject* CAugustaBurstWeapon::Clone(void* pArg)
{
    CAugustaBurstWeapon* pInstance = new CAugustaBurstWeapon(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaBurstWeapon");
    }
    return pInstance;
}

void CAugustaBurstWeapon::Free()
{
    CProp::Free();
}
