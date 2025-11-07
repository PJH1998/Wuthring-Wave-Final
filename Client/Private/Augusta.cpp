#include "ClientPch.h"
#include "Augusta.h"
#include "Player.h"
#include "SpringCamera.h"
#include "Collider.h"
#include "Ability.h"

#include "AugustaFactory.h"
#include "AugustaState_Enum.h"
#include "AugustaBayonet.h"
#include "AugustaSkillWeapon.h"
#include "AugustaGriffon.h"
#include "AttackVolume.h"
#include "Wing.h"


CAugusta::CAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CAugusta::CAugusta(const CAugusta& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CAugusta::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugusta::Initialize_Clone(void* pArg)
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

    CAugustaFactory::Register_States(m_pStateMachineCom, this);
	m_pBayonet->SetActivate(false);
    m_pSkillWeapon->SetActivate(false);
    m_pGriffon->SetActivate(false);
	m_pWing->SetActivate(false);
    
	
    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());

	
    return S_OK;
}

void CAugusta::Priority_Update(_float fTimeDelta)
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
	if (nullptr != m_pTargetTransform)
	{
		_vector vDistance = (m_pTransformCom->Get_State(STATE::POSITION) - m_pTargetTransform->Get_State(STATE::POSITION));
		m_fTargetDistance = XMVectorGetX(XMVector3Length(vDistance));

		/*
		_vector vVelocity = m_pTransformCom->Get_Velocity();
		//m_fDistance : 플레이어와 몬스터 사이의 거리
		m_pColliderCom->Update(vVelocity / fTimeDelta * (m_fDistance * fTimeDelta));
		*/
	}
	
	//// 3. Ability Update();
	//m_pAbillityCom->Update(fTimeDelta);
}

void CAugusta::Update(_float fTimeDelta)
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
	// 여기서 PlayAnimation 도중에 Notify가 실행됨 => 그럼 이시점에서 WorldMatrix를 줌.


    // 5. 현재 위치 - 1Frame 이전 위치 값 계산
    _vector vVelocity = m_pTransformCom->Get_Velocity();

	//vVelocity += XMVectorSet(0.f, -9.8f, 0.f, 0.f) * fTimeDelta * 0.1f;

    // 6. Collider 갱신 => Jolt 자체에서도 fTimeDelta 값을 적용하고 있기 때문에 
	m_pColliderCom->Update(vVelocity / fTimeDelta);

    // 7. Camera 갱신 => 위치 따라오게
    m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 1.2f);

	// 8. Land Check
	m_IsLand = Is_LandCollider();

	// 9. Hit 초기화 => ObjectUpdate -> Font -> Camera -> Physics Update(Hit Judge 판단) -> Late_Update
	m_IsHit = false;
}
void CAugusta::Late_Update(_float fTimeDelta)
{
    // 1. 파츠 갱신
    for (auto& pPart : m_PartObjects)
    {
        if (pPart.second->IsActivate())
            pPart.second->Late_Update(fTimeDelta);
    }


    m_pColliderCom->Sync_Position(m_pTransformCom);
	
    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE, this)))
		return;

	if(FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
		return;
}

void CAugusta::Render()
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

		if(FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
			CRASH("Ready Shader Begin Failed");
        //if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
        //    CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }

#ifdef _DEBUG
    m_pColliderCom->Render();

#endif // _DEBUG
}

void CAugusta::Render_Shadow()
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

void CAugusta::Render_OutLine()
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

void CAugusta::TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType)
{
	// 애니메이션 변경할 값.
	switch (eTransitionType)
	{
	case CHARACTER_TRANSITIONTYPE::IDLE:
		GetStateContextForWrite().m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
		m_pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(EAugustaGroundState::IDLE));
		break;
	case CHARACTER_TRANSITIONTYPE::RUN:
		break;
	}
	

	// 상태 변수 초기화
	m_StateContext.Clear();
}

// AnimName이 같은걸로 매핑되어있음.
void CAugusta::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _bool IsLoop)
{
    switch (iPartType)
    {
    case PART_BAYONET:
		m_pBayonet->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    case PART_SKILLWEAPON:
		m_pSkillWeapon->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    case PART_GRIFFON:
		m_pGriffon->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    case PART_WING:
		m_pWing->Play_Animation(strAnimName, fTimeDelta, pTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate);
        break;
    }
}

void CAugusta::PartActivate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
    case PART_BAYONET:
        m_pBayonet->Activate(IsActive);
        break;
    case PART_SKILLWEAPON:
        m_pSkillWeapon->Activate(IsActive);
        break;
    case PART_GRIFFON:
        m_pGriffon->Activate(IsActive);
        break;
	case PART_WING:
		m_pWing->Activate(IsActive);
		break;
    }
}

void CAugusta::Part_VolumeChange(_uint iPartType, _uint iVolumeIdx)
{
	switch (iPartType)
	{
	case PART_BAYONET:
		m_pBayonet->Change_Volume(iVolumeIdx);
		break;
	case PART_SKILLWEAPON:
		m_pSkillWeapon->Change_Volume(iVolumeIdx);
		break;
	case PART_GRIFFON:
		m_pGriffon->Change_Volume(iVolumeIdx);
		break;
	case PART_WING:
		m_pWing->Change_Volume(iVolumeIdx);
		break;
	}
}

