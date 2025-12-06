#include"ClientPch.h"
#include "NPC_Griffin.h"

CNPC_Griffin::CNPC_Griffin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CActor(pDevice,pContext)
{
}

CNPC_Griffin::CNPC_Griffin(const CNPC_Griffin& Prototype)
	:CActor(Prototype)
{
}

HRESULT CNPC_Griffin::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNPC_Griffin::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Component(static_cast<GRIFFIN_DESC*>(pArg));

	m_pAnimMachine->Reset(m_pModelCom, "Idle_Start");
	return S_OK;
}

void CNPC_Griffin::Priority_Update(_float fTimeDelta)
{
}

void CNPC_Griffin::Update(_float fTimeDelta)
{
	_bool m_IsTrash = { false };

	m_pAnimMachine->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iAnimState, m_IsTrash, fTimeDelta);
	
	if (m_IsTrash)
		m_iAnimState = m_pGameInstance->Rand(0.f, 3.f - XMVectorGetX(g_XMEpsilon));
}

void CNPC_Griffin::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this);
}

void CNPC_Griffin::Render()
{
	Bind_Resources();
	for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
	{
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);

		m_pShaderCom->Begin(0);
		m_pModelCom->Render(i);
	}
}

HRESULT CNPC_Griffin::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	return S_OK;
}

void CNPC_Griffin::Ready_Component(GRIFFIN_DESC* pDesc)
{
	pDesc->shaderData;
	pDesc->modelData;
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->pTransformMatrix));
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 0.5f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NPC);
	ColliderDesc.fHeight = 0.9f;
	ColliderDesc.fRadius = 0.4f;
	//Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	//ASSERT_CRASH(m_pColliderCom);

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Griffin/Com_Shader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("NPC_Hiding/Com_Model");
	//m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
	

		// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->computeShaderData.first), pDesc->computeShaderData.second,
		TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("NPC_Hiding/Com_Model");

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag.assign("Idle_Start");

	//Com_AnimMachine
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), pDesc->pAnimMachineTag,
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachine), &AnimMachineDesc)))
		CRASH("NPC_Hiding/Com_AnimMachine");
}

void CNPC_Griffin::Ready_InstanceCells(GRIFFIN_DESC* pDesc)
{
}

CNPC_Griffin* CNPC_Griffin::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNPC_Griffin* pInstance = new CNPC_Griffin(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype())) {
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNPC_Griffin::Clone(void* pArg)
{
	CNPC_Griffin* pInstance = new CNPC_Griffin(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg))) {
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNPC_Griffin::Free()
{
	__super::Free();
	Safe_Release(m_pAnimMachine);
}
