
#include "ClientPch.h"
#include "UI_Button_Interact.h"

#include "Animator_UI.h"
#include "GameSystem.h"
#include "UI_Text.h"

CUI_Button_Interact::CUI_Button_Interact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Button(pDevice, pContext)
{
}

CUI_Button_Interact::CUI_Button_Interact(const CUI_Button_Interact& Prototype)
	: CUI_Button(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}


HRESULT CUI_Button_Interact::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Button_Interact::Initialize_Clone(void* pArg)
{
	//__super::Initialize_Clone(pArg);

	CGameObject::Initialize_Clone(pArg);
#ifdef KSTA_ON_TRANSFORM_CACHING
	m_vecCachedUITransform.resize(1);
#endif // KSTA_ON_TRANSFORM_CACHING

	Ready_Components(pArg);
	__super::Ready_Events();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Interact.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_FadeIn.json",			// 생김		[Root_Interact]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_FadeOut.json",			// 사라짐	[Root_Interact]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_Focused_On.json",		// 호버 On	[Interact_Focused]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_Focused_Off.json",		// 호버 Off	[Interact_Focused]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_Pressed_Trigger.json",	// 클릭		[Interact_Pressed]

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_Focused_Default.json",	// 호버 def	[Interact_Focused]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Interact_Pressed_Default.json"	// 클릭 def [Interact_Pressed]
	};
	Load_Animations(vecAnimFilePaths);

	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	Create_ChildText();
	m_pGameInstance->Add_RootUI(L"UI_Interact", this);

	m_isClone = true;

	return S_OK;
}

void CUI_Button_Interact::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Button_Interact::Update(_float fTimeDelta)
{
	// 1. 이벤트 판별에 따른 애니메이션 전환
	// - animation change via mouse/etc event
	// 2. Update!
	// 3. Update_CombinedMatrix
	// 4. Update_CombinedDesc

	Update_MouseFeedback(fTimeDelta);

	__super::Update(fTimeDelta);            // Update Animator_UI Component
}


void CUI_Button_Interact::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}


void CUI_Button_Interact::Render()
{
	if (!m_isActivate)
		return;

	__super::Render();
}

void CUI_Button_Interact::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	CCustom_UI* pRootUI = m_pRUI_Interact_Multiplier;

	//pRootUI->SetActivate(false);
	//auto combinedKFDesc_Root = static_cast<CAnimator_UI*>(pRootUI->Get_Component(L"Com_Animator_UI"))->Get_CurCombinedAnimKeyframeDesc();
	//combinedKFDesc_Root->fAlpha = 1.f;
	//static_cast<CAnimator_UI*>(pRootUI->Get_Component(L"Com_Animator_UI"))->Set_CurCombinedAnimKeyframeDesc(*combinedKFDesc_Root);


	CCustom_UI* pFocusedUI = m_pUI_Interact_Focused;
	pFocusedUI->SetActivate(false);
	//auto combinedKFDesc_Focused = static_cast<CAnimator_UI*>(pFocusedUI->Get_Component(L"Com_Animator_UI"))->Get_CurCombinedAnimKeyframeDesc();
	//combinedKFDesc_Focused->fAlpha = 1.f;
	//static_cast<CAnimator_UI*>(pFocusedUI->Get_Component(L"Com_Animator_UI"))->Set_CurCombinedAnimKeyframeDesc(*combinedKFDesc_Focused);



	CCustom_UI* pPressedUI = m_pUI_Interact_Pressed;
	pPressedUI->SetActivate(false);

	//auto combinedKFDesc_Pressed = static_cast<CAnimator_UI*>(pPressedUI->Get_Component(L"Com_Animator_UI"))->Get_CurCombinedAnimKeyframeDesc();
	//combinedKFDesc_Pressed->fAlpha = 1.f;
	//static_cast<CAnimator_UI*>(pPressedUI->Get_Component(L"Com_Animator_UI"))->Set_CurCombinedAnimKeyframeDesc(*combinedKFDesc_Pressed);


	static_cast<CAnimator_UI*>(pRootUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_FadeIn");

	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0;

	m_isActivate = true;

	//if ( FAILED (static_cast<CAnimator_UI*>(pFocusedUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_Focused_Default")))
	//	CRASH("");
	//if ( FAILED (static_cast<CAnimator_UI*>(pPressedUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_Pressed_Default")))
	//	CRASH("");
}

