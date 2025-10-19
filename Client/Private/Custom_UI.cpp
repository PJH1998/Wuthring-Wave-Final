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
    m_pAnimator_UICom->Update(fTimeDelta);

    // ksta : 이거 부모가 한번 Update 타이밍에 쏴줘야함
    //Update_CombinedMatrix();

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
    _uint iShaderPassIndex = 2; // alphapass, back cull none

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        CRASH(Binding_Matrix_Failed);

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
        CRASH(Binding_Matrix_Failed);
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
        CRASH(Binding_Matrix_Failed);




    // ksta IF : "g_AlphaStrength" 에 매 프레임마다 Animator_UI 컴포넌트에서 값 갱신중

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_Texture", m_iCurTexIndex)))
        CRASH(Binding_Shader_Failed);


    m_pShaderCom->Begin(iShaderPassIndex);

    m_pVIBufferCom->Bind_Resources();

    m_pVIBufferCom->Render();



    for (auto& child : m_vecChildObjects)
        child->Render();

}

CCustom_UI* CCustom_UI::Find_ChildObject(_wstring strChildName)
{
    for (auto& child : m_vecChildObjects)
    {
        if (child->Get_UIDesc().strUIName == strChildName)
            return child;

        Find_ChildObject(strChildName);
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

HRESULT CCustom_UI::Ready_Components(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const _wstring      strFilePath = pDesc->strFilePath;
    const _wstring	    strFileName = pDesc->strFileName;
    const _uint         iNumFiles = pDesc->iNumFiles;

    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::GAMEPLAY);

    // ksta : 텍스쳐 등 안쓰는 최상위 컨테이너가 호출될 시 여기서 E_FAIL 걸림
    // VIBuffer_Rect 도 그렇고 desc로 조정 가능해야 할 듯 rootdesc 이런식으로 customuidesc 상속받게 해서?
    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Texture_Custom_" + strFileName),
        TEXT("Com_Texture_Custom_") + strFileName, reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;
    
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

    m_tUIDesc.vecChildNames = pDesc->vecChildNames;

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

    for (auto& child : m_vecChildObjects)
        Safe_Release(child);
    m_vecChildObjects.clear();

    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom); 
    Safe_Release(m_pAnimator_UICom);
}
