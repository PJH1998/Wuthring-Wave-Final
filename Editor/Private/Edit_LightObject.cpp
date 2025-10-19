#include"EditorPch.h"
#include "Edit_LightObject.h"

CEdit_LightObject::CEdit_LightObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

CEdit_LightObject::CEdit_LightObject(const CEdit_LightObject& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CEdit_LightObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEdit_LightObject::Initialize_Clone(void* pArg)
{
	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
#ifdef _DEBUG
	m_LightDesc = m_pGameInstance->Get_LightDesc_For_Map(TEXT("Test"));
#endif
	return S_OK;
}

void CEdit_LightObject::Priority_Update(_float fTimeDelta)
{
}

void CEdit_LightObject::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_U) == KEYSTATE::DOWN)
		m_LightDesc->vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	else if(m_pGameInstance->Get_DIKeyState(DIK_I) == KEYSTATE::DOWN)
		m_LightDesc->vDiffuse = _float4(0.f, 1.f, 0.f, 1.f);
	else if (m_pGameInstance->Get_DIKeyState(DIK_O) == KEYSTATE::DOWN)
		m_LightDesc->vDiffuse = _float4(0.f, 0.f, 1.f, 1.f);
	else if (m_pGameInstance->Get_DIKeyState(DIK_P) == KEYSTATE::DOWN)
		m_LightDesc->vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
}

void CEdit_LightObject::Late_Update(_float fTimeDelta)
{
}

void CEdit_LightObject::Render()
{
}

void CEdit_LightObject::Render_Shadow()
{
}

CEdit_LightObject* CEdit_LightObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_LightObject* pInstance = new CEdit_LightObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : LightObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_LightObject::Clone(void* pArg)
{
	CEdit_LightObject* pInstance = new CEdit_LightObject(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : LightObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}
void CEdit_LightObject::Free()
{
	__super::Free();
	m_LightDesc = nullptr;
}
