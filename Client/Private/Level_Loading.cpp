#include "ClientPch.h"
#include "Level_Loading.h"

#include "Event_Level.h"

// ========Loader========
#include "Loader_Logo.h"
//#include "Loader_GamePlay.h"
//#include "Loader_Lord.h"
//#include "Loader_Test.h"
// =====================
//#include "BackGround.h"
//#include "LoadingBar.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevel)
{
    m_eNextLevel = eNextLevel;

    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Event()))
        return E_FAIL;

    if (FAILED(Ready_LoadingThread()))
        return E_FAIL;

    if (FAILED(Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
    if (true == m_isFinished || true == m_pLoader->IsFinished())
    {
        CHANGE_LEVEL_EVENT event{ m_eNextLevel, false };
        m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Event_Change_Level"), event);
    }
}

void CLevel_Loading::Render()
{
}

HRESULT CLevel_Loading::Ready_Prototype()
{
    return S_OK;
}

HRESULT CLevel_Loading::Ready_Event()
{
    m_pGameInstance->Subscribe<LOADING_END_EVENT>(ENUM_CLASS(STATIC::NONE), TEXT("Event_Loading_End"), [this](const LOADING_END_EVENT& event) {
            if(false == m_isFinished)
                m_isFinished = event.isFinish;
        });

    return S_OK;
}

HRESULT CLevel_Loading::Ready_LoadingThread()
{
    switch (m_eNextLevel)
    {
    case LEVEL::LOGO:
        m_pLoader = CLoader_Logo::Create(m_pDevice, m_pContext);
        break;
    }

	ASSERT_CRASH(m_pLoader);

    return S_OK;
}

HRESULT CLevel_Loading::Ready_GameObject()
{
    //// BackGroud
    //CUIObject::UIOBJECT_DESC UIObjectDesc = {};
    //UIObjectDesc.fX = g_iWinSizeX >> 1;
    //UIObjectDesc.fY = g_iWinSizeY >> 1;
    //UIObjectDesc.fSizeX = g_iWinSizeX;
    //UIObjectDesc.fSizeY = g_iWinSizeY;
    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::LOADING), TEXT("Prototype_GameObject_BackGround"),
    //    ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_BackGround"), &UIObjectDesc)))
    //    return E_FAIL;
	//
    //// LoadingBar
    //CUIObject::UIOBJECT_DESC LoadingBarDesc = {};
    //LoadingBarDesc.fX = g_iWinSizeX >> 1;
    //LoadingBarDesc.fY = g_iWinSizeY - 50;
    //LoadingBarDesc.fSizeX = g_iWinSizeX - 20.f;
    //LoadingBarDesc.fSizeY = 20.f;
    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::LOADING), TEXT("Prototype_GameObject_LoadingBar"),
    //    ENUM_CLASS(LEVEL::LOADING), TEXT("Layer_BackGround"), &LoadingBarDesc)))
    //    return E_FAIL;

    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevel)
{
    CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevel)))
    {
        MSG_BOX("Failed to Create : Level_Loading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Loading::Free()
{
    __super::Free();

    Safe_Release(m_pLoader);
}
