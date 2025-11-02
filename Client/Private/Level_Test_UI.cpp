#include "ClientPch.h"
#include "Level_Test_UI.h"

#define KSTA_FONTTEXTTEST

CLevel_Test_UI::CLevel_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice, pContext)
{
}

HRESULT CLevel_Test_UI::Initialize()
{
    // HUD ���ӿ�����Ʈ �߰�
    const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
    const _wstring strLayertag_UI = L"Layer_Custom_UI";
    const _wstring strPrototypeTag_UI[] = {
         L"Prototype_GameObject_Custom_UI_Container_HUD"
    };

    for (auto& strPrototypeTag : strPrototypeTag_UI)
    {
        CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
        if (FAILED(m_pGameInstance->Add_RootUI(L"UI_UHD", pTargetUI)))
            CRASH("Failed to Add RootUI to UI_Manager.");
        if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
            CRASH("Failed to Add RootUI to Object_Manager.");
    }

    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Prototype_GameObject_Custom_UI_Container_HUD", iDestLevel, L"Layer_Custom_UI")))
    //    CRASH("Create HUD FAILED.");

    return S_OK;
}

void CLevel_Test_UI::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Test_UI"));





#ifdef KSTA_FONTTEXTTEST
	static _float fTimeElapsed = 0.f;
	const _float fTimeCheckCycle = 1.f;

	fTimeElapsed += fTimeDelta;
	if (fTimeElapsed >= fTimeCheckCycle)
	{
		fTimeElapsed = 0.f;
		//m_pGameInstance->Add_FloatingText(L"WW_Medium", L"This is Test", { 0.f, 0.f }	, 1.f, 3.f, 0, {.5f, .5f, .5f, 1.f});
		//m_pGameInstance->Add_FloatingText(L"WW_Medium", L"This is Test", { 0.5f, 0.5f }	, 1.f, 3.f, 0, {.5f, .5f, .5f, 1.f});
		//m_pGameInstance->Add_FloatingText(L"WW_Medium", L"This is Test", { -0.5f, -0.5f }, 1.f, 3.f, 0, {.5f, .5f, .5f, 1.f});
		_float4 vRandColor = _float4{
			m_pGameInstance->Rand_Normal(),
			m_pGameInstance->Rand_Normal(),
			m_pGameInstance->Rand_Normal(),
			1.f //m_pGameInstance->Rand_Normal()
		};
		_float2 vRandPos = _float2{
			m_pGameInstance->Rand(-100.f, +100.f),
			m_pGameInstance->Rand(-100.f, +100.f)
		};
		FONT_SINGLEDESC tDesc = {};
		tDesc.strFontTag = L"WW_Bold";
		tDesc.strText = L"Test 테스트입니다.";
		tDesc.vScreenPos = { 920.f + vRandPos.x, 1045.f + vRandPos.y };
		tDesc.fScale = 1.f;
		tDesc.vLifeTime = { 0.f, 10.f };
		tDesc.iShaderFlag = ENUM_CLASS(FONT_FLAG::FL_OUTLINE);
		tDesc.vColor = vRandColor;

		tDesc.vOutlineColor = { 0.f, 0.f, 0.f, .1f };
		//tDesc.vFontTexPerPixel = ;// ?
		tDesc.fFontOutlineWidth = .5f;

		//m_pGameInstance->Add_FloatingText(L"WW_Bold", L"Test 테스트입니다.", { 920.f + vRandPos.x, 1045.f + vRandPos.y }, 1.f, 10.f, 0, vRandColor);
		m_pGameInstance->Add_FloatingText(tDesc);
	}
#endif // KSTA_FONTTEXTTEST



}

void CLevel_Test_UI::Render()
{
}

CLevel_Test_UI* CLevel_Test_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Test_UI* pInstance = new CLevel_Test_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Test_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Test_UI::Free()
{
    m_pGameInstance->Clear_RootUI();

    __super::Free();
}
