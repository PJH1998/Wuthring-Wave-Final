#include "ClientPch.h"
#include "SequenceAugusta.h"
#include "Player.h"
#include "SpringCamera.h"

#include "SequenceAugustaFactory.h"
#include "Collider.h"

#include "AugustaBayonet.h"
#include "AttackVolume.h"
#include "GameSystem.h"


CSequenceAugusta::CSequenceAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CSequenceAugusta::CSequenceAugusta(const CSequenceAugusta& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CSequenceAugusta::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CSequenceAugusta::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CCharacter::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eCurLevel;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
    Ready_PartObjects(pDesc); // Parts 추가.
	Ready_AttackVolumes();
    Register_AllNotifies(pDesc->strFolderPath);

	CSequenceAugustaFactory::Register_States(m_pStateMachineCom, this);
	
	// 비활성화. 
    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());

    return S_OK;
}

void CSequenceAugusta::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	// 1. Parts 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Priority_Update(fTimeDelta);
	}

    // 2. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();

	// 3. 몬스터가 있다면?
	Update_TargetDistance(fTimeDelta);

	// Dissolve 체크.
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));

	// 5. Dissovle 체크
	if (IsDissolve)
	{
		if (m_fDissolveTimer <= m_fMaxDissolveTime)
			m_fDissolveTimer += fTimeDelta;
		else
		{
			m_isActivate = false;
			Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
		}
	}
}

void CSequenceAugusta::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

	
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));

	if (!IsDissolve)
	{
		// 2. 상태 머신 갱신
		m_pStateMachineCom->Update(fTimeDelta); // 여기서 Weapon이나 Parts의 갱신을 해야함.. => 여기서 Play_Animation 실행됨.
		// 3. Physcis 업데이트
		Update_Physics(fTimeDelta);
	}

	// 5. 파츠 갱신.?
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Update(fTimeDelta);
	}


	// 6. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);

}
void CSequenceAugusta::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

    // 1. 파츠 갱신
    for (auto& pPart : m_PartObjects)
    {
        if (pPart.second->IsActivate())
            pPart.second->Late_Update(fTimeDelta);
    }

	// 2. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Late_Update(fTimeDelta);

	// 3. QTE인 경우 Collider 갱신하지 않습니다.?
	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_IsVisible)
	{
		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
			return;

		if (!m_IsOutLineVisible)
		{
			if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE, this)))
				return;
		}

		

		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
			return;
	}
}

void CSequenceAugusta::Render()
{
    Bind_Resources();

	// 1. Dissolve 체크.
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		_float fDissolveRate = (m_fDissolveTimer / m_fMaxDissolveTime);
		if (FAILED(m_pShaderCom->Bind_Value("g_fDissolveRate", &fDissolveRate, sizeof(_float))))
			CRASH("Failed Bind Dissolve Rate");

		if (FAILED(m_pShaderCom->Bind_Value("g_vDissolveColor", &m_vDissolveColor, sizeof(_float4))))
			CRASH("Ready EnergyColor");

		if (FAILED(m_pShaderCom->Bind_Value("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
			CRASH("Ready EmissiveIntensity")
	}

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
			return;

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

		m_pShaderCom->UndBind_All_VS_SRV();
    }

#ifdef _DEBUG
	m_pColliderCom->Render();

	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG

}

void CSequenceAugusta::Render_OutLine()
{
	Bind_Resources();

	_uint iNumMeshes = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (i == 5)	//Cloths
			continue;

		_bool HasNormal = { false };

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::OUNTLINE))))
			CRASH("Ready Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");
	}
}

void CSequenceAugusta::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::SHADOW));

		m_pModelCom->Render(i);
	}
}

// AnimName이 같은걸로 매핑되어있음.
void CSequenceAugusta::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    switch (iPartType)
    {
	case PART_BAYONET:
		m_pBayonet->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
    default:
        break;
    }
}

void CSequenceAugusta::PartActivate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
	case PART_BAYONET:
		m_pBayonet->Activate(IsActive);
		break;
    default:
        break;
    }
}

void CSequenceAugusta::Part_VolumeChange(_uint iPartType, _uint iVolumeIdx)
{
	switch (iPartType)
	{
	case PART_BAYONET:
		m_pBayonet->Change_Volume(iVolumeIdx);
		break;
	}
}

void CSequenceAugusta::Part_VolumeActivate(_uint iPartType, _bool IsActive)
{
	switch (iPartType)
	{
	case PART_BAYONET:
		m_pBayonet->Volume_Activate(IsActive); // MainVolume 켜기
		break;
	}
}

void CSequenceAugusta::Clear_PartAnimation(_uint iPartType, const _string& strAnimName)
{
    switch (iPartType)
    {
	case PART_BAYONET:
		m_pBayonet->Clear_Animation(strAnimName);
		break;
    default:
        break;
    }

}

