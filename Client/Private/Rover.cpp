#include "ClientPch.h"
#include "Rover.h"
#include "Player.h"
#include "SpringCamera.h"
#include "RoverSword.h"
#include "RoverDarkWing.h"
#include "RoverDarkScythe.h"

#include "Wing.h"
#include "RoverFactory.h"
#include "Collider.h"

#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CRover::CRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CRover::CRover(const CRover& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CRover::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CRover::Initialize_Clone(void* pArg)
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
	//Register_AbilityFiles(pDesc->strAbilityFolderPath);

	CRoverFactory::Register_States(m_pStateMachineCom, this);
	
	// 비활성화. 
	PartActivate(PART_SWORD, false);
	PartActivate(PART_DARKWING, false);
	PartActivate(PART_DARKSCYTHE, false);
	//PartActivate(PART_DARKSCYTHE, true);
	
	PartActivate(PART_WING, false);
	
	m_IsQTE = false;
    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());

    return S_OK;
}

void CRover::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

	// 1. Parts 갱신
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Priority_Update(fTimeDelta);
	}

	// 0. Delayed Action 수행.
	Process_DelayedActions();

    // 2. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();

	// 3. 몬스터가 있다면?
	if (nullptr != m_pTargetTransform)
	{
		_vector vDistance = (m_pTransformCom->Get_State(STATE::POSITION) - m_pTargetTransform->Get_State(STATE::POSITION));
		vDistance = XMVectorSetY(vDistance, 0.f);
		m_fTargetDistance = XMVectorGetX(XMVector3Length(vDistance));
	}

	// 4. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Priority_Update(fTimeDelta);
  
}

void CRover::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

	// 2. 파츠 갱신.?
	for (auto& pPart : m_PartObjects)
	{
		if (pPart.second->IsActivate())
			pPart.second->Update(fTimeDelta);
	}

	// 3. 상태 머신 갱신
	m_pStateMachineCom->Update(fTimeDelta); // 여기서 Weapon이나 Parts의 갱신을 해야함.. => 여기서 Play_Animation 실행됨.

	// 4. 현재 위치 - 1Frame 이전 위치 값 계산
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if (!m_IsQTE)
	{
		// 5. Collider 갱신 => Jolt 자체에서도 fTimeDelta 값을 적용하고 있기 때문에 
		m_pColliderCom->Update(vVelocity / fTimeDelta);

		// 6. Camera 갱신 => 위치 따라오게
		m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 1.2f);

	}
	else
	{
		m_pQTEColliderCom->Update(vVelocity / fTimeDelta);
	}

	// 7. Land Check
	m_IsLand = Is_LandCollider();

	// 8. Hit 초기화 => ObjectUpdate -> Font -> Camera -> Physics Update(Hit Judge 판단) -> Late_Update
	Remove_Condition(CHARACTER_CONDITION::HIT);

	// 9. MainAttackVolume 설정
	if (nullptr != m_pMainAttackVolume)
		m_pMainAttackVolume->Update(fTimeDelta);

}
void CRover::Late_Update(_float fTimeDelta)
{
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
	if (!m_IsQTE)
		m_pColliderCom->Sync_Position(m_pTransformCom);
	else
		m_pQTEColliderCom->Sync_Position(m_pTransformCom);

	if (m_IsQTEend)
	{
		Notify_HarmonyEnd();
		m_IsQTEend = false;
	}


    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CRover::Render()
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
	if (!m_IsQTE)
		m_pColliderCom->Render();
	else
		m_pQTEColliderCom->Render();

	if (m_pMainAttackVolume->IsActivate())
		m_pMainAttackVolume->Render();
#endif // _DEBUG

}

void CRover::Render_Shadow()
{
}


// 캐릭터 전환시 Idle로 상태 전환..
void CRover::TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType)
{
	switch (eTransitionType)
	{
		case CHARACTER_TRANSITIONTYPE::IDLE:
		{
			// 애니메이션 변경할 값.
			GetStateContextForWrite().m_eIdleType = ERoverIdleType::STAND1;
			m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::IDLE));
			break;
		}
	}

	// 상태 변수 초기화
	m_IsQTE = false;
	m_StateContext.Clear();
}



	// AnimName이 같은걸로 매핑되어있음.
