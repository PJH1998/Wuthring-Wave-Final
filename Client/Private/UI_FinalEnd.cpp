#include "ClientPch.h"

#include "UI_FinalEnd.h"
#include "GameSystem.h"
#include "Animator_UI.h"
#include "UI_Text.h"


CUI_FinalEnd::CUI_FinalEnd(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_FinalEnd::CUI_FinalEnd(const CUI_FinalEnd& Prototype)
	: CUI_Image(Prototype)
	, m_pGameSystem (CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_FinalEnd::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_FinalEnd::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	CUI_FinalEnd::Ready_Components(pArg);

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_FinalEnd.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	Create_ChildText();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Image_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Image_Play.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Grad_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Grad_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Grad_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Black_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Black_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Black_FadeOut.json",
		
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Name_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_Name_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_TY_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/End_TY_FadeIn.json",
	};
	Load_Animations(vecAnimFilePaths);

	m_pUIAnim_MainImage	->Change_Animation(L"End_Image_Initialize");
	m_pUIAnim_FadeAll	->Change_Animation(L"End_Black_Initialize");
	m_pUIAnim_FadeGrad	->Change_Animation(L"End_Grad_Initialize");

	m_pUIAnim_Names		->Change_Animation(L"End_Name_Initialize");
	m_pUIAnim_TY		->Change_Animation(L"End_TY_Initialize");


	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_FinalEnd", this);


	return S_OK;
}

void CUI_FinalEnd::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_FinalEnd::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
	if (!m_isStart)
		return;

	Update_AnimOrder(fTimeDelta);

	m_fElapsedTime += fTimeDelta;
	__super::Update(fTimeDelta);
}

void CUI_FinalEnd::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_FinalEnd::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_FinalEnd::PreAssign_ChildUIs()
{
	m_pRUI_All		= Find_ChildObject(L"Sub_All");

	m_pUI_MainImage	= Find_ChildObject(L"SectorA_Image");
	m_pUI_FadeAll	= Find_ChildObject(L"SectorA_FadeAll");
	m_pUI_FadeGrad	= Find_ChildObject(L"SectorA_FadeGrad");

	m_pUI_Names		= Find_ChildObject(L"SectorB_Names");
	m_pUI_TY		= Find_ChildObject(L"SectorB_Thankyou");



	m_pUIAnim_MainImage	= dynamic_cast<CAnimator_UI*>(m_pUI_MainImage	->Get_Component(L"Com_Animator_UI"));
	m_pUIAnim_FadeAll	= dynamic_cast<CAnimator_UI*>(m_pUI_FadeAll		->Get_Component(L"Com_Animator_UI"));
	m_pUIAnim_FadeGrad	= dynamic_cast<CAnimator_UI*>(m_pUI_FadeGrad	->Get_Component(L"Com_Animator_UI"));

	m_pUIAnim_Names		= dynamic_cast<CAnimator_UI*>(m_pUI_Names		->Get_Component(L"Com_Animator_UI"));
	m_pUIAnim_TY		= dynamic_cast<CAnimator_UI*>(m_pUI_TY			->Get_Component(L"Com_Animator_UI"));

}