void CSequenceAugusta::Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName)
{
    ASSERT_CRASH(m_pModelCom);
    
    const _float4x4* pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());
    if (nullptr == pSocketMatrix)
        pSocketMatrix = &m_MatrixIdentity;
    
    switch (iPartType)
    {
	case PART_BAYONET:
		m_pBayonet->Set_SocketMatrix(pSocketMatrix);
		break;
    }
}

void CSequenceAugusta::Sync_Position()
{
    m_pColliderCom->Sync_Position(m_pTransformCom);
}



#pragma region NOTIFY
void CSequenceAugusta::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{

	_wstring var1, var2, var3;
	wstringstream wss(wStrColliderTag);
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)
	_uint iVolumeIdx = {  };

	if (var1 == TEXT("Main"))
		m_pMainAttackVolume->TriggerActivate(IsActive);


	if (var1 == TEXT("Bayonet"))
	{
		if (var2 == TEXT("ATK"))
			iVolumeIdx = CAugustaBayonet::VOLUME::VOLUME_ATTACK;
		if (var2 == TEXT("STRATK"))
			iVolumeIdx = CAugustaBayonet::VOLUME::VOLUME_STRONG_ATTACK;

		m_pBayonet->Change_Volume(iVolumeIdx);
		if (var3 == TEXT("ATTACK"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		else if (var3 == TEXT("KNOCKBACK"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var3 == TEXT("SKILL"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);

		m_pBayonet->Volume_Activate(IsActive);
	}
}

void CSequenceAugusta::Effect_Active(const _wstring& wStrEffectTag)
{
    if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
        return;

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &effecInfo);
}
void CSequenceAugusta::Object_Func(const _wstring& wStrObjectTag)
{
	// 3개의 변수 준비
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);

	// std::getline을 사용하여 L'|' 구분자를 만날 때까지 읽어 변수에 저장합니다.
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)

	// GalbrenaWing|Bone
	// 자르는거야.
	if (var1 == TEXT("StateDelay")) // 애니메이션 State의 속도를 Delay 시킵니다.
	{
		m_fStateTimeRate = stof(var2);
		m_fStateDelayTimer = stof(var3);
		Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::STATE_DELAY));
	}
	else if (var1 == TEXT("Sound"))
		Process_PlaySound(wStrObjectTag); // Character 함수.

}
void CSequenceAugusta::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	
}

#pragma endregion

#pragma region 4. EVENT
void CSequenceAugusta::Bind_ChangeEffect()
{
	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();

	m_pGameInstance->Spawn_PoolingObject(TEXT("Common_SwapEffect"), matWorld, &effecInfo);
}
void CSequenceAugusta::Bind_DissolveTimer()
{
	Add_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	m_fDissolveTimer = 0.f;
}
void CSequenceAugusta::Bind_DefaultShaderPath()
{
	// 기본 Shader Path
	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::AUGUSTA);
}
void CSequenceAugusta::Bind_DissolveShaderPath()
{
	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::DISSOLVE_NORMAL);
}
void CSequenceAugusta::Activate(_bool IsActivate)
{
	//m_isActivate = IsActivate;
	if (false == IsActivate)
	{
		m_pColliderCom->Set_Position(XMVectorSet(0.f, -3000.f, 0.f, 1.f));
		Bind_DissolveTimer();
		Bind_DissolveShaderPath();
		XMStoreFloat4x4(&m_DissolveWorldMatrix, m_pTransformCom->Get_WorldMatrix());
	}

	if (true == IsActivate)
	{
		m_isActivate = true;
		m_IsVisible = true;
		m_IsOutLineVisible = true;
		Remove_Condition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
		Bind_DefaultShaderPath();
		m_pTransformCom->Save_PreviousPosition();
		m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));
		Sync_Collider(XMVectorZero(), 0.f);

		// State까지 결정
		m_StateContext.m_eSkillType = ESequenceAugustaSkillType::ATTACK_SPSKILL;
		m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ESequenceAugustaGroundState::SKILL));
	}
}
#pragma endregion



void CSequenceAugusta::Update_Physics(_float fTimeDelta)
{
	// 3. 현재 위치 - 1Frame 이전 위치 값 계산'
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	m_pColliderCom->Update(vVelocity / fTimeDelta);
	m_IsLand = Is_LandCollider();
}


void CSequenceAugusta::Update_TargetDistance(_float fTimeDelta)
{
	const _float4x4* pTargetMatrix = nullptr;

	_vector vTargetPos = {};

	if (nullptr != m_pTargetTransform)
		vTargetPos = m_pTargetTransform->Get_State(STATE::POSITION);

	// 거리 계산. Y제외.
	_vector vDistance = m_pTransformCom->Get_State(STATE::POSITION) - vTargetPos;
	vDistance = XMVectorSetY(vDistance, 0.f);
	m_fTargetDistance = XMVectorGetX(XMVector3Length(vDistance));
}

