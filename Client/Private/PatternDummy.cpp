#include "ClientPch.h"
#include "PatternDummy.h"


CPatternDummy::CPatternDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CPatternDummy::CPatternDummy(const CPatternDummy& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CPatternDummy::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPatternDummy::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	PAT_DUMMYDESC* pDesc = static_cast<PAT_DUMMYDESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vInitPosition), 1.f));

	Ready_Component(pDesc);

	m_strInitAnimTag = pDesc->strInitAnimTag;
	m_strAnimTag = m_strInitAnimTag;

	Register_AllNotifies(pDesc->strFolderPath);
	return S_OK;
}

void CPatternDummy::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();
}

void CPatternDummy::Update(_float fTimeDelta)
{
	_bool isFinished{};
	isFinished = m_pModelCom->Play_Animation_CPU(m_strAnimTag, fTimeDelta * 1.f, &m_fTrackPosition, false, m_isRootMotion, m_isRootRotate, m_isRootTranslate, 1.f);
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	XMStoreFloat3(&m_vPosition, m_pTransformCom->Get_State(STATE::POSITION));
	if (isFinished)
	{
		m_strAnimTag = m_strInitAnimTag;
	}
	//m_pModelCom->Play_Animation("Stand1", fTimeDelta, nullptr);
	//_vector vVelocity = m_pTransformCom->Get_Velocity();
	//m_pColliderCom->Update(vVelocity / fTimeDelta);
}

void CPatternDummy::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);
	// Guizmo Test
	m_pGameInstance->Use_Gizmo(m_pTransformCom);

#ifdef _DEBUG

	if(!m_strAnimationTags.empty())
	{
		const _char* szPreview = m_strAnimTag.c_str();
		if(ImGui::BeginCombo("Select Animation", szPreview))
		{
			_int iGuiID{};
			for(auto& strAnimName : m_strAnimationTags)
			{
				ImGui::PushID(iGuiID);
				const _bool isSelected = strAnimName == m_strAnimTag;
				if(ImGui::Selectable(strAnimName.c_str(), isSelected))
				{
					m_strAnimTag = strAnimName;
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
	}
	if (ImGui::DragFloat3("Pos", reinterpret_cast<_float*>(&m_vPosition), 0.1f))
	{
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vPosition), 1.f));
	}
	ImGui::Checkbox("Root Motion Enable", &m_isRootMotion);
	ImGui::Checkbox("Root Rotate", &m_isRootRotate);
	ImGui::Checkbox("Root Translate", &m_isRootTranslate);

#endif // _DEBUG


	m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this);
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);
}

void CPatternDummy::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::DEFAULT_NORMAL));
		
		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	//m_pRigidbodyCom->Render();
#endif
}

void CPatternDummy::Render_Shadow()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");

	m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		m_pShaderCom->Begin(5);

		m_pModelCom->Render(i);
	}
}

void CPatternDummy::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	int a = 0;
}

void CPatternDummy::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	int a = 0;
}

void CPatternDummy::OnCollide_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	int a = 0;
}

void CPatternDummy::Ready_Component(PAT_DUMMYDESC* pDesc)
{
	// Com_Shader
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), 
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr);

	// Com_Model
	Add_Component(ENUM_CLASS(pDesc->eLevel), pDesc->strModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr);

#ifdef _DEBUG
	m_strAnimationTags = m_pModelCom->Get_AnimationNames();
#endif // _DEBUG


	// Com_Rigidbody
	//CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eShape = SHAPE::MESH;
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	//RigidbodyDesc.pModel = m_pModelCom;
	//CRigidbody::CAPSULEBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eShape = SHAPE::CAPSULE;
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Kinematic;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	//RigidbodyDesc.fHeight = 10.f;
	//RigidbodyDesc.fRadius = m_pGameInstance->Rand(5.f, 20.f);
	//RigidbodyDesc.eBodyType = CRigidbody::BODYTYPE::VIRTUAL;
	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	// Com_Collider
	//CCollider::COLLIDER_DESC ColliderDesc = {};
	//XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	////ColliderDesc.vPos = _float3(0.f, 0.f, 0.f);
	//ColliderDesc.eType = EMotionType::Kinematic;
	//ColliderDesc.vOffset = _float3(0.f, 1.35f, 0.f);
	//ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	//ColliderDesc.fHeight = 1.8f;
	//ColliderDesc.fRadius = 7.f; //m_pGameInstance->Rand(5.f, 20.f);
	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
	//	TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	//
	//// Collide Callback Func Setting
	//m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& inManifold) {
	//		OnCollide_Enter(iLayer, pDesc, inManifold);
	//	});
	//// Collide Callback Func Setting
	//m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& inManifold) {
	//	OnCollide_During(iLayer, pDesc, inManifold);
	//	});
	//// Collide Callback Func Setting
	//m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::REMOVE, [this](_uint iLayer, void* pDesc, const ContactManifold& inManifold) {
	//	OnCollide_Remove(iLayer, pDesc, inManifold);
	//	});
	//m_pColliderCom->Set_Gravity(false);

}

void CPatternDummy::Register_AllNotifies(const _string& strFolderPath)
{
	if (strFolderPath == "")
		return;

	ASSERT_CRASH(m_pModelCom);
	auto colliderCallback = [this](const _wstring& tag, bool active) {
		this->Collider_Active(tag, active);
		};

	auto effectCallBack = [this](const _wstring& tag) {
		this->Effect_Active(tag);
		};

	auto objectCallBack = [this](const _wstring& tag) {
		this->Object_Func(tag);
		};

	m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack, objectCallBack);
}

void CPatternDummy::Effect_Active(const _wstring& wStrEffectTag)
{
	if(nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}

CPatternDummy* CPatternDummy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPatternDummy* pInstance = new CPatternDummy(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Dummy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPatternDummy::Clone(void* pArg)
{
	CPatternDummy* pClone = new CPatternDummy(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Dummy (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CPatternDummy::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	//Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pColliderCom);
}
