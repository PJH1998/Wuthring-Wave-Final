#include "UIObject.h"
#include "ClientPch.h"
#include "Custom_UI.h"
#include "Animator_UI.h"

#include "Event_Level.h"

//#define KSTA_UICLICKTEST
//#define KSTA_UIEVENTTEST

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

    if (m_pAnimator_UICom)
        m_pAnimator_UICom->Update(fTimeDelta);

    Update_InputState();
#ifdef KSTA_ON_TRANSFORM_CACHING
	Update_CacheTransform(fTimeDelta);
#endif // KSTA_ON_TRANSFORM_CACHING

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

	if (m_tUIDesc.isInstance)
		dynamic_cast<CVIBuffer_Rect_Instance_UI*>(m_pVIBufferCom)->Update_Instances(m_tUIDesc.vecInstanceDescs);

    for (auto& child : m_vecChildObjects)
        child->Late_Update(fTimeDelta);
}

void CCustom_UI::Render()
{
    if (!m_isActivate)
        return;

	if (m_tUIDesc.strFileName == L"EmptyCanvuspng" ||
		m_tUIDesc.strFileName == L"mspaint_vXzwIq5FDL")			// 계층 나누기용 무의미 투명 텍스쳐면 렌더콜 스킵
		return;


    if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)            // 짬통 UI용. 필요한 값을 행렬에 임의로 담아 인스턴스별로 던진다. 던져지는 건 vibuffer에서.
        for (_uint i = 0; i < m_tUIDesc.vecInstanceDescs.size(); i++)
            m_tUIDesc.vecInstanceDescs[i].matExtraData = m_cachedVariantUIDesc.matVariantValues[i];

	if (m_pAnimator_UICom)
		m_pAnimator_UICom->Render();    // Updates Shader Keyframe Variables.

    if (m_pShaderCom)
    {
        if (m_tUIDesc.isInstance && m_cachedVariantUIDesc.isVariant)        // 짬통 UI용. 어떤 유형의 UI에 쓸 건지의 Flag를 전역으로 던진다.
            if (FAILED(m_pShaderCom->Bind_Value("g_iVariantFlag", &m_cachedVariantUIDesc.iShaderFlag, sizeof(m_cachedVariantUIDesc.iShaderFlag))))
                CRASH("Binding_Value_Failed");

		if (m_tUIDesc.isInstance)
			for (_uint i = 0; i < m_vecExtraTextureCom.size(); i++)
			{
				_string strConstantName = "g_TextureExtra" + to_string(i);
				if (FAILED(m_vecExtraTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, strConstantName.c_str())))
					OutputDebugString(L"[Custom_UI::Render] Texture Bind Failed.");
			}

        if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
            CRASH("Binding_Matrix_Failed");

        if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
            CRASH("Binding_Matrix_Failed");
        if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
            CRASH("Binding_Matrix_Failed");

        if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_Texture", m_iCurTexIndex)))
            CRASH("Binding_Texture_Failed");

        if (FAILED(m_pShaderCom->Bind_Value("g_InverseScreenDiscard", &m_tUIDesc.isInverseScreenDiscard, sizeof(m_tUIDesc.isInverseScreenDiscard))))
            CRASH("Binding_Value_Failed");
        if (FAILED(m_pShaderCom->Bind_Value("g_CutoutAlphaDiscard", &m_tUIDesc.fCutout, sizeof(m_tUIDesc.fCutout))))
            CRASH("Binding_Value_Failed");

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


#ifdef KSTA_ON_TRANSFORM_CACHING
	if (!m_tUIDesc.isInstance)
	{
		if (ISINSPACE(tCursorPos, m_vecCachedUITransform[0][POS], m_vecCachedUITransform[0][SCA]))
			isIn_InteractableSpace = true;
	}
	else
	{
		for (_uint i = 0; i < m_vecCachedUITransform.size(); i++)
		{
			if (ISINSPACE(tCursorPos, m_vecCachedUITransform[i][POS], m_vecCachedUITransform[i][SCA]))
			{
				isIn_InteractableSpace = true;
				m_iInputInstanceIndex = i;
				break;
			}
		}
	}
