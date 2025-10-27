#include "ClientPch.h"
#include "UI_Button.h"

CUI_Button::CUI_Button(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Button::CUI_Button(const CUI_Button& Prototype)
	:CCustom_UI(Prototype)
{
}

HRESULT CUI_Button::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Button::Initialize_Clone(void* pArg)
{
    __super::Initialize_Clone(pArg);

    Ready_Components(pArg);

	return S_OK;
}

void CUI_Button::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Button::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Button::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Button::Render()
{
    __super::Render();                      // Nothing.
}

HRESULT CUI_Button::Ready_Components(void* pArg)
{
    return S_OK;
}

CUI_Button* CUI_Button::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Button* pInstance = new CUI_Button(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Button");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Button::Clone(void* pArg)
{
    CUI_Button* pInstance = new CUI_Button(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_Button");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Button::Free()
{

	__super::Free();
}