void CUI_FinalEnd::Update_AnimOrder(_float fTimeDelta)
{
	// == Images
	const array<_float, 4> arrKeyframes = { 0.f, 3.f, 5.f, 7.f };

	if		(m_fElapsedTime >= arrKeyframes[0] &&
			m_iAnimOrder == 0)
	{
		m_pUIAnim_FadeAll->Change_Animation(L"End_Black_FadeIn", true);

		m_iAnimOrder++;
	}
	else if (m_fElapsedTime >= arrKeyframes[1] &&
			m_iAnimOrder == 1)
	{
		m_pUIAnim_FadeAll->Change_Animation(L"End_Black_FadeOut", true);
		m_pUIAnim_MainImage->Change_Animation(L"End_Image_Play", true);

		m_iAnimOrder++;
	}
	else if ((m_fElapsedTime >= arrKeyframes[2] &&
			m_iAnimOrder == 2))
	{
		m_pUIAnim_FadeGrad->Change_Animation(L"End_Black_FadeIn", true);

		m_iAnimOrder++;
	}
	else if ((m_fElapsedTime >= arrKeyframes[3] &&
			m_iAnimOrder == 3))
	{
		m_pUIAnim_FadeGrad->Change_Animation(L"End_Grad_FadeIn", true);

		m_iAnimOrder++;
	}



	// == Texts
	const array<_float, 4> arrTextKeyframes = { 7.5f, 7.5f, 8.3f, 8.3f };

	if		(m_fElapsedTime >= arrTextKeyframes[0] &&
			m_iTextAnimOrder == 0)
	{
		m_pUIAnim_Names->Change_Animation(L"End_Name_FadeIn", true);


		m_iTextAnimOrder++;
	}
	else if	(m_fElapsedTime >= arrTextKeyframes[1] &&
			m_iTextAnimOrder == 1)
	{
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles1)	->Change_Text(L"Framework");
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles2)	->Change_Text(L"Animation");
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles3)	->Change_Text(L"UI");
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles4)	->Change_Text(L"Map");
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles5)	->Change_Text(L"Effect");
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles6)	->Change_Text(L"Shader");
		static_cast<CUI_Text*>(m_pTextUI_TeamRoles7)	->Change_Text(L"AI");

		//static_cast<CUI_Text*>(m_pTextUI_TeamNames1)	->Change_Text(L"박지호 노영훈 김기훈 신우혁 임은비 김정훈 이진호");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames1)	->Change_Text(L"박지호");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames2)	->Change_Text(L"노영훈");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames3)	->Change_Text(L"김기훈");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames4)	->Change_Text(L"신우혁");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames5)	->Change_Text(L"임은비");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames6)	->Change_Text(L"김정훈");
		static_cast<CUI_Text*>(m_pTextUI_TeamNames7)	->Change_Text(L"이진호");
		m_iTextAnimOrder++;
	}
	else if (m_fElapsedTime >= arrTextKeyframes[2] &&
			m_iTextAnimOrder == 2)
	{
		m_pUIAnim_TY->Change_Animation(L"End_TY_FadeIn", true);

		m_iTextAnimOrder++;
	}
	else if (m_fElapsedTime >= arrTextKeyframes[3] &&
			m_iTextAnimOrder == 3)
	{
		static_cast<CUI_Text*>(m_pTextUI_Thankyou)->Change_Text(L"감사합니다.");
		m_iTextAnimOrder++;
	}

}

HRESULT CUI_FinalEnd::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_FinalEnd::Ready_Presets()
{

}

void CUI_FinalEnd::Create_ChildText()
{
	// 생성.						..나중에 타입, 위치, 크기 설정필요!
	_float2 vTextPos;
	CUI_Text* pFont;
	CCustom_UI* pAttacher;

	// ==============================
	// * 1-1. roles
	// ==============================
	vTextPos = { -500.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole1"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles1 = pFont;

	vTextPos = { -400.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole2"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles2 = pFont;

	vTextPos = { -300.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole3"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles3 = pFont;

	vTextPos = { -200.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole4"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles4 = pFont;

	vTextPos = { -100.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole5"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles5 = pFont;

	vTextPos = { -000.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole6"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles6 = pFont;

	vTextPos = { +100.f, 420.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.25f,
		L"UI_Text_EndRole7"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamRoles7 = pFont;



	// ==============================
	// * 2-1. names
	// ==============================
	vTextPos = { -500.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName1"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames1 = pFont;

	vTextPos = { -400.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName2"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames2 = pFont;

	vTextPos = { -300.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName3"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames3 = pFont;

	vTextPos = { -200.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName4"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames4 = pFont;

	vTextPos = { -100.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName5"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames5 = pFont;

	vTextPos = { -000.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName6"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames6 = pFont;

	vTextPos = { +100.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.45f,
		L"UI_Text_EndName7"
	);
	pAttacher = m_pUI_Names;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_TeamNames7 = pFont;



	// ==============================
	// * 3. ty
	// ==============================
	vTextPos = { 500.f, 450.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"",
		TEXT_COLOR_TYPE::TT_TITLE,
		1.0f,
		L"UI_Text_EndTY"
	);
	pAttacher = m_pUI_TY;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::RIGHT);

	m_pTextUI_Thankyou = pFont;

}

CUI_FinalEnd* CUI_FinalEnd::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_FinalEnd* pInstance = new CUI_FinalEnd(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_FinalEnd");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_FinalEnd::Clone(void* pArg)
{
	CUI_FinalEnd* pInstance = new CUI_FinalEnd(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_FinalEnd");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_FinalEnd::Free()
{
	Safe_Release(m_pGameSystem);

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_FinalEnd");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