void CAugusta::Part_VolumeActivate(_uint iPartType, _bool IsActive)
{
	switch (iPartType)
	{
	case PART_BAYONET:
		m_pBayonet->Volume_Activate(IsActive); // MainVolume 켜기
		break;
	case PART_SKILLWEAPON:
		m_pSkillWeapon->Volume_Activate(IsActive); // MainVolume 켜기
	//	m_pSkillWeapon->Change_Volume(iVolumeIdx);
		break;
	case PART_GRIFFON:
		m_pGriffon->Volume_Activate(IsActive); // MainVolume 켜기
	//	m_pGriffon->Change_Volume(iVolumeIdx);
		break;
	case PART_WING:
	//	m_pWing->Change_Volume(iVolumeIdx);
		break;
	}
}

void CAugusta::Clear_PartAnimation(_uint iPartType, const _string& strAnimName)
{
    switch (iPartType)
    {
    case PART_BAYONET:
        m_pBayonet->Clear_Animation(strAnimName);
        break;
    case PART_SKILLWEAPON:
        m_pSkillWeapon->Clear_Animation(strAnimName);
        break;
    case PART_GRIFFON:
        m_pGriffon->Clear_Animation(strAnimName);
        break;
	case PART_WING:
		m_pWing->Clear_Animation(strAnimName);
		break;
    }
}

void CAugusta::Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName)
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
    case PART_SKILLWEAPON:
        m_pSkillWeapon->Set_SocketMatrix(pSocketMatrix);
        break;
    case PART_GRIFFON:
        m_pGriffon->Set_SocketMatrix(pSocketMatrix);
        break;
    }
}

// Hit 판정.
void CAugusta::Hit_Judge(void* pArg)
{
	if (nullptr == pArg || m_IsHit)
		return;

	StateKey eKey = m_pStateMachineCom->Get_CurrentStateKey();
	_uint iCategory = eKey.iCategory;
	_uint iSubState = eKey.iSubState;

	EStateCategory eCategory = static_cast<EStateCategory>(iCategory);
	
	// 1. 맞는데 또맞진 말자..
	if (EStateCategory::HIT == eCategory)
		return;

	// 2. 데미지는 바로 감소시킵니다.
	CCharacter::HIT_DESC* pDesc = static_cast<HIT_DESC*>(pArg);
	m_pAbillityCom->Add_Hp(-pDesc->fAttack);


	// 3. 캐스팅 해서? => 들고 있기.
	m_PendingHitDesc = *pDesc;

	

	// 4. 현재 상태 변경.
	m_IsHit = true;
}


void CAugusta::Sync_Position()
{
    m_pColliderCom->Sync_Position(m_pTransformCom);
}


#ifdef _DEBUG
void CAugusta::PartRotation(_uint iPartType, _fvector vQuaternion)
{

}

#endif // _DEBUG

#pragma region NOTIFY
void CAugusta::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);

	// Main Attack Volume의 TriggerActivate
	if (wstrTypeTag == TEXT("Bayonet"))
	{
		if (nullptr != m_pBayonet)
			m_pBayonet->Volume_Activate(IsActive);
	}
	else if (wstrTypeTag == TEXT("SkillWeapon"))
	{
		if (nullptr != m_pSkillWeapon)
			m_pSkillWeapon->Volume_Activate(IsActive);
	}
	else if (wstrTypeTag == TEXT("Griffon"))
	{
		if (nullptr != m_pGriffon)
			m_pGriffon->Volume_Activate(IsActive);
	}
	else if (wstrTypeTag == TEXT("Augusta"))
	{
		if (nullptr != m_pMainAttackVolume)
			m_pMainAttackVolume->TriggerActivate(IsActive);
	}

}
void CAugusta::Effect_Active(const _wstring& wStrEffectTag)
{
    if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
        return;

    _matrix matWorld = m_pTransformCom->Get_WorldMatrix();
    m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}
void CAugusta::Object_Func(const _wstring& wStrObjectTag)
{

	// 3개의 변수 준비
	_wstring var1, var2, var3;
	wstringstream wss(wStrObjectTag);

	// std::getline을 사용하여 L'|' 구분자를 만날 때까지 읽어 변수에 저장합니다.
	getline(wss, var1, L'|');
	getline(wss, var2, L'|');
	getline(wss, var3, L'|'); // 마지막 부분 (구분자가 없어도 끝까지 읽음)
	
	_uint iVolumeIdx = stoul(var3);



	/* BAYONET|ATTACK|0*/
	// 1. 어떤 무기인가?
	if (var1 == TEXT("BAYONET"))
	{
		if (var2 == TEXT("NONE"))
		{
			// 볼륨 끄기.
			m_pBayonet->Volume_Activate(false);
			return;
		}
		// 볼륨 인덱스로 볼륨 변경.
		m_pBayonet->Change_Volume(iVolumeIdx);

		// 2. 어떤 레이어인가? , 3. 어떤 볼륨인덱스를 사용할건가 ?.
		if (var2 == TEXT("ATTACK"))
		{
			// 3. 볼륨 레이어 변경
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::ATTACK);
		}
		else if (var2 == TEXT("KNOCKBACK"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::KNOCKBACK);
		else if (var2 == TEXT("SKILL"))
			m_pBayonet->Change_VolumeLayer(iVolumeIdx, COLLISIONLAYER::SKILL);
		
	}
	else if (var1 == TEXT("GRIFFON"))
	{

	}
}
void CAugusta::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
#ifdef _DEBUG
	cout << "On Hit! Augusta Bayonet" << endl;
