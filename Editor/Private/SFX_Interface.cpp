#include "EditorPch.h"
#include "SFX_Interface.h"

CSFX_Interface::CSFX_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext}
{
}

HRESULT CSFX_Interface::Initialize()
{
    return E_NOTIMPL;
}

CSFX_Interface* CSFX_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return nullptr;
}

void CSFX_Interface::Free()
{
	__super::Free();

}