void CRover::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	case PART_WING:
		m_pWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
		break;
	
    default:
        break;
    }
}

void CRover::PartActivate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Activate(IsActive);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Activate(IsActive);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Activate(IsActive);
		break;
	case PART_WING:
		m_pWing->Activate(IsActive);
		break;
    default:
        break;
    }
}

void CRover::Part_VolumeChange(_uint iPartType, _uint iVolumeIdx)
{
	switch (iPartType)
	{
	case PART_SWORD:
		m_pRoverSword->Change_Volume(iVolumeIdx);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Change_Volume(iVolumeIdx);
		break;
	}
}

void CRover::Part_VolumeActivate(_uint iPartType, _bool IsActive)
{
	switch (iPartType)
	{
	case PART_SWORD:
		m_pRoverSword->Volume_Activate(IsActive);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Volume_Activate(IsActive);
		break;
	}
}

void CRover::Clear_PartAnimation(_uint iPartType, const _string& strAnimName)
{
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Clear_Animation(strAnimName);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Clear_Animation(strAnimName);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Clear_Animation(strAnimName);
		break;
	case PART_WING:
		m_pWing->Clear_Animation(strAnimName);
		break;
    default:
        break;
    }

}

void CRover::Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName)
{
    ASSERT_CRASH(m_pModelCom);
    
    const _float4x4* pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(strBoneName.c_str());
    if (nullptr == pSocketMatrix)
        pSocketMatrix = &m_MatrixIdentity;
    
    switch (iPartType)
    {
    case PART_SWORD:
		m_pRoverSword->Set_SocketMatrix(pSocketMatrix);
        break;
	case PART_DARKWING:
		m_pRoverDarkWing->Set_SocketMatrix(pSocketMatrix);
		break;
	case PART_DARKSCYTHE:
		m_pRoverDarkScythe->Set_SocketMatrix(pSocketMatrix);
		break;
	case PART_WING:
		m_pWing->Set_SocketMatrix(pSocketMatrix);
		break;
    }
}

// Hit 판정.
void CRover::Hit_Judge(void* pArg)
{
	if (nullptr == pArg || m_IsHit)
		return;

	StateKey eKey = m_pStateMachineCom->Get_CurrentStateKey();
	_uint iCategory = eKey.iCategory;
	_uint iSubState = eKey.iSubState;

	EStateCategory eCategory = static_cast<EStateCategory>(iCategory);

	if (EStateCategory::HIT == eCategory)
		return;

	// 2. 즉시 중복 방지 플래그 세팅
	m_PendingConditions[HIT] = true;

	// 3. 데이터 저장.
	CCharacter::HIT_DESC* pDesc = static_cast<HIT_DESC*>(pArg);
	//m_pAbillityCom->Add_Hp(-pDesc->fAttack);

	// 4. 큐에 Hit 이벤트 push (실제 로직은 처리 시 실행)
	m_DelayedActions.push(DELAYED_ACTION(DELAYED_ACTION::TYPE::HIT, pDesc));

	m_PendingHitDesc = *pDesc;
}

void CRover::Sync_Position()
{
    m_pColliderCom->Sync_Position(m_pTransformCom);
}

void CRover::Bind_QTE(_bool IsQTE)
{
	m_IsQTE = IsQTE;

	if (m_IsQTE)
	{
		// Activate
		SetActivate(true);
		_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);

		// 내 앞에서 생성. (안 곂치게)
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
		_vector vUp = XMVectorSet(0.f, 2.f, 0.f, 0.f);
		vPos += vLook * 1.f;
		m_pQTEColliderCom->Set_Position(vPos);
		GetStateContextForWrite().m_eQTEType = ERoverQTEType::SKILL_QTE;
		Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ERoverGroundState::QTE));
	}
}



