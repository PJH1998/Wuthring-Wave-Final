#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_TabUtility.h"
#include "UI_Text.h"
#include "GameSystem.h"

#define	 IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만
#define	 FLOAT2_LENGTH(x)								(XMVectorGetX(XMVector2Length(XMLoadFloat2(x))))


// 생각할 것 : 탭중에는 커서락 해제되어야 함

CUI_TabUtility::CUI_TabUtility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_TabUtility::CUI_TabUtility(const CUI_TabUtility& Prototype)
	: CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_TabUtility::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_TabUtility::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	PreAssign_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_TabUtility.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	Create_ChildText_CurUtil();
	Create_ChildText_IsUsing();


	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Show.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hide.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hover_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hover_Show.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Hover_Hide.json",
	};
	Load_Animations(vecAnimFilePaths);


	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Initialize");
	static_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Initialize");

	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_TabUtility", this);

	return S_OK;
}

void CUI_TabUtility::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_TabUtility::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;
	
	Update_InitialCheck_SelectedUtility();
	Update_MouseSelection();
	Update_GoinDisable(fTimeDelta);


	__super::Update(fTimeDelta);
}

void CUI_TabUtility::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_TabUtility::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_TabUtility::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pGameInstance->Play_Sound(L"UI_TabUtility_Open", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);

	static_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Initialize", true);
	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Show", true);


	m_iCharSelectedIndex = static_cast<UI_TABUTIL_DESC*>(pArg)->iCharSelectedUtilityIndex;

	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0;
	m_isActivate = true;
	m_isFirstCheckedIndex = false;
	m_isFirstCheckedSelectedUtil = false;

	m_pGameSystem->Set_MouseFix(false);
	m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::BLUR);
	m_pGameSystem->Change_TimeRate(COLLISIONLAYER::PLAYER, 0.1f);
	m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 0.1f);
}

_uint CUI_TabUtility::Req_OffTabUI()
{
	m_IsGoinDisabled = true;
	m_pGameInstance->Play_Sound(L"UI_TabUtility_Close", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);
	m_pGameInstance->End_SFX();
	m_pGameSystem->Change_TimeRate(COLLISIONLAYER::PLAYER, 1.f);
	m_pGameSystem->Change_TimeRate(COLLISIONLAYER::ENEMY, 1.f);
	return m_iSelectedIndex;
}

void CUI_TabUtility::Update_InitialCheck_SelectedUtility()
{
	if (m_isFirstCheckedSelectedUtil)
		return;
	m_isFirstCheckedSelectedUtil = true;		// UI On 시 1회만 체크


	// 테스트용 체크는 test level 에서 하고, 
	// 여기서는 그냥 받아온 정보를 기반으로 선택 및 UI 인스턴스 거르기만 하도록

	// 최초 정보는 그냥 desc로 받아오도록?


	// selected indicator
	auto& targetDesc = m_pUI_InstSelected->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;

	for (_uint i = 0; i < targetInstDesc.size(); i++)		// 현재 인덱스에 해당하는 인스턴스만 보이게 하고, 나머지는 가림.
	{
		targetInstDesc[i].vClipTexcoordX = (m_iCharSelectedIndex == i) ?
			_float2{ 0.f, 1.f } :
			_float2{ 0.f, 0.f };
	}

	//m_pUI_InstSelected->Set_UIDesc(targetDesc);


	// selected icon on center
	auto& targetIconDesc = m_pUI_CHSelectedIcon->Get_UIDesc();
	auto& targetIconInstDesc = targetIconDesc.vecInstanceDescs;

	targetIconInstDesc[0].vSInstCoordX = m_arrCoordPresets[m_iCharSelectedIndex][0]; // 현재 인덱스에 해당하는 coord로 변경.
	targetIconInstDesc[0].vSInstCoordY = m_arrCoordPresets[m_iCharSelectedIndex][1];

	//m_pUI_CHSelectedIcon->Set_UIDesc(targetDesc);


	// Text on Center
	CUI_Text* pTargetText = dynamic_cast<CUI_Text*>(m_pTextUI_Selected);

	_wstring strSelectedUtilityName = {};
	switch (m_iCharSelectedIndex)
	{
	case ENUM_CLASS(Client::UI_TAB_UTILITY::GRAPPLE):			strSelectedUtilityName = L"로프";		break;
	case ENUM_CLASS(Client::UI_TAB_UTILITY::SENSOR):			strSelectedUtilityName = L"스캔";		break;
	case ENUM_CLASS(Client::UI_TAB_UTILITY::FLIGHT):			strSelectedUtilityName = L"활공";		break;
	case ENUM_CLASS(Client::UI_TAB_UTILITY::LEVITATOR):			strSelectedUtilityName = L"컨트롤";		break;
	case ENUM_CLASS(Client::UI_TAB_UTILITY::NOTHING):			strSelectedUtilityName = L"미선택";		break;
	}

	pTargetText->Change_Text(strSelectedUtilityName, TEXT_ALIGN_TYPE::CENTER);



	//std::cout << "[CUI_TabUtility::Update_InitialCheck_SelectedUtility] ScPos X : " << pTargetText->Get_TextUIDesc().vScreenPos.x << "\t, Y : " << pTargetText->Get_TextUIDesc().vScreenPos.x << std::endl;
}

