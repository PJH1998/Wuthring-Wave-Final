#include "ClientPch.h"
#include "AugustaBayonet.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"


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

    return S_OK;
}

void CAugustaBayonet::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);
	

}

void CAugustaBayonet::Update(_float fTimeDelta)
{
	// 호출 순서. Character Update -> Activate 상태라면-> WingUpdate(행렬 및 RigidBody 갱신) -> StateMachine Update 
	// -> m_pSocketMatrix에 뼈 행렬 포인터 전달. -> Animation 실행. -> 캐릭터 Update  종료
    CProp::Update(fTimeDelta);

	// Combined Matrix 
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

    _matrix matWorld = XMLoadFloat4x4(&m_CombinedMatrix);

	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			pAttackVolume->Update(fTimeDelta);
	}
}

void CAugustaBayonet::Late_Update(_float fTimeDelta)
{
	

    CProp::Late_Update(fTimeDelta);

	/*if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);*/
	
		


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
    //m_pRigidbodyCom->Render();
	//if (m_pMainAttackVolume->IsActivate())
	m_pMainAttackVolume->Render();
#endif // _DEBUG
}

void CAugustaBayonet::Activate(_bool IsActivate)
{
	CProp::Activate(IsActivate);
	m_pModelCom->Clear_Animation(m_strCurrentAnimName);

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

void CAugustaBayonet::Change_Volume(_uint iVolumeIdx)
{
	if ((m_AttackVolumes[iVolumeIdx] == nullptr) || (m_pMainAttackVolume == nullptr))
		return;

	// 교체.
	m_pMainAttackVolume->TriggerActivate(false);
	m_iVolumeIdx = iVolumeIdx;
	m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];
	
}

void CAugustaBayonet::Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer)
{
	if (m_AttackVolumes[iVolumeIdx] != nullptr)
		m_AttackVolumes[iVolumeIdx]->Change_Layer(eLayer);
}


void CAugustaBayonet::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 1. 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::AUGUSTA));

	if (nullptr == pAbility)
		return;


	switch (m_iVolumeIdx)
	{
	case VOLUME::VOLUME_ATTACK: // 기본 공격시 협주 게이지와 궁게이지 채우기
		pAbility->Add_Cost(COST_TYPE::COST1, 8.f);
		pAbility->Add_Cost(COST_TYPE::COST2, 5.f);
		pAbility->Add_Cost(COST_TYPE::COST5, 3.f);
		pAbility->Add_HarmonyGauge(4.f);
		break;
	case VOLUME::VOLUME_STRONG_ATTACK: // 강공 시 POINT 게이지와 궁 게이지 채우기.
		pAbility->Add_Cost(COST_TYPE::COST2, 10.f);
		pAbility->Add_Cost(COST_TYPE::COST5, 5.f);
		pAbility->Add_HarmonyGauge(8.f);
		break;
	}

	// 2. 타격감을 위한. Shake
	CAMERA_SHAKE ShakeDesc{};
	ShakeDesc.fDuration = 0.12f;
	ShakeDesc.fFrequency = 12.f;
	ShakeDesc.fAmplitude = 1.f;
	ShakeDesc.fFovKick = XMConvertToRadians(0.5f);
	ShakeDesc.vRotation = _float3(0.0f, 0.1f, 0.f);  // Pitch(x: 위아래), Yaw(y: 좌우), Roll(z: 0)
	m_pGameInstance->OnShake(ShakeDesc);

	//m_pGameInstance->Change_TimeRate(TEXT("Timer_60"), 0.8f);
	
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

 /*   CRigidbody::CAPSULEBODY_DESC RigidbodyDesc{};
    RigidbodyDesc.fRadius = 0.3f;
    RigidbodyDesc.fHeight = 0.5f;
    RigidbodyDesc.eShape = SHAPE::CAPSULE;
    RigidbodyDesc.vPos = { 0.f, 0.f, 0.f };
    RigidbodyDesc.eType = EMotionType::Kinematic;
    RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ATTACK);
    RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::BODY;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->rigidBodyData.first)
        , pDesc->rigidBodyData.second, TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
        CRASH("Rigidbody");*/
}

void CAugustaBayonet::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

	for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON);

	m_iShaderPath = ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON);
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

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP; // 장
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.pParenTransform = m_pParentTransform;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(1.5f, 1.5f, 1.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 200.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::ELEC;
	TriggerDesc.eDir = ATTACKVOULME_DIR::DEFAULT;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
			this->OnHitEnter(iLayer, pOther, Manifold);
	};

	// Attack용 만들기.
	m_AttackVolumes[VOLUME_ATTACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
		, PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	ASSERT_CRASH(m_AttackVolumes[VOLUME_ATTACK])
	m_AttackVolumes[VOLUME_ATTACK]->TriggerActivate(false);

	TriggerDesc.eLayer = COLLISIONLAYER::ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 2.f);
	TriggerDesc.fAttackDmg = 400.f;
	m_AttackVolumes[VOLUME_STRONG_ATTACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_STRONG_ATTACK])
	m_AttackVolumes[VOLUME_STRONG_ATTACK]->TriggerActivate(false);

	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(4.f, 4.f, 1.5f); // 평면으로 크게
	TriggerDesc.fAttackDmg = 600.f;
	m_AttackVolumes[VOLUME_ULTI] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME_ULTI])
	m_AttackVolumes[VOLUME_ULTI]->TriggerActivate(false);

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
