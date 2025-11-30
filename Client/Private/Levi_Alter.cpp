#include "ClientPch.h"
#include "Levi_Alter.h"
#include "Levi_Bayonet.h"
#include "Levi_Bow.h"

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

	ALTER_DESC* pDesc = static_cast<ALTER_DESC*>(pArg);
	m_fAttackDmg = pDesc->fAttackDmg;

	Ready_Component(pDesc);
	Ready_PartObject(pDesc);
	m_Tracks.emplace(make_pair("Attack18", make_pair(0.f, 195.f)));
	m_Tracks.emplace(make_pair("Attack19", make_pair(0.f, 195.f)));
	m_Tracks.emplace(make_pair("Attack_20|1", make_pair(30.f, 49.f)));
	m_Tracks.emplace(make_pair("Attack_20|2", make_pair(60.f, 72.f)));
	m_Tracks.emplace(make_pair("Attack_20|3", make_pair(120.f, 138.f)));
	m_Tracks.emplace(make_pair("Attack05_5", make_pair(12, 50)));
	m_isActivate = false;
	m_vBaseColor = _float4(0.2f, 0.2f, 0.2f, 1.f);
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
	if (fTrackPos >= m_Tracks[m_strPatternKey].second)
	{
		Reset_NotifyInteraction();
		UnActive_Resources();
		return;
	}
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
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_strPatternKey = pDesc->strPatternKey;
	size_t Index = pDesc->strPatternKey.find("|");
	_string wstrAnimTag = pDesc->strPatternKey.substr(0, Index);
	_string wstrTypeTag = pDesc->strPatternKey.substr(Index + 1);
	m_strAnimKey = wstrAnimTag;
	//m_pAnimMachineCom->Reset(m_pModelCom, m_strAnimKey);
	m_pModelCom->Clear_Animation(m_strAnimKey);
	if(Index == pDesc->strPatternKey.npos)
		m_pModelCom->Set_TrackPosition(m_strAnimKey, m_Tracks[m_strAnimKey].first);
	else
	{
		m_pModelCom->Set_TrackPosition(m_strAnimKey, m_Tracks[m_strPatternKey].first);
	}
	if (pDesc->eType == ATTACK_TYPE::SWORD)
	{
		CLevi_Bayonet* pWeapon = dynamic_cast<CLevi_Bayonet*>(m_PartObjects[TEXT("Part_Bayonet")]);
		pWeapon->SetActivate(true);
	}
	else if (pDesc->eType == ATTACK_TYPE::BOW)
	{
		m_PartObjects[TEXT("Part_Bow")]->SetActivate(true);
	}
	m_pColliderCom->Set_Gravity(false);
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));
	m_isActivate = true;
}

void CLevi_Alter::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	size_t Index = wStrColliderTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrColliderTag.substr(0, Index);
	_wstring wstrPartTag = wStrColliderTag.substr(Index + 1);
	if (wstrTypeTag == TEXT("Attack"))
	{
		if (wstrPartTag == TEXT("Sword"))
		{
			dynamic_cast<CLevi_Bayonet*>(m_PartObjects[TEXT("Part_Bayonet")])->Attack_Active(IsActive);
		}
	}
	else if (wstrTypeTag == TEXT("Gravity"))
	{
		m_pColliderCom->Set_Gravity(IsActive);
	}
	else if (wstrTypeTag == TEXT("Lerp"))
	{
		m_isTurnLerp = IsActive;
	}
	else if (wstrTypeTag == TEXT("Distance"))
	{
		m_isDist_Interp_Enable = IsActive;
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
	else if (wstrTypeTag == TEXT("Ray"))
	{
		
	}
	else if (wstrTypeTag == TEXT("Reset"))
	{
		Reset_NotifyInteraction();
	}
}

void CLevi_Alter::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_vBaseColor", &m_vBaseColor, sizeof(_float4));
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
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ALTER);
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
	m_ShaderIndices.resize(m_pModelCom->Get_NumMesh(), ENUM_CLASS(SHADER_ANIMMESH::AUGUSTA));
}

void CLevi_Alter::Ready_PartObject(ALTER_DESC* pDesc)
{
	CLevi_Bayonet::LEVIBAYONET_DESC BayonetDesc{};
	BayonetDesc.eType = TEXT_COLOR_TYPE::DARK;
	BayonetDesc.fAttackDmg = m_fAttackDmg;
	BayonetDesc.pParentTransform = m_pTransformCom;
	BayonetDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp02");
	BayonetDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	BayonetDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(0.f), XMConvertToRadians(0.f));

	if (FAILED(CContainerObject::Add_PartObject(TEXT("Part_Bayonet"), ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_GameObject_Levi_Bayonet"), &BayonetDesc)))
		CRASH("Failed to Add Part : Bayonet");
	m_PartObjects[TEXT("Part_Bayonet")]->SetActivate(false);

	CLevi_Bow::LEVIBOW_DESC BowDesc{};
	BowDesc.fAttackDmg = m_fAttackDmg;
	BowDesc.pParentTransform = m_pTransformCom;
	BowDesc.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr("WeaponProp01");
	BowDesc.vOffsetPos = _float3(0.f, 0.f, 0.f);
	BowDesc.vOffsetRadian = _float3(XMConvertToRadians(0.f), XMConvertToRadians(-90.f), XMConvertToRadians(0.f));

	if (FAILED(CContainerObject::Add_PartObject(TEXT("Part_Bow"), ENUM_CLASS(pDesc->eCurLevel), TEXT("Prototype_GameObject_Levi_Bow"), &BowDesc)))
		CRASH("Failed to Add Part : Bow");

	m_PartObjects[TEXT("Part_Bow")]->SetActivate(false);
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
	m_isActivate = false;
}

void CLevi_Alter::Reset_NotifyInteraction()
{
	m_isTurnLerp = false;
	m_isDist_Interp_Enable = false;

	m_pColliderCom->Set_Gravity(true);
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
