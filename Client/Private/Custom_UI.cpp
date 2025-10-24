#include "ClientPch.h"
#include "Custom_UI.h"
#include "Animator_UI.h"

#include "Event_Level.h"

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

    //Ready_Prototypes(pArg);
    Ready_Components(pArg);
    Ready_Events();

    Bind_Description(pArg);
    
    
    __super::Begin();

    return S_OK;
}

void CCustom_UI::Priority_Update(_float fTimeDelta)
{

    for (auto& child : m_vecChildObjects)
        child->Priority_Update(fTimeDelta);
}

void CCustom_UI::Update(_float fTimeDelta)
{
    if (m_tUIDesc.isInstance)
        dynamic_cast<CVIBuffer_Rect_Instance_UI*>(m_pVIBufferCom)->Update_Instances(fTimeDelta, m_tUIDesc.vecInstanceDescs);

    if (m_pAnimator_UICom)
        m_pAnimator_UICom->Update(fTimeDelta);

    for (auto& child : m_vecChildObjects)
        child->Update(fTimeDelta);
}

void CCustom_UI::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::UI, this)))
        return;

    for (auto& child : m_vecChildObjects)
        child->Late_Update(fTimeDelta);
}

void CCustom_UI::Render()
{
    if (m_pShaderCom)
    {
        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
            CRASH("Binding_Matrix_Failed");

        if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
            CRASH("Binding_Matrix_Failed");
        if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
            CRASH("Binding_Matrix_Failed");

        if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_Texture", m_iCurTexIndex)))
            CRASH("Binding_Matrix_Failed");
        if (FAILED(m_pShaderCom->Bind_Value("g_InverseScreenDiscard", &m_tUIDesc.isInverseScreenDiscard, sizeof(m_tUIDesc.isInverseScreenDiscard))))
            CRASH("Binding_Value_Failed");
        if (FAILED(m_pShaderCom->Bind_Value("g_CutoutAlphaDiscard", &m_tUIDesc.fCutout, sizeof(m_tUIDesc.fCutout))))
            CRASH("Binding_Value_Failed");

        // �̹��� ũ�� �Ѱ��ֱ�
        if (FAILED(m_pShaderCom->Bind_Value("g_ImageSize", &m_tUIDesc.vecSize[m_iCurTexIndex], sizeof(m_tUIDesc.vecSize[m_iCurTexIndex]))))
            CRASH("Binding_Value_Failed");
        if (FAILED(m_pShaderCom->Bind_Value("g_SectorBorder", &m_tUIDesc.vSectorBorder, sizeof(m_tUIDesc.vSectorBorder))))
            CRASH("Binding_Value_Failed");
        if (FAILED(m_pShaderCom->Bind_Value("g_UIScale", &m_tUIDesc.fUIScale, sizeof(m_tUIDesc.fUIScale))))
            CRASH("Binding_Value_Failed");





        m_pShaderCom->Begin(m_tUIDesc.iPassType);

        m_pVIBufferCom->Bind_Resources();

        m_pVIBufferCom->Render();
    }

    for (auto& child : m_vecChildObjects)
        child->Render();

}

CCustom_UI* CCustom_UI::Find_ChildObject(_wstring strChildName)
{
    for (auto& child : m_vecChildObjects)
    {
        if (child->Get_UIDesc().strUIName == strChildName)
            return child;

        return child->Find_ChildObject(strChildName);
    }

    return nullptr;
}

void CCustom_UI::Add_EventFunction(_uint iEventType, function<void()> function)
{
    m_vecFunctions[iEventType].push_back(function);
}

void CCustom_UI::OnEvent(_uint iEventType)
{
    for (auto& func : m_vecFunctions[iEventType])
        func();
}

/*
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
*/

//HRESULT CCustom_UI::Ready_Prototypes

