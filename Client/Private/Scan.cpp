#include "ClientPch.h"
#include "Scan.h"

CScan::CScan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CScan::CScan(const CScan& Prototype)
	: CGameObject { Prototype }
	, m_vRadius { Prototype.m_vRadius }
	, m_fSpeed { Prototype.m_fSpeed }
	, m_vColor { Prototype.m_vColor }
	, m_fBlendRadius { Prototype.m_fBlendRadius }
	, m_fWidth { Prototype.m_fWidth }
{
}

HRESULT CScan::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vRadius = _float2(0.f, 150.f);

	m_fBlendRadius = 70.f;
	
	m_fWidth = 0.5f;

	m_fSpeed = 35.f;

	m_vColor = _float4(0.87f, 0.95f, 0.02f, 1.f);

	return S_OK;
}

HRESULT CScan::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CScan::Priority_Update(_float fTimeDelta)
{
}

void CScan::Update(_float fTimeDelta)
{
	m_fCurRadius += m_fSpeed * fTimeDelta;

	m_tInfo.fRadius = m_fCurRadius;
	_float fScale = (m_fCurRadius + m_fWidth) * 2.f;
	_float fScaleY = (m_vRadius.y + m_fWidth) * 2.f;

	m_pTransformCom->Scale(_float3(fScale, fScaleY, fScale));
} 

void CScan::Late_Update(_float fTimeDelta)
{
	if (m_fCurRadius >= m_vRadius.y)
	{
		m_pRigidBody->IsActivate(false);
		m_pRigidBody->Set_Desc(nullptr);
		m_isActivate = false;
		return;
	}

	if (FAILED(m_pGameInstance->Add_CustomDecal(this)))
		return;
}

void CScan::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return;

	m_pShader->Begin(0);

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CScan::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;

	m_pRigidBody->IsActivate(true);

	_vector vPosition = WorldMatrix.r[3];

	_matrix ScanWorldMatrix = XMMatrixTranslationFromVector(vPosition);

	m_pTransformCom->Set_WorldMatrix(ScanWorldMatrix);
	m_fCurRadius = m_vRadius.x;

	_float fScale = (m_fCurRadius + m_fWidth) * 2.f;
	_float fScaleY = (m_vRadius.y + m_fWidth) * 2.f;

	m_pTransformCom->Scale(_float3(fScale, fScaleY, fScale));

	m_pRigidBody->Set_Position(vPosition);
	m_tInfo.vCenterPos = vPosition;
	m_tInfo.fRadius = m_fCurRadius;
}

HRESULT CScan::Ready_Components()
{
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Cube"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Scan"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTexture), nullptr)))
		ASSERT_CRASH(m_pTexture);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Scan"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);


	CRigidbody::SPHEREBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::SPHERE;
	RigidbodyDesc.eType = EMotionType::Dynamic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::INTERACTION);
	RigidbodyDesc.fRadius = m_vRadius.y;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBody), &RigidbodyDesc)))
		ASSERT_CRASH(m_pRigidBody);

	m_pRigidBody->IsActivate(false);

	m_pRigidBody->Set_Desc(&m_tInfo);

	return S_OK;
}

HRESULT CScan::Bind_ShaderResources()
{
	if (FAILED(m_pTexture->Bind_Shader_Resource(m_pShader, "g_MaskTexture")))
		CRASH("Failed to Bind Mask Texture");

	if (FAILED(m_pGameInstance->Bind_OpenRT(OPEN_RT::DEPTH, m_pShader, "g_DepthTexture")))
		CRASH("Failed to Bind Depth Texture");

	if(FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if(FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed to Bind ProjMatrix");

	_float4x4 WorldMatirxInv = {};
	XMStoreFloat4x4(&WorldMatirxInv, m_pTransformCom->Get_WorldMatrix_Inv());
	
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrixInv", &WorldMatirxInv)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		CRASH("Failed to Bind ProjMatrix");

	if(FAILED(m_pShader->Bind_Value("g_fCircleRadius", &m_fCurRadius, sizeof(_float))))
		CRASH("Failed to Bind CircleRadius");

	if (FAILED(m_pShader->Bind_Value("g_fCircleWidth", &m_fWidth, sizeof(_float))))
		CRASH("Failed to Bind CircleWidth");

	if (FAILED(m_pShader->Bind_Value("g_fBlendRadius", &m_fBlendRadius, sizeof(_float))))
		CRASH("Failed to Bind BlendRadius");

	if (FAILED(m_pShader->Bind_Value("g_fMaxRadius", &m_vRadius.y, sizeof(_float))))
		CRASH("Failed to Bind BlendRadius");

	if (FAILED(m_pShader->Bind_Value("g_vScanColor", &m_vColor, sizeof(_float4))))
		CRASH("Failed to Bind BlendRadius");

	return S_OK;
}

CScan* CScan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CScan* pInstance = new CScan(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CScan");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CScan::Clone(void* pArg)
{
	CScan* pInstance = new CScan(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CScan");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CScan::Free()
{
	__super::Free();
	
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
	Safe_Release(m_pTexture);
	Safe_Release(m_pRigidBody);
}
