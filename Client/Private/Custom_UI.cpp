#include "UIObject.h"
#include "ClientPch.h"
#include "Custom_UI.h"
#include "Animator_UI.h"

#include "Event_Level.h"

//#define KSTA_UICLICKTEST
#define KSTA_UIEVENTTEST

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
    
    //__super::Begin();

    return S_OK;
}

void CCustom_UI::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    for (auto& child : m_vecChildObjects)
        child->Priority_Update(fTimeDelta);
}

void CCustom_UI::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    if (m_tUIDesc.isInstance)
        dynamic_cast<CVIBuffer_Rect_Instance_UI*>(m_pVIBufferCom)->Update_Instances(fTimeDelta, m_tUIDesc.vecInstanceDescs);

    if (m_pAnimator_UICom)
        m_pAnimator_UICom->Update(fTimeDelta);

    Update_InputState();
    Update_CacheTransform(fTimeDelta);


    for (auto& child : m_vecChildObjects)
        child->Update(fTimeDelta);


#ifdef KSTA_UICLICKTEST

    if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
    {
        if (Check_IsInSpace())
        {
            std::cout << "Clicked!" << std::endl;
        }

        for (auto& child : m_vecChildObjects)
        {
            if (Check_IsInSpace())
                std::cout << "Clicked!" << std::endl;
        }
    }

#endif // KSTA_UICLICKTEST

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
    if (!m_isActivate)
        return;

    //if (m_tUIDesc.strUIName == L"Background_Dummy")
    //    return;

    if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)            // 짬통 UI용. 필요한 값을 행렬에 임의로 담아 인스턴스별로 던진다. 던져지는 건 vibuffer에서.
        for (_uint i = 0; i < m_tUIDesc.vecInstanceDescs.size(); i++)
            m_tUIDesc.vecInstanceDescs[i].matExtraData = m_cachedVariantUIDesc.matVariantValues[i];

    m_pAnimator_UICom->Render();    // Updates Shader Keyframe Variables.

    if (m_pShaderCom)
    {
        if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)        // 짬통 UI용. 어떤 유형의 UI에 쓸 건지의 Flag를 전역으로 던진다.
            if (FAILED(m_pShaderCom->Bind_Value("g_iVariantFlag", &m_cachedVariantUIDesc.iShaderFlag, sizeof(m_cachedVariantUIDesc.iShaderFlag))))
                CRASH("Binding_Value_Failed");

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

   //for (auto& child : m_vecChildObjects)          // 얘 살려두니까 이중렌더됨.
   //    child->Render();

}

CCustom_UI* CCustom_UI::Find_ChildObject(_wstring strChildName)
{
    for (auto& child : m_vecChildObjects)
    {
        // 부모 검사
        if (child->Get_UIDesc().strUIName == strChildName)
            return child;

        // 이후 자식 재귀검사
        CCustom_UI* child2 = child->Find_ChildObject(strChildName);
        if (child2 != nullptr)
            return child2;
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

_bool CCustom_UI::Check_OnInteract(_uint iEventInteractType, _uint iInstanceIndex)
{
    _bool isSame_InteractType = false;
    _bool isSame_InstanceIndex = false;

    if (iEventInteractType == m_iInputState)
        isSame_InteractType = true;

    if (!m_tUIDesc.isInstance ||
        iInstanceIndex == m_iInputInstanceIndex)
        isSame_InstanceIndex = true;

    _bool isInteracted = (isSame_InteractType && isSame_InstanceIndex);

    return isInteracted;
}

_bool CCustom_UI::Check_OnInteract(_wstring strChildName,_uint iEventInteractType, _uint iInstanceIndex)
{
    return Find_ChildObject(strChildName)->Check_OnInteract(iEventInteractType, iInstanceIndex);
}

_bool CCustom_UI::Check_IsInSpace()
{

#define ISINSPACE(CURSORPOS, RECT_CENTERPOS, RECT_SCALE)    (( (CURSORPOS).x >= ((RECT_CENTERPOS).x - (RECT_SCALE).x / 2.f) &&   \
                                                               (CURSORPOS).x <= ((RECT_CENTERPOS).x + (RECT_SCALE).x / 2.f) &&   \
                                                               (CURSORPOS).y >= ((RECT_CENTERPOS).y - (RECT_SCALE).y / 2.f) &&   \
                                                               (CURSORPOS).y <= ((RECT_CENTERPOS).y + (RECT_SCALE).y / 2.f)) )   \

    _bool isIn_InteractableSpace = false;
    POINT tCursorPos = m_pGameInstance->Get_MousePoint();
    tCursorPos.x = tCursorPos.x - 1920.f / 2.f;
    tCursorPos.y = (1080.f - tCursorPos.y) - 1080.f / 2.f;


    if (!m_tUIDesc.isInstance)
    {
        if (ISINSPACE(tCursorPos, m_vecCachedUITransform[0][POS], m_vecCachedUITransform[0][SCA]))
            isIn_InteractableSpace = true;
    }
    else
    {
        for (_uint i = 0 ; i < m_vecCachedUITransform.size(); i++)
        {
            if (ISINSPACE(tCursorPos, m_vecCachedUITransform[i][POS], m_vecCachedUITransform[i][SCA]))
            {
                isIn_InteractableSpace = true;
                m_iInputInstanceIndex = i;
                break;
            }
        }
    }

    //if (!isIn_InteractableSpace)
    //    m_iInputInstanceIndex = UINT_MAX;

    return isIn_InteractableSpace;
}

