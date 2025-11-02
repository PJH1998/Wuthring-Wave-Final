#include"EditorPch.h"
#include "Edit_TriggerBox.h"
#include"Event_Level.h"
CEdit_TriggerBox::CEdit_TriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CEdit_TriggerBox::CEdit_TriggerBox(const CEdit_TriggerBox& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CEdit_TriggerBox::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_TriggerBox::Initialize_Clone(void* pArg)
{
	TRIGGER* pDesc = static_cast<TRIGGER*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	Ready_Components(pArg);
	m_pRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [&](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		iLayer = ENUM_CLASS(LEVEL::MAP);
		int a = 0;
		});

	//m_pGameInstance->Publish()
	//똥 생김, 트리거박스 저장 방법 및 트리거 번호 설정 및 저장.
	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Trigger"), [this](const MAP_SAVE& event) {
		if (!m_isActivate)
			return;

		event.File.write(reinterpret_cast<const char*>(&m_vExtends), sizeof(_float3));
		event.File.write(reinterpret_cast<const char*>(&m_iTriggerIndex), sizeof(_uint));
		_float4x4 WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
		event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));
		});
    return S_OK;
}

void CEdit_TriggerBox::Priority_Update(_float fTimeDelta)
{
}

void CEdit_TriggerBox::Update(_float fTimeDelta)
{
	const ContactManifold Manifold{};
	//m_pRigidbodyCom->OnCollide_Enter(ENUM_CLASS(LEVEL::MAP), nullptr, Manifold);
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CEdit_TriggerBox::Late_Update(_float fTimeDelta)
{

}

void CEdit_TriggerBox::Ready_Components(void* pArg)
{
	TRIGGER* pDesc = static_cast<TRIGGER*>(pArg);

	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	m_vExtends = RigidbodyDesc.vExtent = pDesc->vExtends;
	
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

}

void CEdit_TriggerBox::Collision()
{
	for (const auto& Func:m_Functions)
	{
		if (Func)
			Func(nullptr);
	}
}

CEdit_TriggerBox* CEdit_TriggerBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_TriggerBox* pInstance = new CEdit_TriggerBox(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_TriggerBox::Clone(void* pArg)
{
	CEdit_TriggerBox* pInstance = new CEdit_TriggerBox(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_TriggerBox::Free()
{
	__super::Free();

	Safe_Release(m_pRigidbodyCom);

}