void CUI_TabUtility::Update_MouseSelection()
{
	if (m_IsGoinDisabled)
		return;


	POINT PointMousePos = m_pGameInstance->Get_MousePoint();
	_float2 vMousePos = {							// Center Aligned.
		PointMousePos.x - g_iWinSizeX * 0.5f,
		PointMousePos.y - g_iWinSizeY * 0.5f
	};


	
	// 1. 커서 위치에 따라 Arrow 도 같이 돌아가야 함. 이는 중앙 대비 커서 위치에 따라 단순히 m_pUI_Arrow 를 돌리기만 하면 될 것.
	// 2. 커서가 중앙으로부터 일정 거리 벗어나면, SelectedIndex 가 갱신되어야 함.
	// 3. SelectedIndex 가 갱신될 떄에, Hover 또한 애니메이션이 재생되어야 하며, 해당 인덱스에 맞는 인스턴스만이 보여야 함
	
	//	 근데 다시 중앙에 돌아오면, SelectedIndex 는 다시 기본값으로 돌아가며, 이 상태로 끄면 선택한 장비는 현행유지여야 함.
	

	// [1] 커서 위치에 따른 Arrow 회전
	// Z 값이 0이면 12시. 90이면 9시와 같이 반시계방향으로 돌아감.
	_float fArrowAngle = atan2f(vMousePos.x, vMousePos.y) + DegreesToRadians(180.f);	// counterclockwise, rad
	m_pTransformCom_UIArrow->Rotation(XMVectorSet(0.f, 0.f, 1.f, 1.f), fArrowAngle);


	// [2] 커서 방향지정에 따른 선택 Util 갱신
	_float fAngleDeg = RadiansToDegrees(fArrowAngle);
	if		(fAngleDeg > 360.f)			fAngleDeg -= 360.f;
	else if (fAngleDeg <= 0.f)			fAngleDeg += 360.f;

	// 만약 중앙에서 일정 벗어나있다면 인덱스 지정
	_uint iPrevSelectedIndex = m_iSelectedIndex;
	_bool isIndexChanged = false;
	_float fDistFromCenter = FLOAT2_LENGTH(&vMousePos);
	if (fDistFromCenter >= 200.f)
	{
		if		(IS_BETWEEN(fAngleDeg, 0.f, 45.f) ||
				 IS_BETWEEN(fAngleDeg, 315.f, 360.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::FLIGHT);		// 2
		else if (IS_BETWEEN(fAngleDeg, 45.f, 135.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::SENSOR);		// 1
		else if (IS_BETWEEN(fAngleDeg, 135.f, 225.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::GRAPPLE);		// 0
		else if (IS_BETWEEN(fAngleDeg, 225.f, 315.f))	m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::LEVITATOR);	// 3
	}
	else
		m_iSelectedIndex = ENUM_CLASS(UI_TAB_UTILITY::NOTHING);

	if (iPrevSelectedIndex != m_iSelectedIndex)
		isIndexChanged = true;

	// [3] Util 갱신에 따른 애니메이션 재생 및 인스턴스 선택
	if ((!m_isFirstCheckedIndex && (fDistFromCenter >= 200.f)) ||							// 최초 1회 커서위치에 따른 확인
		isIndexChanged && !(m_iSelectedIndex == ENUM_CLASS(UI_TAB_UTILITY::NOTHING)))		// 다른 무언가로 선택이 바뀜.
	{
		dynamic_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Show", true);
		
		auto& targetDesc = m_pUI_InstHover->Get_UIDesc();
		auto& targetInstDesc = targetDesc.vecInstanceDescs;

		for (_uint i = 0; i < targetInstDesc.size(); i++)		// 현재 인덱스에 해당하는 인스턴스만 보이게 하고, 나머지는 가림.
		{
			targetInstDesc[i].vClipTexcoordX = (m_iSelectedIndex == i)? 
				_float2{ 0.f, 1.f } : 
				_float2{ 0.f, 0.f };
		}

		//m_pUI_InstHover->Set_UIDesc(targetDesc);
	}
	else if (isIndexChanged && (m_iSelectedIndex == ENUM_CLASS(UI_TAB_UTILITY::NOTHING)))	// 선택 해제함 (커서가 화면 중앙으로 감)
		dynamic_cast<CAnimator_UI*>(m_pUI_Hover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hover_Hide");





	// [+] 현재 사용중인 유틸 선택 시 사용 중 텍스트 출력 및 중앙 아이콘 변화

	if (m_isFirstCheckedIndex ||
		isIndexChanged)
	{
		// 유틸명 텍스트 변화
		_wstring strSelectedUtilityName = {};
		switch (m_iSelectedIndex)
		{
		case ENUM_CLASS(Client::UI_TAB_UTILITY::GRAPPLE):			strSelectedUtilityName = L"로프";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::SENSOR):			strSelectedUtilityName = L"스캔";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::FLIGHT):			strSelectedUtilityName = L"활공";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::LEVITATOR):			strSelectedUtilityName = L"컨트롤";		break;
		case ENUM_CLASS(Client::UI_TAB_UTILITY::NOTHING):			strSelectedUtilityName = L"미선택";		break;
		}

		static_cast<CUI_Text*>(m_pTextUI_Selected)->Change_Text(strSelectedUtilityName, TEXT_ALIGN_TYPE::CENTER);



		// 아이콘 변화
		auto& iconDesc = m_pUI_CHSelectedIcon->Get_UIDesc();
		auto& iconInstDesc = iconDesc.vecInstanceDescs;

		iconInstDesc[0].vSInstCoordX = m_arrCoordPresets[m_iSelectedIndex][0]; // 현재 인덱스에 해당하는 coord로 변경.
		iconInstDesc[0].vSInstCoordY = m_arrCoordPresets[m_iSelectedIndex][1];

		m_pUI_CHSelectedIcon->Set_UIDesc(iconDesc);



		// 사용 중 텍스트 변화
		_wstring strIsUsingText = {};

		if ((m_iCharSelectedIndex == m_iSelectedIndex) &&
			(m_iSelectedIndex != ENUM_CLASS(UI_TAB_UTILITY::NOTHING)))
			m_pTextUI_IsUsing->SetActivate(true);
		else
			m_pTextUI_IsUsing->SetActivate(false);
	}
		

	// [Sound] 선택 중인 게 변경될 시 사운드 재생. 단, nothing으로 바뀌는 경우 제외 / 끌 때에 끄는 사운드, 켤 때에 켜는 사운드
	if (isIndexChanged &&
		m_iSelectedIndex != ENUM_CLASS(UI_TAB_UTILITY::NOTHING))
		m_pGameInstance->Play_Sound(L"UI_TabUtility_Tick", ENUM_CLASS(CHANNEL::UI_HOVER), 0.5f);

	




	m_isFirstCheckedIndex = true;
}