void  CCustom_UI::Update_CacheTransform(_float fTimeDelta)   // Caching Calculated Transform Martix. for Optimizing.
{
    // Calculating Time Rate. Const.
    const _float fCachingTimeRate = 0.1f;
    m_cachingTimeElapsed += fTimeDelta;
    if (m_cachingTimeElapsed >= fCachingTimeRate)
        m_cachingTimeElapsed = 0.f;
    else
        return;

    if (!m_tUIDesc.isInstance)
    {
        _vector vPos, vQuat, vSca;
        XMMatrixDecompose(&vSca, &vQuat, &vPos, XMLoadFloat4x4(&m_CombinedWorldMatrix));
        XMStoreFloat4(&m_vecCachedUITransform[0][POS], vPos);
        XMStoreFloat4(&m_vecCachedUITransform[0][ROT], vQuat);
        XMStoreFloat4(&m_vecCachedUITransform[0][SCA], vSca);
    }
    else
    {
        for (_uint i = 0; i < m_tUIDesc.vecInstanceDescs.size(); i++)
        {
            auto& instDesc = m_tUIDesc.vecInstanceDescs[i];

            _float4 instMat[4] = { instDesc.vSInstRight, instDesc.vSInstUp, instDesc.vSInstLook, instDesc.vSInstTrans };
            _matrix instRelativeMat = XMMatrixSet(
                instMat[0].x, instMat[0].y, instMat[0].z, instMat[0].w,
                instMat[1].x, instMat[1].y, instMat[1].z, instMat[1].w,
                instMat[2].x, instMat[2].y, instMat[2].z, instMat[2].w,
                instMat[3].x, instMat[3].y, instMat[3].z, instMat[3].w
            );
            _matrix combinedInstanceMatrix = instRelativeMat * XMLoadFloat4x4(&m_CombinedWorldMatrix);

            _vector vPos, vQuat, vSca;
            XMMatrixDecompose(&vSca, &vQuat, &vPos, combinedInstanceMatrix);
            XMStoreFloat4(&m_vecCachedUITransform[i][POS], vPos);
            XMStoreFloat4(&m_vecCachedUITransform[i][ROT], vQuat);
            XMStoreFloat4(&m_vecCachedUITransform[i][SCA], vSca);
        }
    }
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
    //const   _uint       iDestLevel = ENUM_CLASS(LEVEL::TEST_UI);
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
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
    m_pGameInstance->Subscribe<ONCLICKENTER_UI_EVENT>   (ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickEnterUI",
        [this](const ONCLICKENTER_UI_EVENT event)
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), event.iInstanceIndex))  
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER)); });

    m_pGameInstance->Subscribe<ONCLICKING_UI_EVENT>     (ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickingUI",
        [this](const ONCLICKING_UI_EVENT event)
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICKING), event.iInstanceIndex))     
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICKING)); });

    m_pGameInstance->Subscribe<ONCLICKEXIT_UI_EVENT>    (ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickExitUI",
        [this](const ONCLICKEXIT_UI_EVENT event)        
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_EXIT), event.iInstanceIndex))   
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICK_EXIT)); });

    m_pGameInstance->Subscribe<ONHOVERENTER_UI_EVENT>   (ENUM_CLASS(LEVEL::STATIC), L"Event_OnHoverEnterUI",
        [this](const ONHOVERENTER_UI_EVENT event)       
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER), event.iInstanceIndex))  
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER));});

    m_pGameInstance->Subscribe<ONHOVERING_UI_EVENT>     (ENUM_CLASS(LEVEL::STATIC), L"Event_OnHoveringUI",
        [this](const ONHOVERING_UI_EVENT event)         
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVERING), event.iInstanceIndex))     
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::HOVERING));});

    m_pGameInstance->Subscribe<ONHOVEREXIT_UI_EVENT>    (ENUM_CLASS(LEVEL::STATIC), L"Event_OnHoverExitUI",
        [this](const ONHOVEREXIT_UI_EVENT event)        
        {if ( (m_iInputState == ENUM_CLASS(UI_EVENT_TYPE::HOVER_EXIT)))   // 나갈때는 Check_IsInSpace 체크를 하면 안됨. 나갔으니까 당연히 false 떨어짐;
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::HOVER_EXIT));});

    //m_pGameInstance->Subscribe<ONSCROLL_UI_EVENT>(ENUM_CLASS(STATIC::NONE), L"Event_OnScrollUI",
    //    [this](const ONSCROLL_UI_EVENT event)   {OnEvent(ENUM_CLASS(UI_EVENT_TYPE::SCROLL));});

