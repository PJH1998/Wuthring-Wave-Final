#include "ClientPch.h"
#include "UI_Image.h"

CUI_Image::CUI_Image(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCustom_UI(pDevice, pContext)
{
}

CUI_Image::CUI_Image(const CUI_Image& Prototype)
    :CCustom_UI(Prototype)
{
}

HRESULT CUI_Image::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Image::Initialize_Clone(void* pArg)
{
    __super::Initialize_Clone(pArg);

    Ready_Components(pArg);

    return S_OK;
}

void CUI_Image::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Image::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Image::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Image::Render()
{
    __super::Render();                      // Nothing.
}

HRESULT CUI_Image::Ready_Components(void* pArg)
{
    return S_OK;
}

CUI_Image* CUI_Image::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Image* pInstance = new CUI_Image(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Image");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Image::Clone(void* pArg)
{
    CUI_Image* pInstance = new CUI_Image(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_Image");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Image::Free()
{

    __super::Free();
}
