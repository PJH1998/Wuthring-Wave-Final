#include "ClientPch.h"
#include "Augusta.h"
#include "Player.h"
#include "SpringCamera.h"
#include "Collider.h"

#include "AugustaFactory.h"
#include "AugustaState_Enum.h"
#include "AugustaBayonet.h"
#include "AugustaSkillWeapon.h"
#include "AugustaGriffon.h"


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
    Register_AllNotifies(pDesc->strFolderPath);

    CAugustaFactory::Register_States(m_pStateMachineCom, this);


    // 초기 State 설정.
    m_StateContext.m_eIdleType = EAugustaIdleType::STAND1_ACTION01;
    m_pStateMachineCom->Change_State(static_cast<_uint>(EStateCategory::GROUND),
        static_cast<_uint>(EAugustaGroundState::IDLE));
    
    m_pColliderCom->Set_Gravity(true);
    m_pBayonet->SetActivate(true);
    m_pSkillWeapon->SetActivate(false);
    m_pGriffon->SetActivate(false);
    
    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());
    return S_OK;
}

void CAugusta::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    // 2. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();

    // 4. Parts 갱신
    for (auto& pPart : m_PartObjects)
    {
        if (pPart.second->IsActivate())
            pPart.second->Priority_Update(fTimeDelta);
    }

}

void CAugusta::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

    // 2. 상태 머신 갱신
    m_pStateMachineCom->Update(fTimeDelta); // 여기서 Weapon이나 Parts의 갱신을 해야함..


    // 3. 현재 위치 - 1Frame 이전 위치 값 계산
    _vector vVelocity = m_pTransformCom->Get_Velocity();


    // 4. Collider 갱신 => Jolt 자체에서도 fTimeDelta 값을 적용하고 있기 때문에 
    m_pColliderCom->Update(vVelocity / fTimeDelta);

    // 5. Camera 갱신 => 위치 따라오게
    m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 1.2f);

    // 6. 파츠 갱신.?
    for (auto& pPart : m_PartObjects)
    {
        if (pPart.second->IsActivate())
            pPart.second->Update(fTimeDelta);
    }

}
void CAugusta::Late_Update(_float fTimeDelta)
{
    // 파츠 갱신
    for (auto& pPart : m_PartObjects)
    {
        if (pPart.second->IsActivate())
            pPart.second->Late_Update(fTimeDelta);
    }

    m_pColliderCom->Sync_Position(m_pTransformCom);
    
    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
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

        m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0);

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }

#ifdef _DEBUG
    m_pColliderCom->Render();
#endif // _DEBUG

}

void CAugusta::Render_Shadow()
{
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
void CAugusta::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
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
    // 임시
    _bool IsLand = Is_Land(0.2f);
    
    // 강공?
    

    // 몬스터 공격 Dir
    ACTORDIR eAttackDir = ACTORDIR::RU;
    // Behit S = SMALL(기본 공 Big), B = Big (Skill로 맞으면 Big)
    if (IsLand)
    {
        // 특수 조건 우선순위에 따라 Change_State
        
        switch (eAttackDir)
        {
        case ACTORDIR::LU:
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_S_L;
            break;
        case ACTORDIR::RU:
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_S_R;
            break;
        case ACTORDIR::U: 
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_S_L;
            break;
        case ACTORDIR::LD:
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_B_L;
            break;
        case ACTORDIR::RD:
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_B_R;
            break;
        case ACTORDIR::D:
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_B_L;
            break;
        case ACTORDIR::L: // L, R은 정면 판단.
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_S_L;
            break;
        case ACTORDIR::R:
            m_StateContext.m_eHitType = EAugustaHitType::BEHIT_S_R;
            break;
        }

        CCharacter::Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitState::HIT));
    }
    else
        CCharacter::Change_State(ENUM_CLASS(EStateCategory::HIT), ENUM_CLASS(EAugustaHitType::BEHIT_FLY_START));

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
    if (wStrColliderTag == TEXT("Player"))
    {
        
    }
    else if (wStrColliderTag == TEXT("Bayonet"))
    {
        
    }
    else if (wStrColliderTag == TEXT("SkillWeapon"))
    {

    }
}
void CAugusta::Effect_Active(const _wstring& wStrEffectTag)
{
    if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
        return;

    _matrix matWorld = m_pTransformCom->Get_WorldMatrix();
    m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
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
    
    // 계산에 사용할 값 지정.
    m_fColliderRadius = 0.4f;
    m_fColliderHeight = 0.5f;
    m_vColliderOffSet = { 0.f, 0.67f, 0.f };
    //m_vColliderOffSet = { 0.f, 0.f, 0.f };


    CCollider::COLLIDER_DESC ColliderDesc{};
    ColliderDesc.vPos = pDesc->vPosition;
    ColliderDesc.vOffset = m_vColliderOffSet;
    ColliderDesc.eType = EMotionType::Kinematic;
    ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
    ColliderDesc.fHeight = m_fColliderHeight;
    ColliderDesc.fRadius = m_fColliderRadius;
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->colliderData.first)
        , pDesc->colliderData.second, TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
        CRASH("Collider");

    //몬스터 탐지용 콜백으로 받을 Desc - LJH
    m_pColliderCom->Set_Desc(m_pTransformCom);

}

void CAugusta::Ready_Variables(const CHARACTER_DESC* pDesc)
{
    m_pOwner = pDesc->pOwner;
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
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

        CWeapon::WEAPON_DESC Desc{};
        switch (i)
        {
        case PARTTYPE::PART_BAYONET:
            
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaBayonetCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);
            

            // WeaponDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pBayonet = dynamic_cast<CAugustaBayonet*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pBayonet);
            Safe_AddRef(m_pBayonet);
            break;

        case PARTTYPE::PART_SKILLWEAPON:
            vScale = { 1.f, 1.f, 1.f };
            vPosition = { 0.f, 0.f, 0.f };
            Desc = PlayerData::GetAugustaSkillWeaponCloneData(vScale, vRotation, vPosition, m_eCurLevel);
            Desc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(Desc.strBoneName.c_str());
            Desc.pParentTransform = m_pTransformCom;
            ASSERT_CRASH(Desc.pSocketMatrix);


            // WeaponDesc
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


            // WeaponDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, &Desc)))
                CRASH("Weapon");

            m_pGriffon = dynamic_cast<CAugustaGriffon*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pGriffon);
            Safe_AddRef(m_pGriffon);
            break;
        }
       
    }
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
}
