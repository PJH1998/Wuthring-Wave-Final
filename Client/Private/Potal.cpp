#include "ClientPch.h"
#include"Potal.h"
#include"GameSystem.h"
#include"Event_Level.h"

CPotal::CPotal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
:CGameObject(pDevice,pContext)
{
}

CPotal::CPotal(const CPotal& Prototype)
	:CGameObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CPotal::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CPotal::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Components(pArg);
	m_szText = TEXT("순례의 천국 입장하기");
	PotalActive(false);
	m_pGameSystem->Potal_Register(this);

	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		if (ENUM_CLASS(COLLISIONLAYER::PLAYER) == iLayer)
			m_pGameSystem->Req_Render_InteractUI(m_szText);
			Change_Level();
		});

    return S_OK;
}

void CPotal::Priority_Update(_float fTimeDelta)
{

}

void CPotal::Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CPotal::Late_Update(_float fTimeDelta)
{
	//빌보드 직접 만드셈ㅇㅇ
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CPotal::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pDiffuseCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture");
	m_pFirstMaskCom->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture");
	//m_pSecondMaskCom->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture2");

	m_pShaderCom->Bind_Value("g_TotalTime", &m_fTotalTime, sizeof(_float));
	m_pShaderCom->Bind_Value("vColor", &m_vColor, sizeof(_float4));
	m_pVIBufferCom->Bind_Resources();
	m_pShaderCom->Begin(2);
	m_pVIBufferCom->Render();
}

void CPotal::PotalActive(_bool B)
{
	m_isActivate = B;
	m_pRigidbodyCom->IsActivate(B);
}

void CPotal::Change_Level()
{
	if (m_pGameInstance->Get_DIKeyState(DIK_F) == KEYSTATE::DOWN)
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::HEAVEN, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
}

void CPotal::Ready_Components(void* pArg)
{
	POTAL_DESC* pDesc = static_cast<POTAL_DESC*>(pArg);

	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&pDesc->vPos));
	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vExtent;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Texture_Potal_Diffuse"),
		TEXT("Com_Diffuse"), reinterpret_cast<CComponent**>(&m_pDiffuseCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Texture_Potal_Mask"),
		TEXT("Com_FirstMask"), reinterpret_cast<CComponent**>(&m_pFirstMaskCom), nullptr)))
		CRASH("FAILED");

	//if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Texture_Potal_Mask"),
	//	TEXT("Com_SecondMask"), reinterpret_cast<CComponent**>(&m_pSecondMaskCom), nullptr)))
	//	CRASH("FAILED");

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
		CRASH("FAILED");

}

CPotal* CPotal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPotal* pInstance = new CPotal(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Potal");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPotal::Clone(void* pArg)
{
	CPotal* pClone = new CPotal(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Potal (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CPotal::Free()
{
	__super::Free();
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pDiffuseCom);
	Safe_Release(m_pFirstMaskCom);
	Safe_Release(m_pSecondMaskCom);
}
