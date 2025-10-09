// ==============================
// * 에디터에서만 사용할 임시 UI 오브젝트
// ==============================

#include "EditorPch.h"
#include "Custom_UI.h"

CCustom_UI::CCustom_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject(pDevice, pContext)
{
}

CCustom_UI::CCustom_UI(const CCustom_UI& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CCustom_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCustom_UI::Initialize_Clone(void* pArg)
{
    __super::Initialize_Clone(pArg);

    Ready_Prototypes(pArg);
    Ready_Components(pArg);

    __super::Begin();

	return S_OK;
}

void CCustom_UI::Priority_Update(_float fTimeDelta)
{
}

void CCustom_UI::Update(_float fTimeDelta)
{
}

void CCustom_UI::Late_Update(_float fTimeDelta)
{
    //if (!m_isActive)
    //    return;

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::UI, this)))
        return;
}

void CCustom_UI::Render()
{
    //__super::Begin();

    if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
        CRASH(Binding_Matrix_Failed);

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
        CRASH(Binding_Matrix_Failed);
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
        CRASH(Binding_Matrix_Failed);

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_Texture", 0)))
        CRASH(Binding_Shader_Failed);


    m_pShaderCom->Begin(2); // AlphaPass

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();
}

HRESULT CCustom_UI::Ready_Prototypes(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const   _wstring    strFilePath = pDesc->strFilePath;
    const   _wstring	strFileName = pDesc->strFileName;
    const   _uint       iNumFiles = pDesc->iNumFiles;

    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::UI);

    // 텍스쳐 프로토타입화
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Texture_Custom_") + strFileName,
        CTexture::Create(m_pDevice, m_pContext, strFilePath.c_str(), iNumFiles))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Texture Load Failed. The texture may have already been loaded.\n");

    return S_OK;
}

HRESULT CCustom_UI::Ready_Components(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const _wstring      strFilePath = pDesc->strFilePath;
    const _wstring	    strFileName = pDesc->strFileName;
    const _uint         iNumFiles   = pDesc->iNumFiles;

    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::UI);

    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Texture_Custom_" + strFileName),
        TEXT("Com_Texture_Custom_") + strFileName, reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

	return S_OK;
}

CCustom_UI* CCustom_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCustom_UI* pInstance = new CCustom_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_ScreenText");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CCustom_UI::Clone(void* pArg)
{
    CCustom_UI* pInstance = new CCustom_UI(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CUI_ScreenText");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCustom_UI::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);

}
