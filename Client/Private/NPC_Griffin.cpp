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
	m_iSoundChannel1 = m_pGameInstance->Register_Channel();
	m_iSoundChannel2 = m_pGameInstance->Register_Channel();
	

	return S_OK;
}

void CNPC_Griffin::Priority_Update(_float fTimeDelta)
{
}

void CNPC_Griffin::Update(_float fTimeDelta)
{
	_bool m_IsTrash = { false };
	m_IsRender = false;
	if (XMVectorGetX(XMVector3Length(XMVectorSetY(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - m_pTransformCom->Get_State(STATE::POSITION), 0.f))) < 500.f)
	{
		m_pAnimMachine->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iAnimState, m_IsTrash, fTimeDelta);
		m_IsRender = true;
	}
	if (m_IsTrash)
		m_iAnimState = m_pGameInstance->Rand(0.f, 3.f - XMVectorGetX(g_XMEpsilon));
}

void CNPC_Griffin::Late_Update(_float fTimeDelta)
{
	if (m_IsRender)
	{
		m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this);
		m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);
	}
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

void CNPC_Griffin::Render_Shadow()
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

void CNPC_Griffin::Object_Func(const _wstring& wStrObjectTag)
{
	if (wStrObjectTag == TEXT("Idle_02"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel2);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_stand_roar_02 (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_stand_roar_01 (SFX)"), m_iSoundChannel2, 1.f, m_pTransformCom, 0.f, 200.f);
	}
	else if (wStrObjectTag == TEXT("Idle_End"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_stand_end (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);

	}
	else if (wStrObjectTag == TEXT("Idle_Start"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_sit_start (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);

	}
	else if (wStrObjectTag == TEXT("Interact"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel2);

		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_bow_salute_02 (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_bow_salute_01 (SFX)"), m_iSoundChannel2, 1.f, m_pTransformCom, 0.f, 200.f);

	}
	else if (wStrObjectTag == TEXT("Land"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_land (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);

	}
	else if (wStrObjectTag == TEXT("Stand_Fly"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_stand_fly (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);

	}
	else if (wStrObjectTag == TEXT("Takeoff"))
	{
		m_pGameInstance->Stop_Sound_Dynamic(m_iSoundChannel1);
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_shouwangjiu_takeoff (SFX)"), m_iSoundChannel1, 1.f, m_pTransformCom, 0.f, 200.f);

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

	Register_AllNotifies(pDesc->strFolderPath);
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
	m_pGameInstance->Return_Channel(m_iSoundChannel1);
	m_pGameInstance->Return_Channel(m_iSoundChannel2);
	Safe_Release(m_pAnimMachine);
}
