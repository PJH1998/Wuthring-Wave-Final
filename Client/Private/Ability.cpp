#include "ClientPch.h"
#include "Ability.h"

CAbility::CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{

}

CAbility::CAbility(const CAbility& Prototype)
	: CComponent ( Prototype)
{

}

HRESULT CAbility::Initialize_Prototype()
{
	if (FAILED(CComponent::Initialize_Prototype()))
		return E_FAIL;
	return S_OK;
}

HRESULT CAbility::Initialize_Clone(void* pArg)
{
	ABILLITY_DESC* pDesc = static_cast<ABILLITY_DESC*>(pArg);
	if (FAILED(CComponent::Initialize_Clone(pDesc)))
		return E_FAIL;

	// 1. File 읽기?
	return S_OK;
}

HRESULT CAbility::Render()
{
	return S_OK;
}

void CAbility::Read_File(const _char* pFilePath)
{
}

CAbility* CAbility::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CComponent* CAbility::Clone(void* pArg)
{
	return nullptr;
}

void CAbility::Free()
{
	CComponent::Free();
}
