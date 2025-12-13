#include "ClientPch.h"
#include "UI_QuestIndicator.h"
#include "GameSystem.h"
#include "UI_Text.h"
#include "Animator_UI.h"

CUI_QuestIndicator::CUI_QuestIndicator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_QuestIndicator::CUI_QuestIndicator(const CUI_QuestIndicator& Prototype)
	: CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_QuestIndicator::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_QuestIndicator::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	CUI_QuestIndicator::Ready_Components(pArg);
	PreAssign_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Quest.json"; //확인헤야함
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	Create_ChildText();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_BG_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_BG_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_BG_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Noti_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Noti_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Noti_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Comp_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Comp_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Comp_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Side_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Side_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Quest_Side_FadeOut.json",
	};
	Load_Animations(vecAnimFilePaths);

	//static_cast<CAnimator_UI*>(m_pUI_SectorA_Images->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Dialog_Initialize");

	m_AnimUI_BG->Change_Animation(L"Quest_BG_Initialize");
	m_AnimUI_Noti->Change_Animation(L"Quest_Noti_Initialize");
	m_AnimUI_Comp->Change_Animation(L"Quest_Comp_Initialize");
	m_AnimUI_Side->Change_Animation(L"Quest_Side_Initialize");



	//m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_QuestIndicator", this);

	return S_OK;
}

void CUI_QuestIndicator::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_QuestIndicator::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


	//Update_DialogOrder(fTimeDelta);
	if (m_isQuestActive)
	{
		Update_AnimOrder(fTimeDelta);

		Update_StartEvent(fTimeDelta);
		Update_EndEvent(fTimeDelta);

		Update_GoinDisable(fTimeDelta);		// 실질적 Inactive
	}
	


	__super::Update(fTimeDelta);
}

void CUI_QuestIndicator::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	//m_fTickElapsedTime += fTimeDelta;
	__super::Late_Update(fTimeDelta);
}

void CUI_QuestIndicator::Render()
{
	if (!m_isActivate)
		return;
	if (!m_isQuestActive)
		return;
}

void CUI_QuestIndicator::Trigger_AddQuestProgress()
{
	m_iProgress++;

#ifdef _DEBUG
	std::cout << "[UI_QuestIndicator] Filled to " << m_iProgress << std::endl;
#endif // _DEBUG
	
	_wstring str = L"찾은 아이 : " + to_wstring(m_iProgress) + L" / " + to_wstring(m_iMaxProgress);
	static_cast<CUI_Text*>(m_pTextUI_SideDesc2)->Change_Text(str);
}

void CUI_QuestIndicator::Update_AnimOrder(_float fTimeDelta)
{
	if (!m_isStarted)
	{
		m_isStarted = true;
		m_isOnEvent_GoinStart = true;
	}

	const _bool isEndAble = (m_iProgress >= m_iMaxProgress);
	if (!m_isEnded && isEndAble)
	{
		m_isEnded = true;
		m_isOnEvent_GoinEnd = true;
	}
}

void CUI_QuestIndicator::Update_GoinDisable(_float fTimeDelta)
{
	if (!m_isGoinDisable)
		return;

	
	if (m_fDisableTimer >= m_fDisableTime)
	{
		m_isActivate = false;
		m_isQuestActive = false;
	}

	m_fDisableTimer += fTimeDelta;
}

void CUI_QuestIndicator::Update_StartEvent(_float fTimeDelta)
{
	if (!m_isOnEvent_GoinStart)
		return;

	const array<_float, 4> arrKeyTimes = { 0.f, 0.5f, 3.5f, 4.0f };


	if		(m_fStartTimer >= arrKeyTimes[0] &&			m_iStartEventOrder == 0)
	{
		m_AnimUI_BG->Change_Animation(L"Quest_BG_FadeIn");

		m_iStartEventOrder++;
	}
	else if(m_fStartTimer >= arrKeyTimes[1] &&			m_iStartEventOrder == 1)
	{
		m_AnimUI_Noti->Change_Animation(L"Quest_Noti_FadeIn");
		m_pGameInstance->Play_Sound(L"UI_QuestAccepted", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.35f);

		m_iStartEventOrder++;
	}
	else if(m_fStartTimer >= arrKeyTimes[2] &&			m_iStartEventOrder == 2)
	{
		m_AnimUI_BG->Change_Animation(L"Quest_BG_FadeOut");
		m_AnimUI_Noti->Change_Animation(L"Quest_Noti_FadeOut");

		m_iStartEventOrder++;
	}
	else if(m_fStartTimer >= arrKeyTimes[3] &&			m_iStartEventOrder == 3)
	{
		m_AnimUI_Side->Change_Animation(L"Quest_Side_FadeIn");

		m_iStartEventOrder++;
	}

	_bool condition = m_fStartTimer >= arrKeyTimes.back();
	if (condition)		m_isOnEvent_GoinStart = false;
	m_fStartTimer += fTimeDelta;
}

