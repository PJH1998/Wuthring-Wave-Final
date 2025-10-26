#include "ClientPch.h"
#include "Augusta.h"
#include "Player.h"
#include "SpringCamera.h"

#include "AugustaStateFactory.h"
#include "AugustaState_Enum.h"
#include "AugustaBayonet.h"


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
    CAugustaStateFactory::Register_States(m_pStateMachineCom, this);
    CAugustaStateFactory::Register_Camera(LEVEL::STATIC, m_eCurLevel, this, m_pGameInstance, &m_pSpringCamera);


    // 초기 State 설정.
    m_StateContext.m_eIdleType = EIdleType::STAND1_ACTION01;
    m_pStateMachineCom->Change_State(static_cast<_uint>(EStateCategory::GROUND),
        static_cast<_uint>(EAugustaGroundState::IDLE));
    
    
    m_pColliderCom->Set_Gravity(true);

    // 기본적으로 무기 Activate 끄기?
    m_pAugustaBayonet->SetActivate(false);

    return S_OK;
}

void CAugusta::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    // 2. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();

    // 3. 키입력 갱신은 Player 객체에서 관리 중
    if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::WB), KEYSTATE::UP))
    {
        m_IsLockOn = !m_IsLockOn;
        m_pSpringCamera->Lock_On();
    }
        
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
    m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 0.5f);

    

    // 7. 파츠 갱신.?
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


    // Collider 충돌 처리후 위치에 맞춘다.
    m_pColliderCom->Sync_Position(m_pTransformCom);


    

    // 사용이 끝났으면 반환.
    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
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

// AnimName이 같은걸로 매핑되어있음.
void CAugusta::Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
{
    switch (iPartType)
    {
    case PART_BAYONET:
        
        break;
    }
}

void CAugusta::PartAcitvate(_uint iPartType, _bool IsActive)
{
    switch (iPartType)
    {
    case PART_BAYONET:
        m_pAugustaBayonet->SetActivate(IsActive);
        break;
    }
}

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

            m_pAugustaBayonet = dynamic_cast<CAugustaBayonet*>(Find_PartObject(strPartName));
            ASSERT_CRASH(m_pAugustaBayonet);
            Safe_AddRef(m_pAugustaBayonet);
            break;

        case PARTTYPE::PART_SHIELD:
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
    Safe_Release(m_pAugustaBayonet);
}