void CSequenceAugusta::Bind_Resources()
{
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(CHARACTER_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		const _float4x4* pWorldMatrix = &m_DissolveWorldMatrix;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", pWorldMatrix))) // Dissolve는 해당 위치에 멈춰서 재생되어야함.
			CRASH("Failed Bind Matrix");
	}
	else
	{
		if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
			CRASH("Failed Bind Matrix");
	}

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");

}

void CSequenceAugusta::Ready_Components(const CHARACTER_DESC* pDesc)
{
    // 1. Components
    if(FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
        , pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
        CRASH("Compute Shader");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->flyComputeShaderData.first)
		, pDesc->flyComputeShaderData.second, TEXT("Com_ComputeShaderFly"), reinterpret_cast<CComponent**>(&m_pFlyComputeShaderCom), nullptr)))
		CRASH("Com_ComputeShaderFly");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->facialComputeShaderData.first)
		, pDesc->facialComputeShaderData.second, TEXT("Com_ComputeShaderFacial"), reinterpret_cast<CComponent**>(&m_pFacialComputeShaderCom), nullptr)))
		CRASH("Com_ComputeShaderFly");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
        , pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->stateMachineData.first)
        , pDesc->stateMachineData.second, TEXT("Com_StateMachine"), reinterpret_cast<CComponent**>(&m_pStateMachineCom), nullptr)))
        CRASH("StateMachine");


	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.vPos = pDesc->vPosition;
	ColliderDesc.vOffset = { 0.f, 0.67f, 0.f };
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::QTE); // 땅만 타게.?
	ColliderDesc.fHeight = 0.4f;
	ColliderDesc.fRadius = 0.5f;
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Collider"), TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		CRASH("Collider");

}

void CSequenceAugusta::Ready_Variables(const CHARACTER_DESC* pDesc)
{
	m_fDodgeableDuration = 0.1f; // Dodge 가능 시간.

	m_vMotionTrailColor = { 1.f, 0.5f, 0.1f, 1.f }; // 기본

    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH_CHARACTER::ROVER);

	// Shader Vlaue 추가
	m_fDissolveTimer = 0.f;
	m_fMaxDissolveTime = 0.35f;
	m_vDissolveColor = { 0.5f, 0.2f, 0.1f, 1.f };
	m_fEmissiveIntensity = 3.f;

	

}

void CSequenceAugusta::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

}


void CSequenceAugusta::Ready_PartObjects(const CHARACTER_DESC* pDesc)
{
    _float3 vScale = {};
    _float3 vRotation = {};
    _float3 vPosition = {};

    for (_uint i = 0; i < PARTTYPE::TYPE_END; ++i)
    {
        _wstring strPartName = pDesc->PartPrototypes[i].first;
        _wstring strPrototypeName = pDesc->PartPrototypes[i].second;

        CProp::PROP_DESC Desc{};
        switch (i)
        {
		case PARTTYPE::PART_BAYONET:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = SeqPlayerData::GetAugustaBayonetCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);

			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("Weapon");

			m_pBayonet = dynamic_cast<CAugustaBayonet*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pBayonet);
			Safe_AddRef(m_pBayonet);
			break;
		}
    }
}

void CSequenceAugusta::Ready_AttackVolumes()
{
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc{};
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::BONE; // 뼈
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp01");
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::KNOCKBACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(1.f, 1.f, 1.f); // x, z 크게 y작게
	TriggerDesc.vOffsetPos = _float3(0.0f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 400.f;
	TriggerDesc.eDamageType = TEXT_COLOR_TYPE::DARK;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};


	m_AttackVolumes[VOLUME_ATTACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_ATTACK]);
	m_AttackVolumes[VOLUME_ATTACK]->TriggerActivate(false);


	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 2.f); // x, z 크게 y작게
	m_AttackVolumes[VOLUME_SKILL] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_SKILL]);
	m_AttackVolumes[VOLUME_SKILL]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_ATTACK];
	m_pMainAttackVolume->TriggerActivate(false);
}

CSequenceAugusta* CSequenceAugusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSequenceAugusta* pInstance = new CSequenceAugusta(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CSequenceAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSequenceAugusta::Clone(void* pArg)
{
    CSequenceAugusta* pInstance = new CSequenceAugusta(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CSequenceAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSequenceAugusta::Free()
{
    CCharacter::Free();
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}

	m_AttackVolumes.clear();

	Safe_Release(m_pBayonet);
}