#endif // _DEBUG

}
#pragma endregion

 



void CAugusta::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

void CAugusta::Ready_Components(const CHARACTER_DESC* pDesc)
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


	//if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->abilityData.first)
	//	, pDesc->abilityData.second, TEXT("Com_Ability"), reinterpret_cast<CComponent**>(&m_pAbillityCom), nullptr)))
	//	CRASH("Ability");

}

void CAugusta::Ready_Variables(const CHARACTER_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::AUGUSTA);
}

void CAugusta::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

    //_float3 vRadian = {
    //    XMConvertToRadians(pDesc->vRotation.x),
    //    XMConvertToRadians(pDesc->vRotation.y),
    //    XMConvertToRadians(pDesc->vRotation.z) };
    //m_pTransformCom->Rotation_Quaternion(vRadian);
}


void CAugusta::Ready_PartObjects(const CHARACTER_DESC* pDesc)
{

    _float3 vScale = {};
    _float3 vRotation = {};
    _float3 vPosition = {};

    for (_uint i = 0; i < PARTTYPE::TYPE_END; ++i)
    {
        _wstring strPartName = pDesc->PartPrototypes[i].first;
        _wstring strPrototypeName = pDesc->PartPrototypes[i].second;

        CProp::PROP_DESC Desc{};
		CAttackVolume::ATKVOLUME_DESC VolumeDesc{};

        switch (i)
        {
        case PARTTYPE::PART_BAYONET:
            
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaBayonetCloneData(vScale, vRotation, vPosition, m_eCurLevel);
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

			
			VolumeDesc.eLayer = COLLISIONLAYER::ATTACK;
			VolumeDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
			VolumeDesc.eShape = SHAPE::BOX;
			VolumeDesc.pParenTransform = m_pTransformCom;
			VolumeDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp02");
			VolumeDesc.vExtent = _float3(1.f, 1.f, 1.f);
			VolumeDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
			VolumeDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
			VolumeDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
					this->OnHitEnter(iLayer, pOther, Manifold);
				};

			//m_AttackVolumes[PARTTYPE::PART_BAYONET] = dynamic_cast<CAttackVolume*>(m_pGameInstance->
			//	Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &VolumeDesc));
			//if (nullptr == m_AttackVolumes[PARTTYPE::PART_BAYONET])
			//	CRASH(m_pMainAttackVolume);
			//
			//m_AttackVolumes[PARTTYPE::PART_BAYONET]->TriggerActivate(false); // 끄고 켜기.
            break;

        case PARTTYPE::PART_SKILLWEAPON:
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaSkillWeaponCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pSkillWeapon = dynamic_cast<CAugustaSkillWeapon*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pSkillWeapon);
            Safe_AddRef(m_pSkillWeapon);
            break;
        case PARTTYPE::PART_GRIFFON:
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaGriffonCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


			// PropDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pGriffon = dynamic_cast<CAugustaGriffon*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pGriffon);
            Safe_AddRef(m_pGriffon);
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

void CAugusta::Ready_AttackVolumes()
{
	// size 설정
	m_AttackVolumes.resize(VOLUME_END);

	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::BONE; // 장비
	TriggerDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("Root");
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eLayer = COLLISIONLAYER::ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::ENEMY;
	TriggerDesc.vExtent = _float3(7.f, 7.f, 7.f);
	TriggerDesc.vOffsetPos = _float3(0.5f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = 1000.f;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnHitEnter(iLayer, pOther, Manifold);
		};

	// Attack용 만들기.
	m_AttackVolumes[VOLUME::VOLUME_RISE] = dynamic_cast<CAttackVolume*>(
		m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume")
			, PROTOTYPE::GAMEOBJECT, &TriggerDesc));

	ASSERT_CRASH(m_AttackVolumes[VOLUME::VOLUME_RISE]);
	m_pMainAttackVolume = m_AttackVolumes[VOLUME::VOLUME_RISE]; // Main Attack Volume 설정.
}

CAugusta* CAugusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugusta* pInstance = new CAugusta(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CAugusta::Clone(void* pArg)
{
    CAugusta* pInstance = new CAugusta(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CAugusta::Free()
{
    CCharacter::Free();
    Safe_Release(m_pBayonet);
    Safe_Release(m_pSkillWeapon);
    Safe_Release(m_pGriffon);
	Safe_Release(m_pWing);
}
