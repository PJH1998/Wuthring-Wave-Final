#include "ClientPch.h"
#include "PatternDummy.h"
#include "WeaponDummy.h"
#include "Projectile.h"
#include "Levi_Anchor.h"

CPatternDummy::CPatternDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CContainerObject { pDevice, pContext }
{
}

CPatternDummy::CPatternDummy(const CPatternDummy& Prototype)
	: CContainerObject{ Prototype }
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
	m_pTransformCom->Rotation_Quaternion(pDesc->vInitRotation);
	Ready_Component(pDesc);

	m_strInitAnimTag = pDesc->strInitAnimTag;
	m_strAnimTag = m_strInitAnimTag;

	Register_AllNotifies(pDesc->strFolderPath);
	m_eType = pDesc->eType;
	if (m_eType == MODEL_TYPES::WEAPON)
		Ready_PartObjects(pDesc);
	return S_OK;
}

void CPatternDummy::Priority_Update(_float fTimeDelta)
{
	m_pTransformCom->Save_PreviousPosition();

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Priority_Update(fTimeDelta);
	}
}

void CPatternDummy::Update(_float fTimeDelta)
{
	_bool isFinished{};
	isFinished = m_pModelCom->Play_Animation_GPU(m_pComputeShaderCom, m_strAnimTag, fTimeDelta * 1.f, &m_fTrackPosition, m_isRootMotion, m_isRootRotate, m_isRootTranslate, 1.f);
	
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	XMStoreFloat3(&m_vPosition, m_pTransformCom->Get_State(STATE::POSITION));
	if (isFinished)
	{
		m_strAnimTag = m_strInitAnimTag;
	}
	//m_pModelCom->Play_Animation("Stand1", fTimeDelta, nullptr);
	if(m_pColliderCom)
	{
		_vector vVelocity = m_pTransformCom->Get_Velocity();
		m_pColliderCom->Update(vVelocity / fTimeDelta);
	}

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Update(fTimeDelta);
	}
}

void CPatternDummy::Late_Update(_float fTimeDelta)
{
	if (m_pColliderCom)
		m_pColliderCom->Sync_Position(m_pTransformCom);
	// Guizmo Test

#ifdef _DEBUG
	m_pGameInstance->Use_Gizmo(m_pTransformCom);

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


	//m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(2480.6f, 316.901f, 1826.5f, 1.f));
	m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this);
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);

	for (auto& Pair : m_PartObjects)
	{
		if (Pair.second->IsActivate())
			Pair.second->Late_Update(fTimeDelta);
	}
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

		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_ANIMMESH::NORMAL_TEX));
		

		m_pModelCom->Render(i);

		m_pShaderCom->UndBind_All_VS_SRV();
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

	// Com_ComputeShader
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC)
		, TEXT("Prototype_Component_Shader_ComputeVtxAnimMeshNonRib"), TEXT("Com_ComputeShader"), reinterpret_cast<CComponent**>(&m_pComputeShaderCom), nullptr)))
		CRASH("Leviatan/Com_ComputeShader");

	// Com_Model
	Add_Component(ENUM_CLASS(pDesc->eLevel), pDesc->strModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr);

	if(pDesc->isCollide)
	{
		// Com_Collider
		CCollider::COLLIDER_DESC ColliderDesc = {};
		XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
		ColliderDesc.vOffset = _float3(0.f, 1.1f, 0.f);
		ColliderDesc.eType = EMotionType::Kinematic;
		ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
		ColliderDesc.fHeight = 1.45f;
		ColliderDesc.fRadius = 0.4f;
		ColliderDesc.fRayOffset = -0.15f;
		Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
			TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
		ASSERT_CRASH(m_pColliderCom);
		m_tCallBack.pTransform = m_pTransformCom;
		m_tCallBack.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(2);
		m_pColliderCom->Set_Desc(&m_tCallBack);
	}

#ifdef _DEBUG
	m_strAnimationTags = m_pModelCom->Get_AnimationNames();
#endif // _DEBUG

}

void CPatternDummy::Ready_PartObjects(PAT_DUMMYDESC* pDesc)
{
	CWeaponDummy::WD_DESC WD{};
	WD.vOffsetPos = pDesc->vOffsetPos;
	WD.vOffsetRadian = pDesc->vOffsetRot;
	WD.wstrModelTag = pDesc->strPartTag;
	WD.pParentTransform = m_pTransformCom;
	WD.pSocketMatrix = m_pModelCom->Get_BoneMatrixPtr(pDesc->strBoneName.c_str());
	CContainerObject::Add_PartObject(TEXT("Part_Weapon"), ENUM_CLASS(pDesc->eLevel),TEXT("Prototype_GameObject_WeaponDummy"), &WD);
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

	PREFAB_INFO Info = {};
	Info.pMatrixPtr = m_pTransformCom->Get_WorldMatrixPtr();
	Info.pModelPtr = m_pModelCom;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, &Info);
}

void CPatternDummy::Object_Func(const _wstring& wStrObjectTag)
{
	size_t Index = wStrObjectTag.find(TEXT("|"));
	_wstring wstrTypeTag = wStrObjectTag.substr(0, Index);
	_wstring wstrAnimTag = wStrObjectTag.substr(Index + 1);

	// 레비아탄 투사체
	if(wstrTypeTag == TEXT("Sword"))
	{
		CProjectile::PROJECTILERESET Desc{};
		XMStoreFloat3(&Desc.vTargetPos, XMVectorSetW(m_pTransformCom->Get_State(STATE::POSITION) + m_pTransformCom->Get_State(STATE::LOOK), 1.f));
		Desc.vTargetPos.y += 0.5f; //offset
		_matrix WorldMatrix = XMMatrixIdentity();
		_vector vScale{}, vQuat{}, vTrans{};
		if(wstrAnimTag == TEXT("Aura"))
		{
			XMMatrixDecompose(&vScale, &vQuat, &vTrans, WorldMatrix);
			m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Projectile_LeviAura"), m_pTransformCom->Get_WorldMatrix(), &Desc);
		}
		else if(wstrAnimTag == TEXT("Proj"))
		{
			XMMatrixDecompose(&vScale, &vQuat, &vTrans, WorldMatrix);
			m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_Projectile_LeviSword"), m_pTransformCom->Get_WorldMatrix(), &Desc);
		}
		else if(wstrAnimTag == TEXT("Wave"))
		{
			m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_LeviWave"), m_pTransformCom->Get_WorldMatrix(), nullptr);
		}
	}
	else if(wstrTypeTag == TEXT("Anchor"))
	{
		_float3 vInitPosition{};
		CLevi_Anchor::ANCHORRESET Anchor{};
		Anchor.vTargetPos = _float3(0.f, 0.f, 0.f);
		_vector vRight = m_pTransformCom->Get_State(STATE::RIGHT);
		_vector vInitPos = XMLoadFloat3(&Anchor.vTargetPos) - vRight + XMVectorSet(0.f, 1.f, 0.f, 0.f) * 5.f;
		XMStoreFloat3(&vInitPosition, vInitPos);
		m_pGameInstance->Spawn_PoolingObject(TEXT("Pool_LeviAnchor"), XMMatrixTranslation(vInitPosition.x, vInitPosition.y, vInitPosition.z), &Anchor);
	}
	// 레비아탄 투사체 end
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

	Safe_Release(m_pComputeShaderCom);
	//Safe_Release(m_pFacialShaderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	//Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pColliderCom);
}
