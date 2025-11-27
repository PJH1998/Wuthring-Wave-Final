#include "ClientPch.h"
#include "RopeAnchor.h"

CRopeAnchor::CRopeAnchor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CRopeAnchor::CRopeAnchor(const CRopeAnchor& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CRopeAnchor::Initialize_Prototype()
{
	if (FAILED(CGameObject::Initialize_Prototype()))
		return E_FAIL;

    return S_OK;
}

HRESULT CRopeAnchor::Initialize_Clone(void* pArg)
{
	ROPEOBJECT_DESC* pDesc = static_cast<ROPEOBJECT_DESC*>(pArg);

	if (FAILED(CGameObject::Initialize_Clone(pDesc)))
		return E_FAIL;


	_fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Scale(pDesc->vScale);

	m_fEventDistance = pDesc->fEventDistance;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;
	

	

    return S_OK;
}

void CRopeAnchor::Priority_Update(_float fTimeDelta)
{
	CGameObject::Priority_Update(fTimeDelta);

	m_pTransformCom->Save_PreviousPosition();
}

void CRopeAnchor::Update(_float fTimeDelta)
{
	CGameObject::Update(fTimeDelta);

	// 1. 플레이어 카메라 범위 안에 들어가 있으면서 거리도 적절하다면?
	if (nullptr != m_pTargetTransform)
	{
		_vector vMyPos = m_pTransformCom->Get_State(STATE::POSITION);
		_vector vTargetPos = m_pTargetTransform->Get_State(STATE::POSITION);
		m_fTargetDistance = XMVectorGetX(XMVector3Length(vMyPos - vTargetPos));
	}
	


	// 2. RigidBodyCom 업데이트
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);


	// Last => TargetTransform 비우기?
	m_pTargetTransform = nullptr;
}

void CRopeAnchor::Late_Update(_float fTimeDelta)
{
	CGameObject::Late_Update(fTimeDelta);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CRopeAnchor::Render()
{
	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
			continue;

		_bool HasNormal = { false };
		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			HasNormal = true;

		if (FAILED(m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool))))
			CRASH("Ready g_HasNormal Failed");

		if (SUCCEEDED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK, 0)))
			CRASH("Ready g_MaskTexture");

		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(ENUM_CLASS(SHADER_MESH::DEFAULTPASS));

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Ready Render Failed");
	}

#ifdef _DEBUG
	m_pRigidbodyCom->Render();
#endif // _DEBUG

	
}

void CRopeAnchor::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	// 1. Detect 감지되면?
	if (ENUM_CLASS(COLLISIONLAYER::PLAYER) != iLayer)
		return;

	// 2. CallBack 정보 가져오기
	CALLBACK_CLIENT* pcallDesc = static_cast<CALLBACK_CLIENT*>(pDesc);

	CTransform* pTargetTransform = static_cast<CTransform*>(pcallDesc->pTransform);
	if (nullptr == pTargetTransform)
		return;

	{
		lock_guard<mutex> lock(m_Mutex);
		m_pTargetTransform = pTargetTransform;
	}

}

HRESULT CRopeAnchor::Ready_Components(ROPEOBJECT_DESC* pDesc)
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
		, pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
		, pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");

	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::GRAPPLE);
	RigidbodyDesc.vExtent = _float3(20.f, 20.f, 20.f); // 탐지 범위 안에 들어가있다면?
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollider_During(iLayer, pDesc, Manifold);
	});

	// Transform과 Rope_Anchor 타입임을 알립니다.
	m_CallBack.pTransform = m_pTransformCom; 
	m_CallBack.eObjectType = OBJECTTYPE::ROPE_ANCHOR;
	m_pRigidbodyCom->Set_Desc(&m_CallBack);
	return S_OK;
}

CRopeAnchor* CRopeAnchor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRopeAnchor* pInstance = new CRopeAnchor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CRopeAnchor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRopeAnchor::Clone(void* pArg)
{
	CRopeAnchor* pInstance = new CRopeAnchor(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CRopeAnchor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRopeAnchor::Free()
{
	CGameObject::Free();
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
