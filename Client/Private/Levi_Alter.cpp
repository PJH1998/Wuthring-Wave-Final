#include "ClientPch.h"
#include "Levi_Alter.h"
#include "Levi_Bayonet.h"

CLevi_Alter::CLevi_Alter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CLevi_Alter::CLevi_Alter(const CLevi_Alter& Prototype)
	: CActor { Prototype }
{
}

HRESULT CLevi_Alter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLevi_Alter::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

    return S_OK;
}

void CLevi_Alter::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Priority_Update(fTimeDelta);
	}
}

void CLevi_Alter::Update(_float fTimeDelta)
{
	_vector vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetPos = XMVectorSetW(XMLoadFloat3(&m_vTargetPosition), 1.f);
	_vector vDir = vTargetPos - vPosition;
	m_fDistanceNonY = XMVectorGetX(XMVector3Length(XMVectorSetY(vTargetPos, 0.f) - XMVectorSetY(vPosition, 0.f)));
	vDir = XMVector3Normalize(vDir);
	XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(XMVectorSetY(vDir, 0.f)));

	_float fTrackPos{};
	m_pModelCom->Play_NonRibAnimation_GPU(m_pComputeShaderCom, m_strAnimKey, fTimeDelta, &fTrackPos, true, false, true, 1.f);
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);

	_vector vVelocity = m_pTransformCom->Get_Velocity();
	if (m_isDist_Interp_Enable)
	{
		_float temp = clamp(m_fDistanceNonY - 1.5f, 0.f, 1.f);
		m_pColliderCom->Update(vVelocity / fTimeDelta * temp);
	}
	else
		m_pColliderCom->Update(vVelocity / fTimeDelta);



	//5. 파츠 갱신
	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Update(fTimeDelta);
	}
}

void CLevi_Alter::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this))) return;
	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Late_Update(fTimeDelta);
	}

}

void CLevi_Alter::Render()
{
	Bind_Resources();

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		_bool HasMask = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, 0)))
			HasMask = true;

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		if (FAILED(m_pShaderCom->Bind_Value("g_HasSkinMask", &HasMask, sizeof(_bool))))
			CRASH("Ready g_HasSkinMask Failed");

		if (FAILED(m_pModelCom->Bind_MorphedResult(m_pShaderCom, i, "g_MorphedVertices")))
			CRASH("Bind Morph Result Failed");

		m_pShaderCom->Begin(m_ShaderIndices[i]);

		m_pModelCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
	}
}

void CLevi_Alter::Render_Shadow()
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

void CLevi_Alter::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	ALTER_RESET* pDesc = static_cast<ALTER_RESET*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));
	m_pTransformCom->LookAt_KeepUp(XMVectorSetW(XMLoadFloat3(&pDesc->vLookAt),1.f));
	m_strAnimKey = pDesc->strPatternKey;
	//m_pAnimMachineCom->Reset(m_pModelCom, m_strAnimKey);
	m_pModelCom->Clear_Animation(m_strAnimKey);
	m_pModelCom->Set_TrackPosition(m_strAnimKey, m_Tracks[m_strAnimKey].first);
	if (pDesc->eType == ATTACK_TYPE::SWORD)
	{

	}
	else if (pDesc->eType == ATTACK_TYPE::BOW)
	{

	}
}

void CLevi_Alter::Collider_Active(const _wstring& wStrColliderTag, _bool Isactive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);
	if (wstrTypeTag == TEXT("Attack"))
	{

	}
	else if (wstrTypeTag == TEXT("Gravity"))
	{
		m_pColliderCom->Set_Gravity(Isactive);
	}
	else if (wstrTypeTag == TEXT("Lerp"))
	{
		m_isTurnLerp = Isactive;
	}
	else if (wstrTypeTag == TEXT("Distance"))
	{
		m_isDist_Interp_Enable = Isactive;
	}
}

void CLevi_Alter::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	PREFAB_INFO EffectDesc{};
	EffectDesc.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	EffectDesc.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &EffectDesc);
}

void CLevi_Alter::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrAnimTag = wStrObjectTag.substr(Index + 1);

	if (wstrTypeTag == TEXT("Look"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir));
	}
	else if (wstrTypeTag == TEXT("LookRev"))
	{
		m_pTransformCom->LookDir(XMLoadFloat3(&m_vTargetDir) * -1.f);
		_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(0.f), 0.f);
		m_pTransformCom->Turn_Quaternion(vQuat);
	}
	//else if (wstrTypeTag == TEXT("Reset"))
	//{
	//	Reset_NotifyInteraction();
	//}
}

void CLevi_Alter::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CLevi_Alter::Ready_Component(ALTER_DESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vDetectRange;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnDetect_During(iLayer, pDesc, Manifold);
		});

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 1.35f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 1.8f;
	ColliderDesc.fRadius = 0.4f;
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);
	m_CallBack.pTransform = m_pTransformCom;
	m_CallBack.fAttack = m_fAttackDmg;
	//m_tCallDesc.strEffectTag = ;
	m_CallBack.eType = TEXT_COLOR_TYPE::DARK;

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->shaderData.first), pDesc->shaderData.second,
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->computeShaderData.first)
		, pDesc->computeShaderData.second, TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("ComputeShader");

	// Com_Model
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->modelData.first), pDesc->modelData.second,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
}

void CLevi_Alter::Ready_PartObject(ALTER_DESC* pDesc)
{
}

void CLevi_Alter::OnHit_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
}

void CLevi_Alter::OnDetect_During(_uint iLayer, void* pOther, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(pOther);
		CTransform* pTransform = static_cast<CTransform*>(pDesc->pTransform);
		XMStoreFloat3(&m_vTargetPosition, pTransform->Get_State(STATE::POSITION));
		//if (false == m_isAggro)
		//	m_isAggro = true;
	}
}

void CLevi_Alter::UnActive_Resources()
{

	for (auto& Pair : m_PartObjects)
	{
		Pair.second->SetActivate(false);
		Pair.second->Reset(XMMatrixIdentity(), nullptr);
	}
}

CLevi_Alter* CLevi_Alter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevi_Alter* pInstance = new CLevi_Alter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CLevi_Alter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevi_Alter::Clone(void* pArg)
{
	CLevi_Alter* pClone = new CLevi_Alter(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CLevi_Alter (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CLevi_Alter::Free()
{
	__super::Free();
}
