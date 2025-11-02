#include "ClientPch.h"
#include "Ggobul.h"

CGgobul::CGgobul(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CGgobul::CGgobul(const CGgobul& Prototype)
	:CGameObject { Prototype }
{
}

HRESULT CGgobul::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CGgobul::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	GGOBUL_DESC* pDesc = static_cast<GGOBUL_DESC*>(pArg);
	Ready_Component(pDesc);
	Register_AllNotifies(pDesc->strFolderPath);

	for (size_t i = 0; i < GGOBULTYPE::END; ++i)
	{
		m_pRigidBodyCom[i]->IsActivate(false);
	}

	m_MeshEnables.resize(m_pModelCom->Get_NumMesh(), true);
    return S_OK;
}

void CGgobul::Priority_Update(_float fTimeDelta)
{
}

void CGgobul::Update(_float fTimeDelta)
{
	_float fTrackPos{};
	if (m_pModelCom->Play_Animation_CPU(m_strAnimKey, fTimeDelta * m_fAnimationSpeed, &fTrackPos,
		m_isRootMotion, m_isRootMotionRotate, m_isRootMotionTranslate, m_fRootMotionRate))
	{
		m_pModelCom->Clear_Animation(m_strAnimKey);
		m_isActivate = false;
		return;
	}
	_matrix NonScaleMatrix = XMLoadFloat4x4(m_pAttackTransform) * m_pTransformCom->Get_WorldMatrix();
	_vector vScale, vQuaternion, vTransition;
	XMMatrixDecompose(&vScale, &vQuaternion, &vTransition, NonScaleMatrix);
	NonScaleMatrix = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vQuaternion, vTransition);
	XMStoreFloat4x4(&m_CombindMatrix, NonScaleMatrix);
	m_pRigidBodyCom[m_eType]->Update_Rigidbody(XMLoadFloat4x4(&m_CombindMatrix), fTimeDelta);
}

void CGgobul::Late_Update(_float fTimeDelta)
{
	m_pRigidBodyCom[m_eType]->Sync_Rigidbody(m_pTransformCom);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CGgobul::Render()
{
	Bind_Resources();

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (false == m_MeshEnables[i])
			continue;
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}
#ifdef _DEBUG
#endif
}

void CGgobul::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	GGOBUL_RESET* pDesc = static_cast<GGOBUL_RESET*>(pArg);
	m_eType = pDesc->eType;
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->pWorldMatrix));
	m_isRootMotion = pDesc->isRootMotion;
	m_isRootMotionRotate = pDesc->isRootMotionRotate;
	m_isRootMotionTranslate = pDesc->isRootMotionTranslate;
	m_fRootMotionRate = pDesc->fRootMotionRate;
	m_fAnimationSpeed = pDesc->fAnimationSpeed;

	for (_uint i = 0; i < GGOBULTYPE::END; ++i)
	{
		m_pRigidBodyCom[i]->IsActivate(false);
	}
}

void CGgobul::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CGgobul::Ready_Component(GGOBUL_DESC* pDesc)
{
	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(pDesc->modelData.first, pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");

	// Com_Rigidbody(Head)
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RigidbodyDesc.vExtent = _float3(4.f, 4.f, 4.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(pDesc->rigidBodyData.first, pDesc->rigidBodyData.second,
		TEXT("Com_Rigidbody_Head"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom[GGOBULTYPE::HEAD]), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom[GGOBULTYPE::HEAD]->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	// Com_Rigidbody(Hammer)
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RigidbodyDesc.vExtent = _float3(4.f, 4.f, 4.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(pDesc->rigidBodyData.first, pDesc->rigidBodyData.second,
		TEXT("Com_Rigidbody_Head"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom[GGOBULTYPE::HAMMER]), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom[GGOBULTYPE::HAMMER]->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});

	// Com_Rigidbody(Knife)
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY_ATTACK);
	RigidbodyDesc.vExtent = _float3(4.f, 4.f, 4.f);
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(pDesc->rigidBodyData.first, pDesc->rigidBodyData.second,
		TEXT("Com_Rigidbody_Head"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom[GGOBULTYPE::KNIFE]), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom[GGOBULTYPE::KNIFE]->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_Enter(iLayer, pDesc, Manifold);
		});
}

void CGgobul::Register_AllNotifies(const _string& strFolderPath)
{
	ASSERT_CRASH(m_pModelCom);
	auto colliderCallback = [this](const _wstring& tag, bool active) {
		this->Collider_Active(tag, active);
		};

	auto effectCallBack = [this](const _wstring& tag) {
		this->Effect_Active(tag);
		};

	m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack);
}

void CGgobul::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	if(wStrColliderTag == L"Attack Trig")
		m_pRigidBodyCom[m_eType]->IsActivate(Isactive);
}

void CGgobul::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CGgobul::OnCollide_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

CGgobul* CGgobul::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGgobul* pInstance = new CGgobul(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CGgobul");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGgobul::Clone(void* pArg)
{
	CGgobul* pClone = new CGgobul(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CGgobul (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CGgobul::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pComputeShaderCom);
	for (size_t i = 0; i < GGOBULTYPE::END; ++i)
	{
		Safe_Release(m_pRigidBodyCom[i]);
	}

	Safe_Release(m_pModelCom);
}
