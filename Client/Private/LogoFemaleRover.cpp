#include "ClientPch.h"
#include "LogoFactory.h"
#include "LogoFemaleRover.h"
#include "SpringCamera.h"
#include "Collider.h"
#include "GameSystem.h"

CLogoFemaleRover::CLogoFemaleRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CLogoFemaleRover::CLogoFemaleRover(const CLogoFemaleRover& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CLogoFemaleRover::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CLogoFemaleRover::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CCharacter::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eCurLevel;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
	
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(-150.f, 10.f, 0.f, 1.f));

    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());


	// 시작 프레임 하나 실행.
	m_strCurrentAnimation = "AppearanceIdle";
	m_strPreAnimation = m_strCurrentAnimation;
	m_pModelCom->Play_Animation_CPU(m_strCurrentAnimation, 0.f, &m_fTrackPosition, false);
    return S_OK;
}

void CLogoFemaleRover::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    // 1. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();
  
	Logo_Input();
}

void CLogoFemaleRover::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

	m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, true);

}
void CLogoFemaleRover::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CLogoFemaleRover::Render()
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
	//m_pColliderCom->Render();
#endif // _DEBUG

}

void CLogoFemaleRover::Render_Shadow()
{

}

void CLogoFemaleRover::Logo_Input()
{
	// 1번 누르면 선택됨. 두번 누르면 해제됨.
	if (m_pGameInstance->Get_DIKeyState(DIK_1) == KEYSTATE::UP)
	{
		m_States[STATE_PICK] = false;
		m_strCurrentAnimation = "AppearanceIdle";
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_2) == KEYSTATE::UP)
	{
		m_States[STATE_PICK] = true;
		m_strCurrentAnimation = "AppearanceLogin";
	}
	
		
}


void CLogoFemaleRover::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

void CLogoFemaleRover::Ready_Components(const CHARACTER_DESC* pDesc)
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

	m_vColliderOffSet = { 0.f, 0.67f, 0.f };
	m_fColliderRadius = 0.4f;
	m_fColliderHeight = 0.5f;

	// Collider를 Player가 소유하고 Character들은 AddRef로 참조
	CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.vPos = pDesc->vPosition;
	ColliderDesc.vOffset = m_vColliderOffSet;
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	ColliderDesc.fHeight = m_fColliderHeight;
	ColliderDesc.fRadius = m_fColliderRadius;
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Collider"), TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		CRASH("Collider");
}

void CLogoFemaleRover::Ready_Variables(const CHARACTER_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CLogoFemaleRover::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

}


CLogoFemaleRover* CLogoFemaleRover::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLogoFemaleRover* pInstance = new CLogoFemaleRover(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CLogoFemaleRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLogoFemaleRover::Clone(void* pArg)
{
    CLogoFemaleRover* pInstance = new CLogoFemaleRover(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CLogoFemaleRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLogoFemaleRover::Free()
{
    CCharacter::Free();
}