#pragma region NOTIFY
void CRover::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
    if (wStrColliderTag == TEXT("Body"))
    {
		m_pColliderCom->IsActivate(IsActive);
    }
    else if (wStrColliderTag == TEXT("Sword"))
    {
		if (nullptr != m_pRoverSword)
			m_pRoverSword->Volume_Activate(IsActive);
    }
	else if (wStrColliderTag == TEXT("Rover"))
	{
		if (nullptr != m_pMainAttackVolume)
			m_pMainAttackVolume->TriggerActivate(IsActive);
	}
	else if (wStrColliderTag == TEXT("Scythe"))
	{
		if (nullptr != m_pMainAttackVolume)
			m_pMainAttackVolume->TriggerActivate(IsActive);
	}
	else if (wStrColliderTag == TEXT("DarkWing"))
	{
		if (nullptr != m_pMainAttackVolume)
			m_pMainAttackVolume->TriggerActivate(IsActive);
	}

}

void CRover::Effect_Active(const _wstring& wStrEffectTag)
{
    if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
        return;

    _matrix matWorld = m_pTransformCom->Get_WorldMatrix();
    m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}
void CRover::Object_Func(const _wstring& wStrObjectTag)
{
	// 3개의 변수 준비
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);

	// std::getline을 사용하여 L'|' 구분자를 만날 때까지 읽어 변수에 저장합니다.
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)

	_uint iVolumeIdx = stoul(var3);

	/* SWORD|ROVER|0*/
	// 1. 어떤 무기인가?
	if (var1 == TEXT("SWORD"))
	{
		// 볼륨 인덱스로 볼륨 변경. (VOLUME_ATTACK (0))
		m_pRoverSword->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
		{
			// 3. 볼륨 레이어 변경
			m_pRoverSword->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		}
		else if (var2 == TEXT("SKILL"))
			m_pRoverSword->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pRoverSword->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
	}
	if (var1 == TEXT("SCYTHE"))
	{
		// 볼륨 인덱스로 볼륨 변경. (VOLUME_ATTACK (0))
		m_pRoverDarkScythe->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
		{
			// 3. 볼륨 레이어 변경
			m_pRoverDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		}
		else if (var2 == TEXT("SKILL"))
			m_pRoverDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pRoverDarkScythe->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
	}
	else if (var1 == TEXT("DARKWING"))
	{
		// 볼륨 인덱스로 볼륨 변경. (VOLUME_ATTACK (0))
		m_pRoverDarkWing->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
		{
			// 3. 볼륨 레이어 변경
			m_pRoverDarkWing->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		}
		else if (var2 == TEXT("SKILL"))
			m_pRoverDarkWing->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pRoverDarkWing->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
	}
	else if (var1 == TEXT("ROVER"))
	{
		// 볼륨 인덱스로 볼륨 변경. (VOLUME_RISE (0), VOLUME_HACKDOWN(1))
		if (nullptr == m_AttackVolumes[iVolumeIdx] || nullptr == m_pMainAttackVolume)
			return;

		m_pMainAttackVolume->TriggerActivate(false); // 교체.
		m_pMainAttackVolume = m_AttackVolumes[iVolumeIdx];

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::ATTACK);
		else if (var2 == TEXT("SKILL"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::SKILL);
		else if (var2 == TEXT("KNOCKBACK"))
			m_pMainAttackVolume->Change_Layer(COLLISIONLAYER::KNOCKBACK);
	}
	

}
void CRover::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 1. 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::ROVER));

	if (nullptr == pAbility)
		return;

	switch (m_iVolumeIdx)
	{
	case VOLUME::VOLUME_KNOCKBACK: // 기본 공격시 공명 게이지와 궁게이지 채우기
		pAbility->Add_HarmonyGauge(7.f); // 공명 게이지 채우기.
		pAbility->Add_Cost(COST_TYPE::COST1, 5.f); // 궁 ULTI
		break;
	}
}

#pragma endregion

