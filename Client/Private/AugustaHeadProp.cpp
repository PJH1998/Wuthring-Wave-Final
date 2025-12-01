#include "ClientPch.h"
#include "AugustaHeadProp.h"
#include "AttackVolume.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CAugustaHeadProp::CAugustaHeadProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaHeadProp::CAugustaHeadProp(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaHeadProp::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaHeadProp::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);

	/*m_strCurrentAnimName = "Stand1_idle";*/

	CProp::Play_Animation("Stand1_idle", 0.f, nullptr);

	m_fMaxDissolveTime = 0.35f;

    return S_OK;
}

void CAugustaHeadProp::Priority_Update(_float fTimeDelta)
{
    CProp::Priority_Update(fTimeDelta);

	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(PROP_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		if (m_fDissolveTimer <= m_fMaxDissolveTime)
			m_fDissolveTimer += fTimeDelta;
		else
		{
			m_isActivate = false;
			Prop_Reset();
		}
	}

}

void CAugustaHeadProp::Update(_float fTimeDelta)
{
    CProp::Update(fTimeDelta);

    // Augusta StateMachine
	//CProp::Play_Animation(m_strCurrentAnimName, fTimeDelta, nullptr);
	
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(PROP_CONDITION::DISSOLVE));

	if (!IsDissolve) // Dissolve가 아니라면 업데이트 계속.
	{
		XMStoreFloat4x4(&m_CombinedMatrix,
			m_pTransformCom->Get_WorldMatrix() *
			XMLoadFloat4x4(m_pSocketMatrix) *
			m_pParentTransform->Get_WorldMatrix());
	}

	//// Last :  Combined 
	//XMStoreFloat4x4(&m_CombinedMatrix,
	//	m_pTransformCom->Get_WorldMatrix() *
	//	XMLoadFloat4x4(m_pSocketMatrix) *
	//	m_pParentTransform->Get_WorldMatrix());

    _matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);
	
}

void CAugustaHeadProp::Late_Update(_float fTimeDelta)
{
    CProp::Late_Update(fTimeDelta);


    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;
}

void CAugustaHeadProp::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();

	// 1. Dissolve 체크.
	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(PROP_CONDITION::DISSOLVE));
	if (IsDissolve)
	{
		_float fDissolveRate = (m_fDissolveTimer / m_fMaxDissolveTime);
		if (FAILED(m_pShaderCom->Bind_Value("g_fDissolveRate", &fDissolveRate, sizeof(_float))))
			CRASH("Failed Bind Dissolve Rate");

	}


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

        if (FAILED(m_pShaderCom->Begin(m_iShaderPath)))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }
}

void CAugustaHeadProp::Activate(_bool IsActivate)
{
	m_pModelCom->Clear_Animation(m_strCurrentAnimName);

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	if (true == IsActivate)
	{
		Prop_Reset();
		m_isActivate = IsActivate;
		m_iShaderPath = ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON);
	}

	if (false == IsActivate)
	{
		_matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Common_Weapon"), mat, &effecInfo);
		Bind_DissolveTimer();
	}
}


void CAugustaHeadProp::Ready_Components(const PROP_DESC* pDesc)
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

void CAugustaHeadProp::Ready_Variables(const PROP_DESC* pDesc)
{
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

	m_iShaderPath = ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON);
}

void CAugustaHeadProp::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

void CAugustaHeadProp::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaHeadProp* CAugustaHeadProp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaHeadProp* pInstance = new CAugustaHeadProp(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaHeadProp");
    }
    return pInstance;
}

CGameObject* CAugustaHeadProp::Clone(void* pArg)
{
    CAugustaHeadProp* pInstance = new CAugustaHeadProp(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaHeadProp");
    }
    return pInstance;
}

void CAugustaHeadProp::Free()
{
    CProp::Free();
}