#endif // KSTA_ON_TRANSFORM_CACHING


#ifndef KSTA_ON_TRANSFORM_CACHING
	if (!m_tUIDesc.isInstance)
	{
		_float2 vPos = _float2{ m_CombinedWorldMatrix._41, m_CombinedWorldMatrix._42 };
		_float2 vSca = _float2{ m_CombinedWorldMatrix._11, m_CombinedWorldMatrix._22 };

		if (ISINSPACE(tCursorPos, vPos, vSca))
			isIn_InteractableSpace = true;
	}
	else
	{
		// inst 도 combine 된 좌표 기준으로 확인 필요.
		
		for (_uint i = 0; i < m_tUIDesc.vecInstanceDescs.size(); i++)
		{
			_float2 vPos = _float2{ 
				m_tUIDesc.vecInstanceDescs[i].vSInstTrans.x + m_CombinedWorldMatrix._41,
				m_tUIDesc.vecInstanceDescs[i].vSInstTrans.y + m_CombinedWorldMatrix._42
			};
			_float2 vSca = _float2{
				m_tUIDesc.vecInstanceDescs[i].vSInstRight.x * m_CombinedWorldMatrix._11,
				m_tUIDesc.vecInstanceDescs[i].vSInstUp.y	* m_CombinedWorldMatrix._22
			};

			if (ISINSPACE(tCursorPos, vPos, vSca))
			{
				isIn_InteractableSpace = true;
				m_iInputInstanceIndex = i;
				break;
			}
		}
	}
#endif // KSTA_ON_TRANSFORM_CACHING


    //if (!isIn_InteractableSpace)
    //    m_iInputInstanceIndex = UINT_MAX;

    return isIn_InteractableSpace;
}

#ifdef KSTA_ON_TRANSFORM_CACHING
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
#endif // KSTA_ON_TRANSFORM_CACHING

HRESULT CCustom_UI::Add_ExtraTexture(_wstring strFileName)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();
	CTexture* pExtraTexture = nullptr;

	if (FAILED(CGameObject::Add_Component(iDestLevel, TEXT("Prototype_Component_Texture_Custom_" + strFileName),
		TEXT("Com_Texture_Custom_") + strFileName, reinterpret_cast<CComponent**>(&pExtraTexture), nullptr)))
		CRASH("해당하는 텍스쳐 프로토타입 없음.");

	if (pExtraTexture)
		m_vecExtraTextureCom.push_back(pExtraTexture);

	return S_OK;
}