void CUI_TabUtility::Update_GoinDisable(_float fTimeDelta)
{
	if (!m_IsGoinDisabled)
		return;


	const _float fMaxDisableTimer = 0.5f;
	
	// 시간 충분히 경과시 inactive.
	m_fDisableTimer += fTimeDelta;
	if (m_fDisableTimer >= fMaxDisableTimer)
	{
		m_isActivate = false;
		m_iAnimOrder = 0;
		m_fDisableTimer = 0.f;

		return;
	}

	m_pGameSystem->Set_MouseFix(true);

	// 시간 경과 중 애니메이션 재생 (disable 시작 시 1회)
	if (m_iAnimOrder == 0)
	{
		static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Hide");
		m_iAnimOrder++;
		//m_isFirstCheckedSelectedUtil = false;
	}
}

HRESULT CUI_TabUtility::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_TabUtility::PreAssign_ChildUIs()
{
	m_pRUI_All			= Find_ChildObject(L"Sub_All");
	m_pUI_Back			= Find_ChildObject(L"SectorA_Back");
	m_pUI_Hover			= Find_ChildObject(L"SectorA_Hover");
	m_pUI_Arrow			= Find_ChildObject(L"SectorA_Arrow");
	m_pUI_GuideCircle	= Find_ChildObject(L"SectorA_GuideCircle");

	m_pUI_InstHover		= Find_ChildObject(L"Switch_Hover");
	m_pUI_InstSelected	= Find_ChildObject(L"Switch_Select");

	m_pUI_CHSelectedIcon= Find_ChildObject(L"Icon_CurSkill");


	m_pTransformCom_UIArrow = dynamic_cast<CTransform*>(m_pUI_Arrow->Get_Component(L"Com_Transform"));
}

