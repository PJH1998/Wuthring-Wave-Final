#include "ClientPch.h"
#include "Levi_Bow.h"
#include "AttackVolume.h"

CLevi_Bow::CLevi_Bow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject { pDevice, pContext }
{
}

CLevi_Bow::CLevi_Bow(const CLevi_Bow& Prototype)
	: CPartObject { Prototype }
{
}

HRESULT CLevi_Bow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLevi_Bow::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	LEVIBOW_DESC* pDesc = static_cast<LEVIBOW_DESC*>(pArg);
	m_pParentTransform = pDesc->pParentTransform;
	m_pSocketMatrix = pDesc->pSocketMatrix;

	Ready_Component(pDesc);

#ifdef _DEBUG
	m_vOffsetPos = pDesc->vOffsetPos;
	m_vOffsetRot = pDesc->vOffsetRadian;
#else
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(pDesc->vOffsetRadian.x, pDesc->vOffsetRadian.y, pDesc->vOffsetRadian.z),
		XMVectorSetW(XMLoadFloat3(&pDesc->vOffsetPos), 1.f));
	XMStoreFloat4x4(&m_OffsetMatrix, matOffset);
#endif // _DEBUG

	m_ShaderPaths.resize(SHADERPATH::END, ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));



	return S_OK;
}

void CLevi_Bow::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
}

void CLevi_Bow::Update(_float fTimeDelta)
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

	m_pModelCom->Play_Animation_CPU("Stand2_Ex", fTimeDelta, nullptr, false, false, false, false);
}

void CLevi_Bow::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CLevi_Bow::Render()
{
	if (FAILED(Bind_Resources()))
		CRASH("Failed to Bind Resources (Levi_Bayonet)");

	_uint iNumMeshes = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
			CRASH("Ready Diffuse Texture Failed");

		_bool HasNormal = { false };

		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(m_ShaderPaths[i])))
			CRASH("Ready Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");

		m_pShaderCom->UndBind_All_VS_SRV();
	}
#ifdef _DEBUG
	_float4 temp{};
	m_pGameInstance->Ray_Cast(m_pTransformCom->Get_State(STATE::POSITION), m_pTransformCom->Get_State(STATE::POSITION) + XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)), &temp);
#endif // _DEBUG

}

void CLevi_Bow::Change_Offset(LEVIBOW_DESC& Desc)
{
#ifdef _DEBUG
	m_vOffsetPos = Desc.vOffsetPos;
	m_vOffsetRot = Desc.vOffsetRadian;
#else
	_matrix matOffset = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f),
		XMQuaternionRotationRollPitchYaw(Desc.vOffsetRadian.x, Desc.vOffsetRadian.y, Desc.vOffsetRadian.z),
		XMVectorSetW(XMLoadFloat3(&Desc.vOffsetPos), 1.f));
	XMStoreFloat4x4(&m_OffsetMatrix, matOffset);
#endif // _DEBUG

}

HRESULT CLevi_Bow::Bind_Resources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Proj Matrix");

	return S_OK;
}

void CLevi_Bow::Ready_Component(LEVIBOW_DESC* pDesc)
{
	// 1. Components
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel()
		, TEXT("Prototype_Component_Model_Leviatan_Bow"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");
}

void CLevi_Bow::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
}

CLevi_Bow* CLevi_Bow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Bow* pInstance = new CLevi_Bow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Bow");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Bow::Clone(void* pArg)
{
	CLevi_Bow* pClone = new CLevi_Bow(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Bow (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Bow::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