HRESULT CCustom_UI::Ready_Components(void* pArg)
{
    ASSERT_CRASH(pArg);
    CUSTOM_UI_DESC* pDesc = static_cast<CUSTOM_UI_DESC*>(pArg);

    const _wstring      strFilePath = pDesc->strFilePath;
    const _wstring	    strFileName = pDesc->strFileName;
    const _uint         iNumFiles = pDesc->iNumFiles;

	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
    const   _bool       isInstance = pDesc->isInstance;

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
	if (m_tUIDesc.iUIType != ENUM_CLASS(UI_TYPE::BUTTON))
		return S_OK;



    m_pGameInstance->Subscribe<ONCLICKENTER_UI_EVENT>   (ENUM_CLASS(STATIC::NONE), L"Event_OnClickEnterUI",
        [this](const ONCLICKENTER_UI_EVENT event)
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), event.iInstanceIndex))  
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER)); });

    m_pGameInstance->Subscribe<ONCLICKING_UI_EVENT>     (ENUM_CLASS(STATIC::NONE), L"Event_OnClickingUI",
        [this](const ONCLICKING_UI_EVENT event)
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICKING), event.iInstanceIndex))     
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICKING)); });

    m_pGameInstance->Subscribe<ONCLICKEXIT_UI_EVENT>    (ENUM_CLASS(STATIC::NONE), L"Event_OnClickExitUI",
        [this](const ONCLICKEXIT_UI_EVENT event)        
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_EXIT), event.iInstanceIndex))   
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::CLICK_EXIT)); });

    m_pGameInstance->Subscribe<ONHOVERENTER_UI_EVENT>   (ENUM_CLASS(STATIC::NONE), L"Event_OnHoverEnterUI",
        [this](const ONHOVERENTER_UI_EVENT event)       
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER), event.iInstanceIndex))  
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER));});

    m_pGameInstance->Subscribe<ONHOVERING_UI_EVENT>     (ENUM_CLASS(STATIC::NONE), L"Event_OnHoveringUI",
        [this](const ONHOVERING_UI_EVENT event)         
        {if (Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVERING), event.iInstanceIndex))     
        OnEvent(ENUM_CLASS(UI_EVENT_TYPE::HOVERING));});

    m_pGameInstance->Subscribe<ONHOVEREXIT_UI_EVENT>    (ENUM_CLASS(STATIC::NONE), L"Event_OnHoverExitUI",
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

#ifdef KSTA_ON_TRANSFORM_CACHING
	_uint iCacheTransformAmount = (m_tUIDesc.isInstance) ? pDesc->vecInstanceDescs.size() : 1;
	m_vecCachedUITransform.resize(iCacheTransformAmount);
#endif // KSTA_ON_TRANSFORM_CACHING

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

void CCustom_UI::Update_CombinedDesc(CAnimator_UI* pParentAnimatorCom)
{
	if (m_pAnimator_UICom)
	{
		if (!pParentAnimatorCom)
		{
			auto thisCalcedKFDesc = m_pAnimator_UICom->Get_CalcedAnimKeyframeDesc();
			if (thisCalcedKFDesc)
				m_pAnimator_UICom->Set_CurCombinedAnimKeyframeDesc(*thisCalcedKFDesc);
		}
		else
		{
			auto pParentKFDesc = pParentAnimatorCom->Get_CurCombinedAnimKeyframeDesc();
			auto thisCalcedKFDesc = m_pAnimator_UICom->Get_CalcedAnimKeyframeDesc();

			if (pParentKFDesc)
			{
				CAnimator_UI::UI_ANIM_KEYFRAME_DESC tDesc = {};
				tDesc = *thisCalcedKFDesc;

				// 일단은 Alpha만 연결되도록.. 
				tDesc.fAlpha = 1.f - ((1.f - thisCalcedKFDesc->fAlpha) * (1.f - pParentKFDesc->fAlpha)); // 다시 사라짐 값으로 되돌림
				m_pAnimator_UICom->Set_CurCombinedAnimKeyframeDesc(tDesc);
			}

		}
	}



	// transfer to child..
	for (auto& child : m_vecChildObjects)
		child->Update_CombinedDesc(m_pAnimator_UICom);

}

void CCustom_UI::Update_InputState()
{
    if (!m_isActivate ||
		m_tUIDesc.iUIType != ENUM_CLASS(UI_TYPE::BUTTON))
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

HRESULT CCustom_UI::Load_ChildObjects(_wstring strFilePath)
{
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();

	// parse json
	ifstream file(strFilePath);
	json jUITreeData = {};
	if (file.is_open()) { file >> jUITreeData; }
	CUSTOM_UITREE_DESC tLoadTreeDesc = {};
	from_json(jUITreeData, tLoadTreeDesc);

	// load objects
	vector<CGameObject*> vecLoadObjects = {};
	for (auto& loadDesc : tLoadTreeDesc.vecUIInfoDescs)
	{
		UI_INFO_DESC tLoadUIInfoDesc = loadDesc;

		// Transform ���� ������ ��, ���ȭ�Ͽ� �ݿ��ϰ�, (�ӽ÷�) �ڽ� ������Ʈ�ν� �߰��Ѵ�.
		_float3 vCurObjPos = tLoadUIInfoDesc.vPos;
		_float3 vCurObjRot = tLoadUIInfoDesc.vRot;
		_float3 vCurObjSca = tLoadUIInfoDesc.vSca;

		CGameObject* pCustomObj = nullptr;
		switch (tLoadUIInfoDesc.tUIDesc.iUIType)
		{
		case ENUM_CLASS(UI_TYPE::NONE):   pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Image", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc));  break;
		case ENUM_CLASS(UI_TYPE::BUTTON): pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_Button", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc)); break;
		default:            break;
		}
		m_vecChildObjects.push_back(static_cast<CCustom_UI*>(pCustomObj)); // ���ÿ� ����.. 


		HIERARCHY_OBJ_DESC tObjDesc = { };
		tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
		tObjDesc.strObjName = tLoadUIInfoDesc.tUIDesc.strUIName;

		_matrix matScale = XMMatrixScaling(vCurObjSca.x, vCurObjSca.y, vCurObjSca.z);
		_matrix matRotX = XMMatrixRotationX(DegreesToRadians(vCurObjRot.x));
		_matrix matRotY = XMMatrixRotationY(DegreesToRadians(vCurObjRot.y));
		_matrix matRotZ = XMMatrixRotationZ(DegreesToRadians(vCurObjRot.z));
		_matrix matRot = matRotZ * matRotY * matRotX;
		_matrix matTrans = XMMatrixTranslation(vCurObjPos.x, vCurObjPos.y, vCurObjPos.z);

		_matrix matWorld = matScale * matRot * matTrans;
		static_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"))->Set_WorldMatrix(matWorld);
	}

	// re-define childs of objects
	for (auto& child : m_vecChildObjects)
	{
		CUSTOM_UI_DESC tChildDesc = child->Get_UIDesc();
		for (auto& otherChild : m_vecChildObjects)
		{
			CUSTOM_UI_DESC tOtherChildDesc = otherChild->Get_UIDesc();

			for (auto& childName : tChildDesc.vecChildNames)
			{
				if (childName == tOtherChildDesc.strUIName)
					child->Add_Child(otherChild);
			}
		}
	}

	// re-define childs of this(container)
	vector<CCustom_UI*> vecTrueChildObjects = {};
	for (auto& child : m_vecChildObjects)
	{
		if (child->Get_UIDesc().strParentName.empty())
			vecTrueChildObjects.push_back(child);
	}

	m_vecChildObjects = move(vecTrueChildObjects);

	return S_OK;
}

HRESULT CCustom_UI::Load_Animations(vector<_wstring> vecAnimFilePath)
{
	for (auto& animPath : vecAnimFilePath)
	{
		// parse json
		ifstream file(animPath);
		json jUIAnimData = {};
		if (file.is_open()) { file >> jUIAnimData; }
		CAnimator_UI::UI_ANIM_DESC tLoadAnimDesc = {};
		from_json(jUIAnimData, tLoadAnimDesc);

		CCustom_UI* pTargetObject = Find_ChildObject(tLoadAnimDesc.tUIDesc.strUIName);

		if (!pTargetObject)
			CRASH("Cannot find targetobject");
		CAnimator_UI* pTargetAnimator = dynamic_cast<CAnimator_UI*>(pTargetObject->Get_Component(L"Com_Animator_UI"));

		pTargetAnimator->Insert_Animation(tLoadAnimDesc);
	}

	return S_OK;
}

void CCustom_UI::Free()
{
    __super::Free(); 

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);
    
	Safe_Release(m_pTextureCom); 

	for (auto& extraTextureCom : m_vecExtraTextureCom)		Safe_Release(extraTextureCom);
	m_vecExtraTextureCom.clear();

    Safe_Release(m_pAnimator_UICom);

    for (auto& child : m_vecChildObjects)					Safe_Release(child);
    m_vecChildObjects.clear();
}
