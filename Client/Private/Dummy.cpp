#include "ClientPch.h"
#include "Dummy.h"


CDummy::CDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CDummy::CDummy(const CDummy& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CDummy::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDummy::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component();
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_pGameInstance->Rand(-20.f, 20.f), 0.f, 0.f, 1.f));

	return S_OK;
}

void CDummy::Priority_Update(_float fTimeDelta)
{
}

void CDummy::Update(_float fTimeDelta)
{
	_vector vVelocity = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	_float fMoveSpeed = 30.f;
	if (m_pGameInstance->Get_DIKeyState(DIK_W) == KEYSTATE::PRESS)
		vVelocity += XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)) * fMoveSpeed;
	if (m_pGameInstance->Get_DIKeyState(DIK_S) == KEYSTATE::PRESS)
		vVelocity -= XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK)) * fMoveSpeed;
	if (m_pGameInstance->Get_DIKeyState(DIK_A) == KEYSTATE::PRESS)
		vVelocity -= XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT)) * fMoveSpeed;
	if (m_pGameInstance->Get_DIKeyState(DIK_D) == KEYSTATE::PRESS)
		vVelocity += XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT)) * fMoveSpeed;

	//m_pModelCom->Play_Animation("Stand1", fTimeDelta, nullptr);
	m_pColliderCom->Update(vVelocity);
}

void CDummy::Late_Update(_float fTimeDelta)
{
	//m_pColliderCom->Sync_Position(m_pTransformCom);

	// Guizmo Test
	m_pGameInstance->Use_Gizmo(m_pTransformCom);

	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);
}

void CDummy::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		//m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(0);
		
		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	//m_pRigidbodyCom->Render();
#endif
}

void CDummy::Render_Shadow()
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

void CDummy::OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	int a = 0;
}

void CDummy::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	int a = 0;
}

void CDummy::OnCollide_Remove(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	int a = 0;
}

void CDummy::Ready_Component()
{
	// Com_Shader
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"), 
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr);

	// Com_Model
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Model_Wolf"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr);

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
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//ColliderDesc.vPos = _float3(0.f, 0.f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	ColliderDesc.fHeight = 10.f;
	ColliderDesc.fRadius = 20.f; //m_pGameInstance->Rand(5.f, 20.f);
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);

	// Collide Callback Func Setting
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& inManifold) {
			OnCollide_Enter(iLayer, pDesc, inManifold);
		});
	// Collide Callback Func Setting
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& inManifold) {
		OnCollide_During(iLayer, pDesc, inManifold);
		});
	// Collide Callback Func Setting
	m_pColliderCom->SetUp_CallBack(COLLIDE_STATE::REMOVE, [this](_uint iLayer, void* pDesc, const ContactManifold& inManifold) {
		OnCollide_Remove(iLayer, pDesc, inManifold);
		});
}

CDummy* CDummy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDummy* pInstance = new CDummy(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Dummy");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDummy::Clone(void* pArg)
{
	CDummy* pClone = new CDummy(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Dummy (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CDummy::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pColliderCom);
}