void CUI_QuestIndicator::Update_EndEvent(_float fTimeDelta)
{
	if (!m_isOnEvent_GoinEnd)
		return;

	
	const array<_float, 4> arrKeyTimes = { 0.f, 1.5f, 2.0f, 5.0f };


	if		(m_fEndTimer >= arrKeyTimes[0] &&			m_iEndEventOrder == 0)
	{
		m_AnimUI_Side->Change_Animation(L"Quest_Side_FadeOut");

		m_iEndEventOrder++;
	}
	else if(m_fEndTimer >= arrKeyTimes[1] &&			m_iEndEventOrder == 1)
	{
		m_AnimUI_BG->Change_Animation(L"Quest_BG_FadeIn");

		m_iEndEventOrder++;
	}
	else if(m_fEndTimer >= arrKeyTimes[2] &&			m_iEndEventOrder == 2)
	{
		m_AnimUI_Comp->Change_Animation(L"Quest_Comp_FadeIn");
		m_pGameInstance->Play_Sound(L"UI_QuestCompleted", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.35f);


		m_iEndEventOrder++;
	}
	else if(m_fEndTimer >= arrKeyTimes[3] &&			m_iEndEventOrder == 3)
	{
		m_AnimUI_BG->Change_Animation(L"Quest_BG_FadeOut");
		m_AnimUI_Comp->Change_Animation(L"Quest_Comp_FadeOut");

		m_iEndEventOrder++;
	}

	_bool condition = m_fEndTimer >= arrKeyTimes.back();
	if (condition)		m_isOnEvent_GoinEnd = false;
	m_fEndTimer += fTimeDelta;
}

HRESULT CUI_QuestIndicator::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_QuestIndicator::PreAssign_ChildUIs()
{
	m_pRUI_All	= Find_ChildObject(L"Sub_All");			

	m_UI_BG		= Find_ChildObject(L"SectorA_Quest_BG");			
	m_UI_Noti	= Find_ChildObject(L"SectorA_QuestNotification");			
	m_UI_Comp	= Find_ChildObject(L"SectorA_QuestComplete");			
	m_UI_Side	= Find_ChildObject(L"SectorA_QuestSide");			

	m_AnimUI_BG		= static_cast<CAnimator_UI*>(m_UI_BG	->Get_Component(L"Com_Animator_UI"));	
	m_AnimUI_Noti	= static_cast<CAnimator_UI*>(m_UI_Noti	->Get_Component(L"Com_Animator_UI"));
	m_AnimUI_Comp	= static_cast<CAnimator_UI*>(m_UI_Comp	->Get_Component(L"Com_Animator_UI"));
	m_AnimUI_Side	= static_cast<CAnimator_UI*>(m_UI_Side	->Get_Component(L"Com_Animator_UI"));
}

void CUI_QuestIndicator::PreAssign_Presets()
{

}

void CUI_QuestIndicator::Create_ChildText()
{
	// 생성.						..나중에 타입, 위치, 크기 설정필요!
	_float2 vTextPos;
	CUI_Text* pFont;
	CCustom_UI* pAttacher;

	// 1-1. 진입 시 알림용 (제목)
	vTextPos = { 0.f, -275.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"숨은 아이 찾기",
		TEXT_COLOR_TYPE::TT_QUESTTITLE,
		0.6f,
		L"UI_Text_NotiTitle"
	);
	pAttacher = m_UI_Noti;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_NotiTitle = pFont;

	// 1-2. 진입 시 알림용 (설명)
	vTextPos = { 0.f, -225.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"퀘스트 발생",
		TEXT_COLOR_TYPE::TT_QUESTNORMAL,
		0.4f,
		L"UI_Text_NotiDesc"
	);
	pAttacher = m_UI_Noti;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_NotiDesc = pFont;

	// 2-1. 완료 시 알림용 (제목)
	vTextPos = { 0.f, -220.f };
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"퀘스트 완료",
		TEXT_COLOR_TYPE::TT_QUESTTITLE,
		0.6f,
		L"UI_Text_CompTitle"
	);
	pAttacher = m_UI_Comp;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_CompTitle = pFont;

	// 3-1. 사이드 바 퀘스트 목록 (제목)
	vTextPos = { -862.f, -170.f };	//
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"숨은 아이 찾기",
		TEXT_COLOR_TYPE::TT_QUESTTITLE,
		0.4f,
		L"UI_Text_SideTitle"
	);
	pAttacher = m_UI_Side;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::LEFT);

	m_pTextUI_SideTitle = pFont;

	// 3-2. 사이드 바 퀘스트 목록 (설명)
	vTextPos = { -862.f, -121.f };	//
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"마을에 숨어있는 아이들 찾기",
		TEXT_COLOR_TYPE::TT_QUESTTITLE,
		0.35f,
		L"UI_Text_SideDesc"
	);
	pAttacher = m_UI_Side;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::LEFT);

	m_pTextUI_SideDesc = pFont;

	// 3-2. 사이드 바 퀘스트 목록 (상세 진행상황)
	vTextPos = { -862.f, -96.f };	//
	pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f + vTextPos.x, g_iWinSizeY / 2.f + vTextPos.y },
		L"찾은 아이 : 0 / 7",
		TEXT_COLOR_TYPE::TT_QUESTPROGRESS,
		0.35f,
		L"UI_Text_SideDesc"
	);
	pAttacher = m_UI_Side;
	pFont->Attach_AsChildToUI(pAttacher);
	pFont->Update_Alignment(TEXT_ALIGN_TYPE::LEFT);

	m_pTextUI_SideDesc2 = pFont;
}

CUI_QuestIndicator* CUI_QuestIndicator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_QuestIndicator* pInstance = new CUI_QuestIndicator(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_QuestIndicator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_QuestIndicator::Clone(void* pArg)
{
	CUI_QuestIndicator* pInstance = new CUI_QuestIndicator(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_QuestIndicator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_QuestIndicator::Free()
{
	Safe_Release(m_pGameSystem);

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_QuestIndicator");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
