#include "ClientPch.h"
#include "PlayerAugusta.h"
#include "PlayerParty.h"

#include "AugustaStand1_Action01.h"
#include "AugustaStand1_Action02.h"
#include "AugustaRun_F.h"
#include "AugustaStop_Run_L.h"

CPlayerAugusta::CPlayerAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPlayer{ pDevice, pContext }
{
}

CPlayerAugusta::CPlayerAugusta(const CPlayerAugusta& Prototype)
    : CPlayer(Prototype)
{
}

HRESULT CPlayerAugusta::Initialize_Prototype()
{
    if (FAILED(CPlayer::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CPlayerAugusta::Initialize_Clone(void* pArg)
{
    PLAYER_DESC* pDesc = static_cast<PLAYER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CPlayer::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eCurLevel;
    // 2. Components
    Ready_Components(pDesc);

    // 3. 변수 설정
    Ready_Variables(pDesc);
    
    // 4. 위치 설정.
    Ready_Positions(pDesc);

    // State 등록.
    m_pStateMachineCom->Add_State("Stand1_Action01", CAugustaStand1_Action01::Create({ this, "Stand1_Action01", 1.f, 0.f }));
    m_pStateMachineCom->Add_State("Stand1_Action02", CAugustaStand1_Action02::Create({ this, "Stand1_Action02", 1.f, 0.f }));
    m_pStateMachineCom->Add_State("Run_F", CAugustaRun_F::Create({ this, "Run_F", 1.f, 0.f }));
    m_pStateMachineCom->Add_State("Stop_Run_L", CAugustaStop_Run_L::Create({ this, "Stop_Run_L", 1.f, 0.f }));

    // 플레이어 키인풋 등록.
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::W), DIK_W);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::A), DIK_A);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::S), DIK_S);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D), DIK_D);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::SPACE), DIK_SPACE);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::Q), DIK_Q);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::E), DIK_E);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::R), DIK_R);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::T), DIK_T);
    m_pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::LSHIFT), DIK_LSHIFT);

    // 마우스 키인풋 등록
    m_pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::LB), MOUSEKEYSTATE::LB);
    m_pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::WB), MOUSEKEYSTATE::WB);
    m_pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::RB), MOUSEKEYSTATE::RB);

    m_pStateMachineCom->Change_State("Stand1_Action01");

    // 5. Parts추가
    // Ready_PartObjects(pDesc);
    
    m_pColliderCom->Set_Gravity(true);
#ifdef _DEBUG
    m_strCurrentAnimation = "Pose";
    m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, 0.f, &m_fTrackPosition, true, 1.f);
#endif // _DEBUG

    return S_OK;
}

void CPlayerAugusta::Priority_Update(_float fTimeDelta)
{
    CPlayer::Priority_Update(fTimeDelta);

    // 1. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();
}

void CPlayerAugusta::Update(_float fTimeDelta)
{
    CPlayer::Update(fTimeDelta);

    // 1. 키 입력 갱신.
    //m_pInputControllerCom->Update();

    // 2. 상태 머신 갱신
    //m_pStateMachineCom->Update(fTimeDelta);

    if (m_IsPlayAnimation)
    {
        m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, true, 1.f);
        m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
    }

    Change_State(fTimeDelta);

    // 현재 위치 - 1Frame 이전 위치 값 계산
    _vector vVelocity = m_pTransformCom->Get_Velocity();

    

    // Collider 갱신 => Jolt 자체에서도 fTimeDelta 값을 적용하고 있기 때문에 
    m_pColliderCom->Update(vVelocity / fTimeDelta);

    if (m_strPreAnimation != m_strCurrentAnimation)
        m_fTrackPosition = 0.f;
}

void CPlayerAugusta::Late_Update(_float fTimeDelta)
{
    CPlayer::Late_Update(fTimeDelta);

    // Collider 충돌 처리후 위치에 맞춘다.
    m_pColliderCom->Sync_Position(m_pTransformCom);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CPlayerAugusta::Render()
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
#endif
}