HRESULT CCustom_UI::Ready_Components(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const _wstring      strFilePath = pDesc->strFilePath;
    const _wstring	    strFileName = pDesc->strFileName;
    const _uint         iNumFiles = pDesc->iNumFiles;

    //const   _uint       iDestLevel = ENUM_CLASS(LEVEL::GAMEPLAY);
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);
    const   _bool       isInstance = pDesc->isInstance;


    // ksta : 텍스쳐 등 안쓰는 최상위 컨테이너가 호출될 시 여기서 E_FAIL 걸림
    // VIBuffer_Rect 도 그렇고 desc로 조정 가능해야 할 듯 rootdesc 이런식으로 customuidesc 상속받게 해서?
    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Texture_Custom_" + strFileName),
        TEXT("Com_Texture_Custom_") + strFileName, reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (!isInstance) {
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
            TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
            return E_FAIL;
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
            TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
            return E_FAIL;
    }
    else if (isInstance) {
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
            TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
            return E_FAIL;
        if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
            TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
            return E_FAIL;
    }
    
    CAnimator_UI::ANIMATOR_UI_DESC tAnimatorUIDesc = { this };
    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Animator_UI"),
        TEXT("Com_Animator_UI"), reinterpret_cast<CComponent**>(&m_pAnimator_UICom), &tAnimatorUIDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CCustom_UI::Ready_Events()
{
    m_pGameInstance->Subscribe<ONCLICK_UI_EVENT>(ENUM_CLASS(STATIC::NONE), L"Event_OnClickUI",
        [this](const ONCLICK_UI_EVENT event){OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICK)); });
    m_pGameInstance->Subscribe<ONHOVER_UI_EVENT>(ENUM_CLASS(STATIC::NONE), L"Event_OnHoverUI",
        [this](const ONHOVER_UI_EVENT event){OnEvent(ENUM_CLASS(UI_EVENT_TYPE::HOVER));});
    m_pGameInstance->Subscribe<ONSCROLL_UI_EVENT>(ENUM_CLASS(STATIC::NONE), L"Event_OnScrollUI",
        [this](const ONSCROLL_UI_EVENT event){OnEvent(ENUM_CLASS(UI_EVENT_TYPE::SCROLL));});

    return S_OK;
}

HRESULT CCustom_UI::Bind_Description(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc   = static_cast<CUSTOM_UI_DESC*>(pArg);

    m_tUIDesc.strFilePath   = pDesc->strFilePath;
    m_tUIDesc.strFileName   = pDesc->strFileName;
    m_tUIDesc.iNumFiles     = pDesc->iNumFiles;

    m_tUIDesc.strUIName     = ((pDesc->strUIName).empty()) ? m_tUIDesc.strFileName : pDesc->strUIName;
    m_tUIDesc.iUIType       = pDesc->iUIType;
    m_tUIDesc.strParentName = pDesc->strParentName;
    m_tUIDesc.fCutout       = pDesc->fCutout;
    m_tUIDesc.iPassType     = pDesc->iPassType;
    m_tUIDesc.vecChildNames = pDesc->vecChildNames;

    uint iIndex = 0;
    while (true)
    {
        ID3D11ShaderResourceView* pSRV = m_pTextureCom->Get_SRV(iIndex);
        if (pSRV == nullptr) break;
        ID3D11Resource* pResource;
        pSRV->GetResource(&pResource);
        ID3D11Texture2D* pTexture;
        if (pResource)
        {
            HRESULT hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
            if (SUCCEEDED(hr) && pTexture)
            {
                D3D11_TEXTURE2D_DESC desc = {};
                pTexture->GetDesc(&desc);
                m_tUIDesc.vecSize.push_back(_float2{ (_float)desc.Width , (_float)desc.Height });
                pTexture->Release();
            }
            pResource->Release();
        }
        iIndex++;
    }

    m_tUIDesc.vSectorBorder = pDesc->vSectorBorder;
    m_tUIDesc.fUIScale      = pDesc->fUIScale;
    m_tUIDesc.isInstance    = pDesc->isInstance;

    m_tUIDesc.vecInstanceDescs = pDesc->vecInstanceDescs;

    return S_OK;
}

void CCustom_UI::Update_CombinedMatrix(_matrix* pParentMatrix)
{
    if (pParentMatrix)
        XMStoreFloat4x4(&m_CombinedWorldMatrix, m_pTransformCom->Get_WorldMatrix() * *pParentMatrix);
    else
        XMStoreFloat4x4(&m_CombinedWorldMatrix, m_pTransformCom->Get_WorldMatrix());

    for (auto& child : m_vecChildObjects)
    {
        _matrix LoadCombinedMatrix = XMLoadFloat4x4(&m_CombinedWorldMatrix);
        child->Update_CombinedMatrix(&LoadCombinedMatrix);
    }
}

void CCustom_UI::Free()
{
    m_pGameInstance->Unscribe();
    __super::Free(); 

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom); 
    Safe_Release(m_pAnimator_UICom);

    for (auto& child : m_vecChildObjects)
        Safe_Release(child);
    m_vecChildObjects.clear();
}