void CUI_TabUtility::PreAssign_Presets()
{
	m_arrCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::GRAPPLE)]	= {_float2(0.00f, 0.25f), _float2(0.50f, 0.75f)}; 
	m_arrCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::SENSOR)]	= {_float2(0.75f, 1.00f), _float2(0.25f, 0.50f)}; 
	m_arrCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::FLIGHT)]	= {_float2(0.25f, 0.50f), _float2(0.75f, 1.00f)}; 
	m_arrCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::LEVITATOR)]= {_float2(0.25f, 0.50f), _float2(0.50f, 0.75f)}; 
	m_arrCoordPresets[ENUM_CLASS(UI_TAB_UTILITY::NOTHING)]	= {_float2(0.75f, 1.00f), _float2(0.75f, 1.00f)};
}
											
void CUI_TabUtility::Create_ChildText_CurUtil()
{
	//
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f/* + 6.f*/, g_iWinSizeY / 2.f + 50.f },
		L"",	// 상호작용 글씨
		TEXT_COLOR_TYPE::TT_TABUTIL,
		0.5f,
		L"UI_Text_TabUtility"
	);

	CCustom_UI* pAttacher = m_pUI_GuideCircle;
	auto& fontDesc = pFont->Get_UIDesc();
	auto& attacherDesc = pAttacher->Get_UIDesc(); // 사본 가져오기

	attacherDesc.vecChildNames.push_back(fontDesc.strUIName);
	pAttacher->Add_Child(pFont);

	for (auto& inst : fontDesc.vecInstanceDescs)
		inst.matExtraData._11 = 1.f;

	fontDesc.strParentName = pAttacher->Get_UIDesc().strUIName;
	fontDesc.pParentObject = pAttacher;

	//pFont->Set_UIDesc(fontDesc);
	pFont->Update_Description(0.f);

	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_Selected = pFont;
}

void CUI_TabUtility::Create_ChildText_IsUsing()
{
	//
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f/* + 6.f*/, g_iWinSizeY / 2.f - 120.f },
		L"사용 중",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.4f,
		L"UI_Text_TabUtilityUsing"
	);

	CCustom_UI* pAttacher = m_pUI_GuideCircle;
	auto fontDesc = pFont->Get_UIDesc();
	auto attacherDesc = pAttacher->Get_UIDesc(); // 사본 가져오기

	attacherDesc.vecChildNames.push_back(fontDesc.strUIName);
	pAttacher->Add_Child(pFont);

	for (auto& inst : fontDesc.vecInstanceDescs)
		inst.matExtraData._11 = 1.f;

	fontDesc.strParentName = pAttacher->Get_UIDesc().strUIName;
	fontDesc.pParentObject = pAttacher;

	//pFont->Set_UIDesc(fontDesc);
	pFont->Update_Description(0.f);

	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);

	m_pTextUI_IsUsing = pFont;
}


CUI_TabUtility* CUI_TabUtility::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_TabUtility* pInstance = new CUI_TabUtility(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_TabUtility");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_TabUtility::Clone(void* pArg)
{
	CUI_TabUtility* pInstance = new CUI_TabUtility(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_TabUtility");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_TabUtility::Free()
{
	Safe_Release(m_pGameSystem);

	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_TabUtility");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
