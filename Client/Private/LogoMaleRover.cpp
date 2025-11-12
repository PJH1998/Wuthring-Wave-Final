#include "ClientPch.h"
#include "LogoFactory.h"
#include "LogoMaleRover.h"
#include "SpringCamera.h"
#include "Collider.h"
#include "GameSystem.h"

CLogoMaleRover::CLogoMaleRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CLogoMaleRover::CLogoMaleRover(const CLogoMaleRover& Prototype)
    : CCharacter(Prototype)
{
}

HRESULT CLogoMaleRover::Initialize_Prototype()
{
    if (FAILED(CCharacter::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CLogoMaleRover::Initialize_Clone(void* pArg)
{
    CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);

    // 1. Player
    if (FAILED(CCharacter::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eCurLevel = pDesc->eCurLevel;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);
	
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 15.f, 0.f, 1.f));

    XMStoreFloat4x4(&m_MatrixIdentity, XMMatrixIdentity());


	// 시작 프레임 하나 실행.
	m_strCurrentAnimation = "AppearanceIdle";
	m_strPreAnimation = m_strCurrentAnimation;
	m_pModelCom->Play_Animation_CPU(m_strCurrentAnimation, 0.f, &m_fTrackPosition, false);
    return S_OK;
}

void CLogoMaleRover::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    // 1. 이전 위치 저장
    m_pTransformCom->Save_PreviousPosition();
  
	Logo_Input();
}

void CLogoMaleRover::Update(_float fTimeDelta)
{
    // 1. 위에서 Activate가 false인경우 업데이트하지 않음.
    if (!m_isActivate)
        return;

	m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(m_strCurrentAnimation, fTimeDelta, &m_fTrackPosition, true);

}
void CLogoMaleRover::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::OUTLINE, this)))
		return;

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
		return;
}

void CLogoMaleRover::Render()
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

void CLogoMaleRover::Render_Shadow()
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

void CLogoMaleRover::Render_OutLine()
{
	Bind_Resources();

	_float4 vColor = _float4(0.2f, 0.2f, 0.2f, 1.f);
	if (FAILED(m_pShaderCom->Bind_Value("g_vOutLineColor", &vColor, sizeof(_float4))))
		return;

	_uint iNumMeshes = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::OUNTLINE))))
			CRASH("Ready Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");
	}
}

void CLogoMaleRover::Logo_Input()
{
	// 1번 누르면 선택됨. 두번 누르면 해제됨.
	if (m_pGameInstance->Get_DIKeyState(DIK_1) == KEYSTATE::UP)
	{
		m_States[STATE_PICK] = true;
		m_strCurrentAnimation = "AppearanceLogin";
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_2) == KEYSTATE::UP)
	{
		m_States[STATE_PICK] = false;
		m_strCurrentAnimation = "AppearanceIdle";
	}
		
}


void CLogoMaleRover::Bind_Resources()
{
    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

void CLogoMaleRover::Ready_Components(const CHARACTER_DESC* pDesc)
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

void CLogoMaleRover::Ready_Variables(const CHARACTER_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());

    for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
        m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::LOGOROVER);
}

void CLogoMaleRover::Ready_Positions(const CHARACTER_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);

}


CLogoMaleRover* CLogoMaleRover::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLogoMaleRover* pInstance = new CLogoMaleRover(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CLogoMaleRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CLogoMaleRover::Clone(void* pArg)
{
    CLogoMaleRover* pInstance = new CLogoMaleRover(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CLogoMaleRover");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLogoMaleRover::Free()
{
    CCharacter::Free();
}
