#include "ClientPch.h"
#include "Levi_Bayonet.h"
#include "AttackVolume.h"

CLevi_Bayonet::CLevi_Bayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject { pDevice, pContext }
{
}

CLevi_Bayonet::CLevi_Bayonet(const CLevi_Bayonet& Prototype)
	: CPartObject { Prototype }
{
}

HRESULT CLevi_Bayonet::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLevi_Bayonet::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	LEVIBAYONET_DESC* pDesc = static_cast<LEVIBAYONET_DESC*>(pArg);
	m_pParentTransform = pDesc->pParentTransform;
	m_pSocketMatrix = pDesc->pSocketMatrix;

	Ready_Component(pDesc);
	Ready_Volumes(pDesc);

#ifdef _DEBUG
	m_vOffsetPos = pDesc->vOffsetPos;
	m_vOffsetRot = pDesc->vOffsetRadian;
#else
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(pDesc->vOffsetRadian.x, pDesc->vOffsetRadian.y, pDesc->vOffsetRadian.z),
		XMVectorSetW(XMLoadFloat3(&pDesc->vOffsetPos), 1.f));
	XMStoreFloat4x4(&m_OffsetMatrix, matOffset);
#endif // _DEBUG

	m_ShaderPaths.resize(SHADERPATH::END);
	m_ShaderPaths[SHADERPATH::FX] = 1; // Shader_VtxMesh_MonsterProp Pass 1
	m_vBaseColor = pDesc->vBaseColor;
	return S_OK;
}

void CLevi_Bayonet::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
}

void CLevi_Bayonet::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(m_vOffsetRot.x, m_vOffsetRot.y, m_vOffsetRot.z), XMVectorSetW(XMLoadFloat3(&m_vOffsetPos), 1.f));
#else
	_matrix matOffset = XMLoadFloat4x4(&m_OffsetMatrix);
#endif // _DEBUG

	_matrix ComBinedMatrix;
	_matrix NonScaleMatrix = XMLoadFloat4x4(m_pSocketMatrix);
	_vector vScale, vQuaternion, vTransition;
	XMMatrixDecompose(&vScale, &vQuaternion, &vTransition, NonScaleMatrix);
	NonScaleMatrix = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuaternion, vTransition);
	ComBinedMatrix = matOffset * NonScaleMatrix * m_pParentTransform->Get_WorldMatrix();
	XMStoreFloat4x4(&m_CombinedMatrix, ComBinedMatrix);
	m_pTransformCom->Set_WorldMatrix(ComBinedMatrix);

	if (m_pAttackVolume)
		m_pAttackVolume->Update(fTimeDelta);
}

void CLevi_Bayonet::Late_Update(_float fTimeDelta)
{
	m_fRateFX = fmod(m_fRateFX + fTimeDelta, 1.f);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
		return;
}

void CLevi_Bayonet::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Failed to Bind Resources (Levi_Bayonet)");

	_uint iNumMeshes = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	if (FAILED(m_pShaderCom->Bind_Value("g_fFxTime", &m_fRateFX, sizeof(_float))))
		CRASH("Failed to Bind fFxTime ");

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
			CRASH("Ready Diffuse Texture Failed");

		_bool HasNormal = { false };

		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
			CRASH("Ready Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");

		m_pShaderCom->UndBind_All_VS_SRV();
	}

#ifdef _DEBUG
	if (m_pAttackVolume)
		m_pAttackVolume->Render();
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif // _DEBUG

}

void CLevi_Bayonet::Render_Shadow()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pShaderCom->Begin(2);

		m_pModelCom->Render(i);
	}
}

void CLevi_Bayonet::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	//LEVI_BY_RESET* pDesc = static_cast<LEVI_BY_RESET*>(pArg);
	m_pAttackVolume->TriggerActivate(false);
	//m_pAttackVolume->Change_Layer(pDesc->eLayer);
}

void CLevi_Bayonet::Attack_Active(_bool isActive)
{
	m_pAttackVolume->TriggerActivate(isActive);
}

void CLevi_Bayonet::Change_Layer(COLLISIONLAYER eLayer)
{
	m_pAttackVolume->Change_Layer(eLayer);
}

HRESULT CLevi_Bayonet::Bind_Resources()
{
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
	//	CRASH("Failed Bind Matrix");
	if(FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed Bind Matrix");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Proj Matrix");

	return S_OK;
}

void CLevi_Bayonet::Ready_Component(LEVIBAYONET_DESC* pDesc)
{
	// 1. Components
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Shader_MonsterProp"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel()
		, TEXT("Prototype_Component_Model_Leviatan_Bayonet"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");
}

void CLevi_Bayonet::Ready_Volumes(LEVIBAYONET_DESC* pDesc)
{
	CAttackVolume::ATKVOLUME_DESC TriggerDesc;
	TriggerDesc.eLayer = COLLISIONLAYER::ENEMY_ATTACK;
	TriggerDesc.eTargetLayer = COLLISIONLAYER::PLAYER;
	TriggerDesc.eShape = SHAPE::BOX;
	TriggerDesc.eType = CAttackVolume::COMBINED_TYPE::PROP;
	TriggerDesc.pParenTransform = m_pTransformCom;
	TriggerDesc.pSocketMatrix = &m_CombinedMatrix;
	TriggerDesc.vExtent = _float3(1.5f, 0.4f, 0.4f);
	TriggerDesc.vOffsetPos = _float3(1.f, 0.f, 0.f);
	TriggerDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));
	TriggerDesc.fAttackDmg = pDesc->fAttackDmg;
	TriggerDesc.eDamageType = pDesc->eType;
	TriggerDesc.CollisionCallback = [this](_uint iLayer, void* pOther, const ContactManifold& Manifold) {
		this->OnCollide_Enter(iLayer, pOther, Manifold);
		};
	
	m_pAttackVolume = dynamic_cast<CAttackVolume*>(m_pGameInstance->Clone_Prototype(m_pGameInstance->Get_CurrentLevel(), TEXT("Prototype_GameObject_AttackVolume"), PROTOTYPE::GAMEOBJECT, &TriggerDesc));
	if (nullptr == m_pAttackVolume)
		CRASH(m_pAttackVolume);
	m_pAttackVolume->TriggerActivate(false);
}

void CLevi_Bayonet::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
#ifdef _DEBUG
		cout << "On Hit! (Levi Bayonet)" << endl;
#endif // _DEBUG
	}
}

CLevi_Bayonet* CLevi_Bayonet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Bayonet* pInstance = new CLevi_Bayonet(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Bayonet");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Bayonet::Clone(void* pArg)
{
	CLevi_Bayonet* pClone = new CLevi_Bayonet(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Bayonet (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Bayonet::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAttackVolume);
}