#pragma region 4. EVENT
void CRover::Process_DelayedActions()
{
	while (!m_DelayedActions.empty())
	{
		DELAYED_ACTION eAction = m_DelayedActions.front();

		void* pData = eAction.pData;
		switch (eAction.type)
		{
		case DELAYED_ACTION::TYPE::HIT:
		{
			//m_IsHit = true;
			Add_Condition(CHARACTER_CONDITION::HIT); // Condition 추가.
			m_pAbillityCom->Add_Hp(-m_PendingHitDesc.fAttack);
			break;
		}
		case DELAYED_ACTION::TYPE::PARRY:
		{
			break;
		}

		default:
			break;
		}

		m_DelayedActions.pop();
	}
}
#pragma endregion



void CRover::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");

}

void CRover::Ready_Components(const CHARACTER_DESC* pDesc)
{
    // 1. Components
    if(FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
        , pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
        CRASH("Compute Shader");

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
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::QTE);
	ColliderDesc.fHeight = 0.4f;
	ColliderDesc.fRadius = 0.5f;
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Collider"), TEXT("Com_QTECollider"), reinterpret_cast<CComponent**>(&m_pQTEColliderCom), &ColliderDesc)))
		CRASH("Collider");

}

void CRover::Ready_Variables(const CHARACTER_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CRover::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

}


void CRover::Ready_PartObjects(const CHARACTER_DESC* pDesc)
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
        case PARTTYPE::PART_SWORD:

            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetRoverWeaponCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


            // PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

			m_pRoverSword = dynamic_cast<CRoverSword*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pRoverSword);
            Safe_AddRef(m_pRoverSword);
            break;
		case PARTTYPE::PART_DARKWING:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetRoverDarkWingCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("DarkWing");

			m_pRoverDarkWing = dynamic_cast<CRoverDarkWing*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pRoverDarkWing);
			Safe_AddRef(m_pRoverDarkWing);
			break;
		case PARTTYPE::PART_DARKSCYTHE:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetRoverDarkScytheCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("DarkScythe");

			m_pRoverDarkScythe = dynamic_cast<CRoverDarkScythe*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pRoverDarkScythe);
			Safe_AddRef(m_pRoverDarkScythe);
			break;

		case PARTTYPE::PART_WING:
			vScale = { 1.f, 1.f, 1.f };
			vPosition = { 0.f, 0.f, 0.f };
			Desc = PlayerData::GetWingCloneData(vScale, vRotation, vPosition, m_eCurLevel);
			Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
			Desc.pParentTransform = m_pTransformCom;
			ASSERT_CRASH(Desc.pSocketMatrix);

			// PropDesc
			if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
				, strPrototypeName, &Desc)))
				CRASH("Weapon");

			m_pWing = dynamic_cast<CWing*>(Find_PartObject(strPartName));
			ASSERT_CRASH(m_pWing);
			Safe_AddRef(m_pWing);
			break;
		}
    }
}

void CRover::Ready_AttackVolumes()
{
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
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
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};


	m_AttackVolumes[VOLUME_KNOCKBACK] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_KNOCKBACK]);
	m_AttackVolumes[VOLUME_KNOCKBACK]->TriggerActivate(false);


	TriggerDesc.eLayer = COLLISIONLAYER::SKILL;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(3.f, 3.f, 2.f); // x, z 크게 y작게
	m_AttackVolumes[VOLUME_SKILL] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));


	ASSERT_CRASH(m_AttackVolumes[VOLUME_SKILL]);
	m_AttackVolumes[VOLUME_SKILL]->TriggerActivate(false);

	m_pMainAttackVolume = m_AttackVolumes[VOLUME_KNOCKBACK];
	m_pMainAttackVolume->TriggerActivate(false);
}

CRover* CRover::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRover* pInstance = new CRover(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CRover::Clone(void* pArg)
{
    CRover* pInstance = new CRover(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CRover::Free()
{
    CCharacter::Free();
	for (auto& pAttackVolume : m_AttackVolumes)
	{
		if (nullptr != pAttackVolume)
			Safe_Release(pAttackVolume);
	}

	m_AttackVolumes.clear();
    Safe_Release(m_pRoverSword);
    Safe_Release(m_pRoverDarkWing);
    Safe_Release(m_pRoverDarkScythe);
	Safe_Release(m_pWing);
}
