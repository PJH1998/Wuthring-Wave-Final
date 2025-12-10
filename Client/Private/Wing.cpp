#include "ClientPch.h"
#include "Wing.h"

CWing::CWing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CProp { pDevice, pContext }
{
}

CWing::CWing(const CPartObject& Prototype)
	: CProp (Prototype)
{
}

HRESULT CWing::Initialize_Prototype()
{
	if (FAILED(CProp::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CWing::Initialize_Clone(void* pArg)
{
	PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
	ASSERT_CRASH(pDesc);

	if (FAILED(CPartObject::Initialize_Clone(pDesc)))
		return E_FAIL;

	Ready_Components(pDesc);
	Ready_Variables(pDesc);
	Ready_Positions(pDesc);
	Register_AllNotifies(pDesc->strFolderPath);

    return S_OK;
}

void CWing::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	CProp::Priority_Update(fTimeDelta);
}

void CWing::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	CProp::Update(fTimeDelta);


	//m_pRigidbodyCom->Update_Rigidbody(mat, fTimeDelta);
}

void CWing::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	//Wing은 본체 Transform이 완전히 확정된 후에 소켓 행렬을 갱신
	XMStoreFloat4x4(&m_CombinedMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		XMLoadFloat4x4(m_pSocketMatrix) *
		m_pParentTransform->Get_WorldMatrix());

	_matrix mat = XMLoadFloat4x4(&m_CombinedMatrix);

	// 호출 순서. Character Update -> Activate 상태라면-> WingUpdate(행렬 및 RigidBody 갱신) -> StateMachine Update 
	// -> m_pSocketMatrix에 뼈 행렬 포인터 전달. -> Animation 실행. -> 캐릭터 Update  종료
	//CProp::Late_Update(fTimeDelta);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CWing::Render()
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

}

void CWing::Activate(_bool IsActivate)
{
	//CProp::Activate(IsActivate);
	if (false == IsActivate)
	{
		m_isActivate = false;
	}


	else if (true == IsActivate)
	{
		m_isActivate = true;

		SPECTRUM_INFO RightSpectrum{};
		RightSpectrum.pModelMarixPtr = &m_CombinedMatrix;
		RightSpectrum.pBoneMatrixPtr = m_pModelCom->Get_BoneMatrixPtr("Bone_Prop009_R");
		RightSpectrum.pIsActive = &m_isActivate;

		_matrix mat = XMMatrixIdentity();
			m_pGameInstance->Spawn_PoolingObject(TEXT("tat"), mat, &RightSpectrum);

		SPECTRUM_INFO LeftSpectrum{};
		LeftSpectrum.pModelMarixPtr = &m_CombinedMatrix;
		LeftSpectrum.pBoneMatrixPtr = m_pModelCom->Get_BoneMatrixPtr("Bone_Prop009_L");
		LeftSpectrum.pIsActive = &m_isActivate;

		m_pGameInstance->Spawn_PoolingObject(TEXT("tat"), mat, &LeftSpectrum);
	}

}

void CWing::Ready_Components(const PROP_DESC* pDesc)
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

void CWing::Ready_Variables(const PROP_DESC* pDesc)
{
	m_ShaderPaths.resize(m_pModelCom->Get_NumMesh());
	m_pSocketMatrix = pDesc->pSocketMatrix;
	m_pParentTransform = pDesc->pParentTransform;

	XMStoreFloat4x4(&m_CombinedMatrix, XMMatrixIdentity());

	for (_uint i = 0; i < m_ShaderPaths.size(); ++i)
		m_ShaderPaths[i] = ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX);
}

void CWing::Ready_Positions(const PROP_DESC* pDesc)
{
	_fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Scale(pDesc->vScale);
}

void CWing::Bind_Resources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Proj Matrix");
}

CWing* CWing::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWing* pInstance = new CWing(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Create Failed CWing");
	}
	return pInstance;
}

CGameObject* CWing::Clone(void* pArg)
{
	CWing* pInstance = new CWing(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Create Failed CWing");
	}
	return pInstance;
}

void CWing::Free()
{
	CProp::Free();
}