void CPlayerAugusta::Render_Shadow()
{
}

void CPlayerAugusta::Change_State(_float fTimeDelta)
{
    _vector vTranslate = {};
    if (m_pGameInstance->Get_DIKeyState(DIK_W) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_F";
        vTranslate = m_pTransformCom->Get_State(STATE::LOOK) * -1.f;
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_S) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_B";
        vTranslate = m_pTransformCom->Get_State(STATE::LOOK);
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_A) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_LF";
        vTranslate = m_pTransformCom->Get_State(STATE::RIGHT);
    }
    if (m_pGameInstance->Get_DIKeyState(DIK_D) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Run_RF";
        vTranslate = m_pTransformCom->Get_State(STATE::RIGHT) * -1.f;
    }


    if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::UP)
        m_strCurrentAnimation = "Move_F";

    if (m_pGameInstance->Get_DIKeyState(DIK_SPACE) == KEYSTATE::PRESS)
    {
        m_strCurrentAnimation = "Jump_Walk_LF";

        _float4 vVelocity = {};
        XMStoreFloat4(&vVelocity, m_pTransformCom->Get_Velocity());
#ifdef _DEBUG
        OutPutDebugFloat4(TEXT("Jump Velocity"), vVelocity);
#endif
    }
        

    // 추가 이동량 지정.
    m_pTransformCom->Go_Force(XMVector3Normalize(vTranslate), fTimeDelta * 30.f);
}

void CPlayerAugusta::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");

}

void CPlayerAugusta::Ready_Components(const PLAYER_DESC* pDesc)
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
        CRASH("Model");

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->controllerData.first)
        , pDesc->controllerData.second, TEXT("Com_InputController"), reinterpret_cast<CComponent**>(&m_pInputControllerCom), nullptr)))
        CRASH("Model");




    
    CCollider::COLLIDER_DESC ColliderDesc{};
    ColliderDesc.vPos = { 0.f, 0.f, 0.f };
	ColliderDesc.vOffset = { 0.f, 6.7f, 0.f };
    ColliderDesc.eType = EMotionType::Kinematic;
    ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
    ColliderDesc.fHeight = 5.f;
    ColliderDesc.fRadius = 4.f;
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->colliderData.first)
        , pDesc->colliderData.second, TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
        CRASH("Collider");
}

void CPlayerAugusta::Ready_Variables(const PLAYER_DESC* pDesc)
{
    m_pOwner = pDesc->pOwner;
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CPlayerAugusta::Ready_Positions(const PLAYER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPostion), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

    //_float3 vRadian = {
    //    XMConvertToRadians(pDesc->vRotation.x),
    //    XMConvertToRadians(pDesc->vRotation.y),
    //    XMConvertToRadians(pDesc->vRotation.z) };
    //m_pTransformCom->Rotation_Quaternion(vRadian);
}


void CPlayerAugusta::Ready_PartObjects(const PLAYER_DESC* pDesc)
{

    for (_uint i = 0; i < PARTTYPE::TYPE_END; ++i)
    {
        _wstring strPartName = pDesc->PartPrototypes[i].first;
        _wstring strPrototypeName = pDesc->PartPrototypes[i].second;

        switch (i)
        {
        case PARTTYPE::PART_WEAPON:
            // WeaponDesc
            if (FAILED(CContainerObject::Add_PartObject(strPartName, ENUM_CLASS(m_eCurLevel)
                , strPrototypeName, nullptr)))
                CRASH("Weapon");
            break;

        case PARTTYPE::PART_SHIELD:
            break;
        }

    }
    
}

CPlayerAugusta* CPlayerAugusta::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CPlayerAugusta* pInstance = new CPlayerAugusta(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CPlayerAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CPlayerAugusta::Clone(void* pArg)
{
    CPlayerAugusta* pInstance = new CPlayerAugusta(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CPlayerAugusta");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPlayerAugusta::Free()
{
    CPlayer::Free();
    
}
