#include "ClientPch.h"
#include "Napal.h"

CNapal::CNapal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CNapal::CNapal(const CNapal& Prototype)
	: CActor { Prototype }
{
}

HRESULT CNapal::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CNapal::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	NAPALDESC* pDesc = static_cast<NAPALDESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPos), 1.f));
	m_pTransformCom->Rotation_Quaternion(pDesc->vInitRot);

	Ready_Component(pDesc);

	m_vBaseColor = _float4(1.f, 1.f, 1.f, 1.f);
	m_strAnimationTag[0] = "SK_Tab_Mon_01AL_Idle_01";
	m_strAnimationTag[1] = "SK_Tab_Mon_01AL_Stand";
	//m_isActivate = false;
	m_isRender = true;
	m_iSoundChannel = m_pGameInstance->Register_Channel();
	Register_AllNotifies(pDesc->strFolderPath);
    return S_OK;
}

void CNapal::Priority_Update(_float fTimeDelta)
{
}

void CNapal::Update(_float fTimeDelta)
{
	_bool isAnimationFinished{ false };
	_float fTrackPosition{};
	isAnimationFinished = m_pModelCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom, m_strAnimationTag[m_iIndex], fTimeDelta, &fTrackPosition, false);
	if (isAnimationFinished)
	{
		if (m_iIndex > 0)
		{
			m_iCount++;
		}
		else
		{
			m_iIndex = 1;
			m_iCount = 0;
		}
		if (m_iCount > 3)
		{
			m_iIndex = 0;
		}
	}
	
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);

}

void CNapal::Late_Update(_float fTimeDelta)
{
	if (m_isRender)
	{
		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
			return;

		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
			return;
	}
}

void CNapal::Render()
{
	if (FAILED(Bind_Resources()))
		return;

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	m_pShaderCom->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4));

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
			HasNormal = true;
		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::NAPAL));
		
		m_pModelCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
	}
}

void CNapal::Render_Shadow()
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

void CNapal::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->Save_PreviousPosition();
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));
	m_pColliderCom->IsActivate(true);
	m_pColliderCom->Set_Gravity(true);
	m_pRigidBodyCom->IsActivate(true);
	m_isActivate = true;
}

void CNapal::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
}

void CNapal::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CNapal::Object_Func(const _wstring& wStrObjectTag)
{
	if (wStrObjectTag == TEXT("Sound"))
	{
		m_pGameInstance->Play_Sound_Dynamic(TEXT("amb_npc_alien_horn_01 (SFX)"), m_iSoundChannel, 0.4f ,m_pTransformCom, 1.f, 250.f);
	}
}

void CNapal::Sound_Active(const _wstring& wStrObjectTag)
{
}

HRESULT CNapal::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_vBaseColor", &m_vBaseColor, sizeof(_float4));
    return S_OK;
}

void CNapal::Ready_Component(NAPALDESC* pDesc)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("NPC_Hiding/Com_Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("NPC_Hiding/Com_ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("NPC_Hiding/Com_Model");
	//m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
}

CNapal* CNapal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNapal* pInstance = new CNapal(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CNapal");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNapal::Clone(void* pArg)
{
	CNapal* pClone = new CNapal(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CNapal (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CNapal::Free()
{
	__super::Free();

}
