#include "EditorPch.h"
#include "SFX_Interface.h"
#include "Augusta_SFX.h"

CSFX_Interface::CSFX_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext}
{
}

HRESULT CSFX_Interface::Initialize()
{
	m_pCurrentSFX = CAugusta_SFX::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pCurrentSFX);

    return S_OK;
}

void CSFX_Interface::Priority_Update(_float fTimeDelta)
{


}

void CSFX_Interface::Update(_float fTimeDelta)
{

}

void CSFX_Interface::Late_Update(_float fTimeDelta)
{
	m_pCurrentSFX->Late_Update(fTimeDelta);
}

void CSFX_Interface::Render()
{
}

CSFX_Interface* CSFX_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSFX_Interface* pInstance = new CSFX_Interface(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CSFX_Interface");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CSFX_Interface::Free()
{
	__super::Free();

	Safe_Release(m_pCurrentSFX);
}
