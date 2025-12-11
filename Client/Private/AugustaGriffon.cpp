#include "ClientPch.h"
#include "AugustaGriffon.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

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
	Ready_AttackVolumes();

	m_fMaxDissolveTime = 0.5f;
    return S_OK;
}

void CAugustaGriffon::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


    CProp::Priority_Update(fTimeDelta);

	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Priority_Update(fTimeDelta);
	}

	
}


void CAugustaGriffon::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

    CProp::Update(fTimeDelta);

	// 1. Combine 행렬 계산
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());
	
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Update(fTimeDelta);
	}
}

void CAugustaGriffon::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

    CProp::Late_Update(fTimeDelta);


	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Late_Update(fTimeDelta);
	}
  
    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
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
		/*_bool HasNormal = { false };

		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");*/

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        //if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
        //    CRASH("Ready Shader Begin Failed");
        if (FAILED(m_pShaderCom->Begin(ENUM_CLASS(SHADER_PROPANIMMESH::NORMAL_TEX))))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }

#ifdef _DEBUG
    //m_pRigidbodyCom->Render();
	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG
}

void CAugustaGriffon::Render_Shadow()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_PROPANIMMESH::SHADOW));

		m_pModelCom->Render(i);
	}
}

void CAugustaGriffon::Activate(_bool IsActivate)
{
	CProp::Activate(IsActivate);
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

void CAugustaGriffon::Change_Volume(_uint iVolumeIdx)
{
	if ((m_AttackVolumes[iVolumeIdx] == nullptr) || (m_pMainAttackVolume == nullptr))
		return;

	// 교체.
	m_pMainAttackVolume->TriggerActivate(false);
	m_iVolumeIdx = iVolumeIdx;
	m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];
	
}

void CAugustaGriffon::Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer)
{
	if (m_AttackVolumes[iVolumeIdx] != nullptr)
		m_AttackVolumes[iVolumeIdx]->Change_Layer(eLayer);
}

void CAugustaGriffon::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 2. 타격감을 위한. Shake
	CAMERA_SHAKE ShakeDesc{};
	ShakeDesc.fDuration = 0.12f;
	ShakeDesc.fFrequency = 12.f;
	ShakeDesc.fAmplitude = 1.f;
	ShakeDesc.fFovKick = XMConvertToRadians(0.5f);
	ShakeDesc.vRotation = _float3(0.0f, 0.1f, 0.f);  // Pitch(x: 위아래), Yaw(y: 좌우), Roll(z: 0)
	m_pGameInstance->OnShake(ShakeDesc);
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

}

void CAugustaGriffon::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

	for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_PROPANIMMESH::NORMAL_TEX);
}

void CAugustaGriffon::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaGriffon::Ready_AttackVolumes()
{
	// size 설정
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장비
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 2.f); // x, z 평면 크게 , y축 작게 나오는 범위 찾기.
	TriggerDesc.vOffsetPos = _float3(0.5f, -1.5f, 0.f); // 조금 앞으로?
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 300.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::ELEC;
	TriggerDesc.eDir = ATTACKVOULME_DIR::DEFAULT;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	// Attack용 만들기.
	m_AttackVolumes[VOLUME_STRIKE] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	ASSERT_CRASH(m_AttackVolumes[VOLUME_STRIKE])
	m_pMainAttackVolume = m_AttackVolumes[VOLUME_STRIKE];
	m_pMainAttackVolume->TriggerActivate(false);
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
