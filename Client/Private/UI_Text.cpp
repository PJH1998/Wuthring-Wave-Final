#include "ClientPch.h"
#include "UI_Text.h"

CUI_Text::CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCustom_UI(pDevice, pContext)
{
}

CUI_Text::CUI_Text(const CUI_Text& Prototype)
    :CCustom_UI(Prototype)
{
}

HRESULT CUI_Text::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Text::Initialize_Clone(void* pArg)
{
    __super::Initialize_Clone(pArg);

    Ready_Components(pArg);

    return S_OK;
}

void CUI_Text::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Text::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Text::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Text::Render()
{
    __super::Render();                      // Binding Shader Variables Continuously.
}

HRESULT CUI_Text::Ready_Components(void* pArg)
{
    return S_OK;
}

CUI_Text* CUI_Text::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Text* pInstance = new CUI_Text(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Text");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Text::Clone(void* pArg)
{
    CUI_Text* pInstance = new CUI_Text(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_Text");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Text::Free()
{

    __super::Free();
}
