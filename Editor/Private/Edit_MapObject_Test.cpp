#include"EditorPch.h"
#include "Edit_MapObject_Test.h"
CEdit_MapObject_Test::CEdit_MapObject_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_MapObject_Test::CEdit_MapObject_Test(const CEdit_MapObject_Test& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Test::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_MapObject_Test::Initialize_Clone(void* pArg)
{
	Ready_Component(pArg);
    return S_OK;
}

void CEdit_MapObject_Test::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Test::Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Test::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::BLEND, this);
	m_pModelCom->Render(0, nullptr);
}

void CEdit_MapObject_Test::Render()
{
}

void CEdit_MapObject_Test::Render_Shadow()
{
}

void CEdit_MapObject_Test::Set_ImGuiOption()
{
}

HRESULT CEdit_MapObject_Test::Ready_Component(void* pArg)
{
	BUFFER_TEST* pDesc = static_cast<BUFFER_TEST*>(pArg);
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::MAP), StringToWString(pDesc->ModelName),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	return S_OK;
}

CEdit_MapObject_Test* CEdit_MapObject_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Test* pInstance = new CEdit_MapObject_Test(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Test::Clone(void* pArg)
{
	CEdit_MapObject_Test* pInstance = new CEdit_MapObject_Test(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapObject_Test::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}