#ifdef KSTA_UIEVENTTEST

    Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER),   [this]() {std::wcout << "[CCustom_UI] [CLK-I] [" << m_iInputInstanceIndex << "] " << m_tUIDesc.strUIName.c_str() << std::endl; });
    Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::CLICKING),      [this]() {std::wcout << "[CCustom_UI] [CLK--] [" << m_iInputInstanceIndex << "] " << m_tUIDesc.strUIName.c_str() << std::endl; });
    Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::CLICK_EXIT),    [this]() {std::wcout << "[CCustom_UI] [CLK-O] [" << m_iInputInstanceIndex << "] " << m_tUIDesc.strUIName.c_str() << std::endl; });
    Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER),   [this]() {std::wcout << "[CCustom_UI] [HOV-I] [" << m_iInputInstanceIndex << "] " << m_tUIDesc.strUIName.c_str() << std::endl; });
    Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::HOVERING),      [this]() {std::wcout << "[CCustom_UI] [HOV--] [" << m_iInputInstanceIndex << "] " << m_tUIDesc.strUIName.c_str() << std::endl; });
    Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::HOVER_EXIT),    [this]() {std::wcout << "[CCustom_UI] [HOV-O] [" << m_iInputInstanceIndex << "] " << m_tUIDesc.strUIName.c_str() << std::endl; });
    //Add_EventFunction(ENUM_CLASS(UI_EVENT_TYPE::SCROLL),    [this]() {std::cout << "[CCustom_UI] " << m_tUIDesc.strUIName.c_str() << " SCROLLED!" << std::endl; });

#endif // KSTA_UIEVENTTEST

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

    // Get & Store Raw Image Size from SRV.
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

    _uint iCacheTransformAmount = (m_tUIDesc.isInstance)? pDesc->vecInstanceDescs.size() : 1;
    m_vecCachedUITransform.resize(iCacheTransformAmount);
    
    m_tUIDesc.vecInstanceDescs = pDesc->vecInstanceDescs;

    return S_OK;
}

void CCustom_UI::Update_CombinedMatrix(_matrix* pParentMatrix)
{
    if (this->m_tUIDesc.strUIName == L"Icon_Augusta")
        int i = 10;

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

void CCustom_UI::Update_InputState()
{
    if (!m_isActivate)
    {
        m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::NONE);
        return;
    }


    _bool isIn = Check_IsInSpace();


    // Check Click
    if (!m_isClicked &&
        m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN &&
        isIn)
    {
        m_isHovered = false;
        m_isClicked = true;
        // click enter..
        m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER);
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickEnterUI", ONCLICKENTER_UI_EVENT(m_iInputInstanceIndex));
    }
    else if (m_isClicked &&
        m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS &&
        isIn)
    {
        m_isHovered = false;
        m_isClicked = true;
        // clicking..
        m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::CLICKING);
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickingUI", ONCLICKING_UI_EVENT(m_iInputInstanceIndex));
    }
    else if (m_isClicked &&
        m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::UP)
    {
        m_isClicked = false;
        // click exit..
        m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::CLICK_EXIT);
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickExitUI", ONCLICKEXIT_UI_EVENT(m_iInputInstanceIndex));
    }
    else if (!m_isClicked)
    {
        m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::NONE);
    }


    // Check Hover
    if (m_isClicked == true)     // click이 hover보다 우선순위 높음
        return;

    if (isIn)
    {
        if (!m_isHovered)
        {
            m_isHovered = true;
            // hover enter
            m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER);
            m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnHoverEnterUI", ONHOVERENTER_UI_EVENT(m_iInputInstanceIndex));
        }
        else
        {
            // hovering
            m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::HOVERING);
            m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnHoveringUI", ONHOVERING_UI_EVENT(m_iInputInstanceIndex));
        }
    }
    else if (m_isHovered)
    {
        m_isHovered = false;
        // hover exit
        m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::HOVER_EXIT);
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnHoverExitUI", ONHOVEREXIT_UI_EVENT(m_iInputInstanceIndex)); // 이 시점에 이미 m_iInputState 가 0인데?
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
