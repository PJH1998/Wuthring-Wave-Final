#include "ClientPch.h"
#include "AugustaEnergyBlade.h"
#include "GameSystem.h"
#include "PlayerStatus.h"
#include "Ability.h"

CAugustaEnergyBlade::CAugustaEnergyBlade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CProp{ pDevice, pContext }
{
}

CAugustaEnergyBlade::CAugustaEnergyBlade(const CPartObject& Prototype)
    : CProp(Prototype )
{
}

HRESULT CAugustaEnergyBlade::Initialize_Prototype()
{
    if (FAILED(CProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CAugustaEnergyBlade::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);

	m_pTransformCom->Scale({ 2.f, 2.f, 2.f }); // 크기 조정.

	m_fTime = 0.f;
	m_vScrollSpeed = { 1.f, 0.f };
	m_vEmissiveColor = { 0.3f, 0.1f, 0.05f, 1.0f };
	m_fEmissiveIntensity = 3.f;
	m_vDissolveColor = { 0.5f, 0.2f, 0.1f, 1.f };

	m_fMaxDissolveTime = 0.35f;

	m_isActivate = false;

    return S_OK;
}

void CAugustaEnergyBlade::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

    CProp::Priority_Update(fTimeDelta);

	m_fTime += fTimeDelta;

	// Dissolve 체크.
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

#ifdef _DEBUG

#endif // _DEBUG


}

void CAugustaEnergyBlade::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

    CProp::Update(fTimeDelta);

	

	_bool IsDissolve = Check_AnyCondition(ENUM_CLASS(PROP_CONDITION::DISSOLVE));

    // Augusta StateMachine
	_matrix matParentWorld = XMMatrixIdentity();

	if (nullptr != m_pParentWorldMatrix)
		matParentWorld = XMLoadFloat4x4(m_pParentWorldMatrix);
	else if (nullptr != m_pParentTransform)
		matParentWorld = m_pParentTransform->Get_WorldMatrix();
	//matParentWorld = m_pParentTransform->Get_WorldMatrix();

	// Last :  Combined 
	if (!IsDissolve)
	{
		XMStoreFloat4x4(&m_CombinedMatrix,
			m_pTransformCom->Get_WorldMatrix() *
			XMLoadFloat4x4(m_pSocketMatrix) *
			matParentWorld);
	}
	

    _matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);

}

void CAugustaEnergyBlade::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

    CProp::Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
        return;

	//if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::EMISSIVE, this)))
	//	return;
}

void CAugustaEnergyBlade::Render()
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

		
		if (FAILED(m_pShaderCom->Bind_Value("g_vEmissiveColor", &m_vEmissiveColor, sizeof(_float4))))
			CRASH("Ready EnergyColor");

		if (FAILED(m_pShaderCom->Bind_Value("g_vDissolveColor", &m_vDissolveColor, sizeof(_float4))))
			CRASH("Ready EnergyColor");

		if (FAILED(m_pShaderCom->Bind_Value("g_fTime", &m_fTime, sizeof(_float))))
			CRASH("Ready EnergyColor");

		if (FAILED(m_pShaderCom->Bind_Value("g_vScrollSpeed", &m_vScrollSpeed, sizeof(_float2))))
			CRASH("Ready EnergyColor"); 

		if (FAILED(m_pShaderCom->Bind_Value("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
			CRASH("Ready EnergyColor");

		

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        //if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
        //    CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pShaderCom->Begin(m_iShaderPath)))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }
}

void CAugustaEnergyBlade::Activate(_bool IsActivate)
{
 //   SetActivate(IsActivate);

	////m_pModelCom->Clear_Animation(m_strCurrentAnimName); // 애니메이션 클리어

	//PREFAB_INFO effecInfo{};
	//effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	//effecInfo.pModelPtr = m_pModelCom;

	//if (false == IsActivate)
	//{
	//	m_fTime = 0.f;
	//	_matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);
	//	m_pGameInstance->Spawn_PoolingObject(TEXT("Common_Weapon"), mat, &effecInfo);
	//}

	m_pModelCom->Clear_Animation(m_strCurrentAnimName);

	PREFAB_INFO effecInfo{};
	effecInfo.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	effecInfo.pModelPtr = m_pModelCom;

	if (true == IsActivate)
	{
		Prop_Reset();
		m_isActivate = IsActivate;
		m_iShaderPath = ENUM_CLASS(SHADER_PROPANIMMESH::ENERGY_BLADE);
	}
	if (false == IsActivate)
	{
		_matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Common_Weapon"), mat, &effecInfo);
		Bind_DissolveTimer(ENUM_CLASS(SHADER_PROPANIMMESH::DISSOLVE_AUGUSTAWEAPON));
	}
}



void CAugustaEnergyBlade::OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	// 게이지 올리기?
	CAbility* pAbility = CGameSystem::GetInstance()
		->Get_PlayerStatus()->Get_Ability(ENUM_CLASS(UI_CHARACTERTYPE::AUGUSTA));

	if (nullptr == pAbility)
		return;
}

void CAugustaEnergyBlade::Ready_Components(const PROP_DESC* pDesc)
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

void CAugustaEnergyBlade::Ready_Variables(const PROP_DESC* pDesc)
{
    m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;

	//for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
	//	m_ShaderPaths[i] = ENUM_CLASS(SHADER_PROPANIMMESH::DEFAULT_WEAPON);

	m_iShaderPath = ENUM_CLASS(SHADER_PROPANIMMESH::ENERGY_BLADE);
}

void CAugustaEnergyBlade::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}


void CAugustaEnergyBlade::Bind_Resources()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        CRASH("Failed Bind Matrix");

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        CRASH("Failed Proj Matrix");
}

CAugustaEnergyBlade* CAugustaEnergyBlade::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAugustaEnergyBlade* pInstance = new CAugustaEnergyBlade(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CAugustaEnergyBlade");
    }
    return pInstance;
}

CGameObject* CAugustaEnergyBlade::Clone(void* pArg)
{
    CAugustaEnergyBlade* pInstance = new CAugustaEnergyBlade(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CAugustaEnergyBlade");
    }
    return pInstance;
}

void CAugustaEnergyBlade::Free()
{
    CProp::Free();
}