void CUI_Button_Interact::PreAssign_ChildUIs()
{
	m_pRUI_Interact_Multiplier	= Find_ChildObject(L"Root_Interact_Multiplier");
	m_pUI_Interact_Focused		= Find_ChildObject(L"Interact_Focused");
	m_pUI_Interact_Pressed		= Find_ChildObject(L"Interact_Pressed");
	m_pUI_Interact_Normal		= Find_ChildObject(L"Interact_Normal");
}

void CUI_Button_Interact::Update_MouseFeedback(_float fTimeDelta)
{
	CCustom_UI* pRootUI = m_pRUI_Interact_Multiplier;

	CCustom_UI* pFocusedUI	= m_pUI_Interact_Focused;
	CCustom_UI* pPressedUI = m_pUI_Interact_Pressed;

	CCustom_UI* pEventTargetUI = m_pUI_Interact_Normal;		// 이벤트 판별용으로 쓸 UI.


	_bool isHoverEnter	= pEventTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER));
	_bool isHoverExit	= pEventTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVER_EXIT));
	_bool isClickEnter	= pEventTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER));

	if (isClickEnter && !m_IsGoinDisabled)
	{
		pPressedUI->SetActivate(true);
		static_cast<CAnimator_UI*>(pPressedUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_Pressed_Trigger");
		m_IsGoinDisabled = true;
		cout << "[UI_Button_Interact::Update_MouseFeedback] || Click Enter" << endl;
	}
	else if (!m_IsGoinDisabled)
	{
		if (isHoverEnter)
		{
			pFocusedUI->SetActivate(true);
			static_cast<CAnimator_UI*>(pFocusedUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_Focused_On");
			cout << "[UI_Button_Interact::Update_MouseFeedback] >> Hover Enter" << endl;
		}
		else if (isHoverExit)
		{
			static_cast<CAnimator_UI*>(pFocusedUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_Focused_Off");
			cout << "[UI_Button_Interact::Update_MouseFeedback] << Hover Exit" << endl;
		}
	}

	

	if (m_IsGoinDisabled)
		m_fDisableTimer += fTimeDelta;;

	const _float fDisableTime = 0.3f;
	if (m_fDisableTimer >= fDisableTime &&
		m_iAnimOrder == 0)
	{
		static_cast<CAnimator_UI*>(pRootUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Interact_FadeOut");
		m_iAnimOrder = 1;
	}
	else if (m_fDisableTimer >= fDisableTime + 0.5f &&
		m_iAnimOrder == 1)
	{
		m_isActivate = false;
		m_iAnimOrder = 2;
	}
	 
}

void CUI_Button_Interact::Create_ChildText()
{
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f - 120.f, g_iWinSizeY / 2.f - 7.f },
		L"",	// 상호작용 글씨
		TEXT_COLOR_TYPE::TT_NORMAL,
		0.33f,
		L"UI_Text_Interact"
	);

	CCustom_UI* pAttacher = m_pRUI_Interact_Multiplier;
	auto& fontDesc = pFont->Get_UIDesc();
	auto& attacherDesc = pAttacher->Get_UIDesc(); // 사본 가져오기

	attacherDesc.vecChildNames.push_back(fontDesc.strUIName);
	//pAttacher->Set_UIDesc(attacherDesc); // 변경된 Desc 설정 (필요한 경우)
	pAttacher->Add_Child(pFont);

	for (auto& inst : fontDesc.vecInstanceDescs)
		inst.matExtraData._11 = 1.f;

	fontDesc.strParentName = pAttacher->Get_UIDesc().strUIName;
	fontDesc.pParentObject = pAttacher;

	//pFont->Set_UIDesc(fontDesc);
	pFont->Update_Description(0.f);
}

CUI_Button_Interact* CUI_Button_Interact::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Button_Interact* pInstance = new CUI_Button_Interact(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Button_Interact");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Button_Interact::Clone(void* pArg)
{
	CUI_Button_Interact* pInstance = new CUI_Button_Interact(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Button_Interact");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Button_Interact::Free()
{
	Safe_Release(m_pGameSystem);
	if (m_isClone)
	    m_pGameInstance->Remove_RootUI(L"UI_Interact");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
