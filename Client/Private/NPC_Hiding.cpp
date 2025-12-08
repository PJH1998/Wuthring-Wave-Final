#include "ClientPch.h"
#include "NPC_Hiding.h"

CNPC_Hiding::CNPC_Hiding(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CActor { pDevice, pContext }
{
}

CNPC_Hiding::CNPC_Hiding(const CNPC_Hiding& Prototype)
	: CActor { Prototype }
{
}

HRESULT CNPC_Hiding::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CNPC_Hiding::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	HIDINGDESC* pDesc = static_cast<HIDINGDESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPos), 1.f));
	m_pTransformCom->Rotation_Quaternion(pDesc->vInitRot);

	Ready_Component(pDesc);

	m_vBaseColor = _float4(1.f, 1.f, 1.f, 1.f);
	m_iFaceIndex = 5;
	m_isFind = false;
	//m_isActivate = false;
	m_isRender = true;
    return S_OK;
}

void CNPC_Hiding::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();

	if (m_isScaned)
	{
		if (m_fScanAcc < 10.f)
			m_fScanAcc += fTimeDelta;
		else
			m_isScaned = false;
		
	}
}

void CNPC_Hiding::Update(_float fTimeDelta)
{
	_bool isAnimFinished{ false };
	if (m_pAnimMachineCom)
		m_pAnimMachineCom->Update(m_pModelCom, m_pComputeShaderCom, m_pTransformCom, &m_iState, isAnimFinished, fTimeDelta);
	
	if (m_iState & ENUM_CLASS(TEST_STATE::MOVE_FORWARD))
	{
		if (isAnimFinished)
		{
			if (m_pAnimMachineCom->Get_CurrentAnimationTag() == "Run_F")
				m_isReturn = true;
		}
	}

	_vector vVelocity = m_pTransformCom->Get_Velocity();
	m_pColliderCom->Update(vVelocity);
	m_pRigidBodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CNPC_Hiding::Late_Update(_float fTimeDelta)
{
	if (m_isFind)
	{
		m_isFind = false;
		m_pRigidBodyCom->IsActivate(false);
	}

	m_pColliderCom->Sync_Position(m_pTransformCom);

	if (m_isRender)
	{
		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
			return;

		if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this)))
			return;
	}
}

void CNPC_Hiding::Render()
{
	if (FAILED(Bind_Resources()))
		return;

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	ID3D11ShaderResourceView* pNullSRV[16] = { nullptr };
	m_pContext->VSSetShaderResources(0, 16, pNullSRV);
	m_pContext->PSSetShaderResources(0, 16, pNullSRV);
	m_pContext->CSSetShaderResources(0, 16, pNullSRV);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		SHADER_ANIMMESH ePath = SHADER_ANIMMESH::DEFAULT_NORMAL;
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		_bool HasNormal{ false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;
		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		if (i == MESH_TYPE::FACE)
		{
			_uint iPadding = 3;
			_float fFaceSize = 1.f / 3;
			m_pShaderCom->Bind_Value("g_iTexPaddingCount", &iPadding, sizeof(_uint));
			m_pShaderCom->Bind_Value("g_fFaceSize", &fFaceSize, sizeof(_float));
			m_pShaderCom->Bind_Value("g_iFaceIndex", &m_iFaceIndex, sizeof(_uint));
			ePath = SHADER_ANIMMESH::NPC_FACE;
		}
		else
			ePath = SHADER_ANIMMESH::NORMAL_N_COLOR;

		m_pShaderCom->Begin(ENUM_CLASS(ePath));
		m_pModelCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
	}

#ifdef _DEBUG
	m_pColliderCom->Render();
#endif // _DEBUG

}

void CNPC_Hiding::Render_Shadow()
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

void CNPC_Hiding::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	m_pTransformCom->Save_PreviousPosition();
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));
	m_iFaceIndex = 5;
	m_pColliderCom->IsActivate(true);
	m_pColliderCom->Set_Gravity(true);
	m_pRigidBodyCom->IsActivate(true);
	m_isFind = false;
	m_isActivate = true;
}

void CNPC_Hiding::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
}

void CNPC_Hiding::Effect_Active(const _wstring& wStrEffectTag)
{
}

void CNPC_Hiding::Object_Func(const _wstring& wStrObjectTag)
{
}

HRESULT CNPC_Hiding::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	m_pShaderCom->Bind_Value("g_vBaseColor", &m_vBaseColor, sizeof(_float4));
    return S_OK;
}

void CNPC_Hiding::Ready_Component(HIDINGDESC* pDesc)
{
	// Com_Rigidbody
	CRigidbody::SPHEREBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::SPHERE;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.fRadius = 1.5f;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	//UI 상호작용
	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnDetect_During(iLayer, pDesc, Manifold);
		});

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnDetect_Enter(iLayer, pDesc, Manifold);
		});
	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::REMOVE, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnDetect_Remove(iLayer, pDesc, Manifold);
		});
	//m_pRigidBodyCom->IsActivate(false);

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 0.55f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NPC);
	ColliderDesc.fHeight = 0.5f;
	ColliderDesc.fRadius = 0.3f;
	ColliderDesc.fRayOffset = -0.15f;
	Add_Component(ENUM_CLASS(pDesc->colliderData.first), pDesc->colliderData.second,
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);

	//스캔 상호작용
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});
	//m_pColliderCom->IsActivate(false);

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

	CAnimMachine::ANIMMACNINE_DESC AnimMachineDesc = {};
	AnimMachineDesc.pAnimationTag.assign(pDesc->pAnimationTag);
	//Com_AnimMachine
	if (FAILED(Add_Component(m_pGameInstance->Get_CurrentLevel(), pDesc->pAnimMachineTag,
		TEXT("Com_AnimMachine"), reinterpret_cast<CComponent**>(&m_pAnimMachineCom), &AnimMachineDesc)))
		CRASH("NPC_Hiding/Com_AnimMachine");
}

void CNPC_Hiding::OnDetect_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
#ifdef _DEBUG
	cout << "붙었어! (NPC_Hiding)" << endl;
#endif // _DEBUG

}

void CNPC_Hiding::OnDetect_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::PLAYER))
	{
		// 근처에 다가갔을 경우 상호작용 연동
		if (false)
		{
			m_isFind = true;
		}
#ifdef _DEBUG

#endif // _DEBUG
	}

}

void CNPC_Hiding::OnDetect_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
#ifdef _DEBUG
	cout << "떨어졌어! (NPC_Hiding)" << endl;
#endif // _DEBUG
}

void CNPC_Hiding::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	if (iLayer == ENUM_CLASS(COLLISIONLAYER::INTERACTION))
	{
		SCAN_INFO* pScan = static_cast<SCAN_INFO*>(pDesc);
		_float fDistance = XMVectorGetX(XMVector3Length(m_pTransformCom->Get_State(STATE::POSITION) - pScan->vCenterPos));
		if (fDistance <= pScan->fRadius)
		{
			//스캔 성공
			if(false == m_isScaned)
			{
				m_isScaned = true;
				m_fScanAcc = 0.f;
			}
		}
	}
}

CNPC_Hiding* CNPC_Hiding::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNPC_Hiding* pInstance = new CNPC_Hiding(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CNPC_Hiding");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNPC_Hiding::Clone(void* pArg)
{
	CNPC_Hiding* pClone = new CNPC_Hiding(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CNPC_Hiding (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CNPC_Hiding::Free()
{
	__super::Free();

	Safe_Release(m_pAnimMachineCom);
}
