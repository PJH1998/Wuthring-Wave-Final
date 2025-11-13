#include "ClientPch.h"
#include "GalbrenaGun.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CGalbrenaGun::CGalbrenaGun(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CGalbrenaGun::CGalbrenaGun(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CGalbrenaGun::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CGalbrenaGun::Initialize_Clone(void* pArg)
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

void CGalbrenaGun::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);

	// 1. Attack Volume 갱신
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Priority_Update(fTimeDelta);
}

void CGalbrenaGun::Update(_float fTimeDelta)
{
    CProp::Update(fTimeDelta);

    // 1. Combine 행렬 계산
    XMStoreFloat4x4(&m_CombinedMatrix,
        m_pTransformCom->Get_WorldMatrix() *
        XMLoadFloat4x4(m_pSocketMatrix) *
        m_pParentTransform->Get_WorldMatrix());

	// 2. 어택 볼륨 업데이트
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);
    
}

void CGalbrenaGun::Late_Update(_float fTimeDelta)
{

    CProp::Late_Update(fTimeDelta);

	// Attack Volume 갱신.
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Late_Update(fTimeDelta);


    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CGalbrenaGun::Render()
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
	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG
}

void CGalbrenaGun::Activate(_bool IsActivate)
{
	CProp::Activate(IsActivate);
}

void CGalbrenaGun::Change_Volume(_uint iVolumeIdx)
{
	if ((m_AttackVolumes[iVolumeIdx] == nullptr) || (m_pMainAttackVolume == nullptr))
		return;

	// 교체.
	m_pMainAttackVolume->TriggerActivate(false);
	m_iVolumeIdx = iVolumeIdx;
	m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];
}

void CGalbrenaGun::Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer)
{
	if (m_AttackVolumes[iVolumeIdx] != nullptr)
		m_AttackVolumes[iVolumeIdx]->Change_Layer(eLayer);
}

void CGalbrenaGun::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 1. 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::GALBRENA));

	if (nullptr == pAbility)
		return;

	switch (m_iVolumeIdx)
	{
	case VOLUME::VOLUME_ATTACK: // 기본 공격시 공명 게이지와 궁게이지 채우기
		pAbility->Add_HarmonyGauge(4.f); // 공명 게이지 채우기.
		pAbility->Add_Cost(COST_TYPE::COST1, 3.f); // 궁 ULTI
		break;
	}
}

void CGalbrenaGun::Ready_Components(const PROP_DESC* pDesc)
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

void CGalbrenaGun::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CGalbrenaGun::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CGalbrenaGun::Ready_AttackVolumes()
{
	// size 설정
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(1.2f, 1.2f, 0.5f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 150.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::DARK;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	m_AttackVolumes[VOLUME_ATTACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_ATTACK])
	m_AttackVolumes[VOLUME_ATTACK]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_ATTACK]; // 기본.
}

void CGalbrenaGun::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CGalbrenaGun* CGalbrenaGun::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CGalbrenaGun* pInstance = new CGalbrenaGun(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CGalbrenaGun");
    }
    return pInstance;
}

CGameObject* CGalbrenaGun::Clone(void* pArg)
{
    CGalbrenaGun* pInstance = new CGalbrenaGun(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CGalbrenaGun");
    }
    return pInstance;
}

void CGalbrenaGun::Free()
{
    CProp::Free();
}
