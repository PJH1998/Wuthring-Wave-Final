#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_Ovfl_Palette.h"
#include "UI_Text.h"
#include "GameSystem.h"

#include "Event_Level.h"

//#define	KSTA_UITEST_RANDOM_GENERATE

#define	 IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만
#define	 FLOAT2_LENGTH(x)								(XMVectorGetX(XMVector2Length(XMLoadFloat2(x))))
#define	 FLOAT2_LENGTH_NOLOAD(x)						(XMVectorGetX(XMVector2Length(x)))


CUI_Ovfl_Palette::CUI_Ovfl_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Ovfl_Palette::CUI_Ovfl_Palette(const CUI_Ovfl_Palette& Prototype)
	: CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
}

HRESULT CUI_Ovfl_Palette::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Ovfl_Palette::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	PreAssign_Presets();

	
	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Palette.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();
	Ready_ChildExtraComponents();
	
	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		// UI Anims..
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Show.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Hide.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Color_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Color_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Color_FadeOut.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Background_Initialize.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_ResetHover_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_ResetHover_FadeIn.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_ResetHover_FadeOut.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_ResetHover_Click.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_ResetHover_TickLoop.json",
	};
	Load_Animations(vecAnimFilePaths);
	Create_ChildText_InfoText();
	Create_ChildText_LeftChance();

	Create_ChildText_Description();
	Create_ChildText_DestColor();



	// Load Levels.
	Load_LevelData(0);


	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Initialize");
	static_cast<CAnimator_UI*>(m_pUI_Background->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Background_Initialize");
	static_cast<CAnimator_UI*>(m_pUI_ResetHover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_ResetHover_Initialize");


	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Ovfl_Palette", this);
	m_isActivate = false;
	
	return S_OK;
}

void CUI_Ovfl_Palette::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_Ovfl_Palette::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	// 종료 조건시에도 작동해야 하는 것들
	Update_ResetBtn();
	Update_GoinDisable(fTimeDelta);
	
	if ((m_isGoinSuccess || m_isGoinFail) &&
		(!m_isGoinChange))			// 종료 이벤트 체크 후, 종료 조건 시 진행 막음
	{
		Update_FinishEvent();
		__super::Update(fTimeDelta);
		return;
	}

	// 종료 조건이 아닐 때만 작동해야 하는 것들
	Update_ChangeColorBtn();
	Update_HoverEvent();


	// is KeyDown
	Trigger_ClickEvent();
	Update_ChangeEvent(fTimeDelta);


#ifdef _DEBUG

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
		Trigger_ResetLevel(0);

#endif // _DEBUG


	Update_PalettesInstance();

	__super::Update(fTimeDelta);
}

void CUI_Ovfl_Palette::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_Ovfl_Palette::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_Ovfl_Palette::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_pGameInstance->Play_Sound(L"UI_OVFL_Open", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);

	UI_OVFLPALETTE_DESC* pDesc = static_cast<UI_OVFLPALETTE_DESC*>(pArg);
	_uint iTargetLevel = pDesc->iTargetLevel;

	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Show", true);

	Trigger_ResetLevel(iTargetLevel);
	m_isActivate = true;
	m_IsGoinDisabled = false;
	m_isFinishedEvent = false;
	m_pGameSystem->Set_MouseFix(false);
}

HRESULT CUI_Ovfl_Palette::Ready_Events()
{
	m_pGameInstance->Subscribe<MINIGAMEPALETTE_SUCCESS_UI_EVENT>(ENUM_CLASS(STATIC::NONE), TEXT("Event_Minigame_Palette_Success"), [this](const MINIGAMEPALETTE_SUCCESS_UI_EVENT event) {
		if (event.isSuccess)
		{
			// 성공 시 시행할 것은 여기에..

			// 이걸 여기서?
		}
	});

	return S_OK;
}

HRESULT CUI_Ovfl_Palette::Ready_Components(void* pArg)
{
	return S_OK;
}

HRESULT CUI_Ovfl_Palette::Ready_ChildExtraComponents()
{
	// 자식들에게 효과용 추가 텍스쳐를 바인딩한다.
	// 사전에 로더에서 프로토타입 생성 필요. 이는 텍스쳐 선 로드용 짬통 json을 사용함.
	
	_wstring strExtraTexName			= L"T_BgTextureGreen";

	if (FAILED(m_pUI_InstBlocks			->Add_ExtraTexture(strExtraTexName)))			return E_FAIL;
	if (FAILED(m_pUI_InstColorBtns		->Add_ExtraTexture(strExtraTexName)))			return E_FAIL;


	_wstring strExtraTexName_Noise12	= L"T_Noise_No.png (12)";
	_wstring strExtraTexName_Noise14	= L"T_Noise_No.png (14)";
	_wstring strExtraTexName_Caustic	= L"T_Caustic_Noise";

	if (FAILED(m_pUI_InstBlocks			->Add_ExtraTexture(strExtraTexName_Noise12)))	return E_FAIL;
	if (FAILED(m_pUI_InstBlocks			->Add_ExtraTexture(strExtraTexName_Noise14)))	return E_FAIL;
	if (FAILED(m_pUI_InstBlocks			->Add_ExtraTexture(strExtraTexName_Caustic)))	return E_FAIL;


	return S_OK;
}

void CUI_Ovfl_Palette::PreAssign_Presets()
{
	// Pre-Assign Color Presets
	m_arrColors[PCOLOR_RED]		= _float4( 0.667f, 0.278f, 0.286f, 1.000f );
	m_arrColors[PCOLOR_GREEN]	= _float4( 0.400f, 0.675f, 0.596f, 1.000f );
	m_arrColors[PCOLOR_BLUE]	= _float4( 0.478f, 0.639f, 0.835f, 1.000f );
	m_arrColors[PCOLOR_YELLOW]	= _float4( 0.882f, 0.804f, 0.569f, 1.000f );
	m_arrColors[PCOLOR_END]		= _float4( 1.000f, 0.000f, 1.000f, 1.000f );		// Magenta.

	//_float4 col0 = _float4(0.667f, 0.278f, 0.286f, 1.000f); // aa4749
	//_float4 col1 = _float4(0.400f, 0.675f, 0.596f, 1.000f); // 66ac98
	//_float4 col2 = _float4(0.478f, 0.639f, 0.835f, 1.000f); // 7aa3d5
	//_float4 col3 = _float4(0.882f, 0.804f, 0.569f, 1.000f); // e1cd91


	
	// Pre-Assign Palette Presets
	for (_uint i = 0; i < m_arrPalettesInfo.size(); i++)
		for (_uint j = 0; j < m_arrPalettesInfo[i].size(); j++)
		{
			m_arrPalettesInfo[i][j].arrIndex[0] = i;
			m_arrPalettesInfo[i][j].arrIndex[1] = j;
		}
}

void CUI_Ovfl_Palette::PreAssign_ChildUIs()
{
	m_pRUI_All				= Find_ChildObject(L"Sub_All");

	m_pUIBackgrounds		= Find_ChildObject(L"SectorA_Backgrounds");
	m_pUIForegrounds		= Find_ChildObject(L"SectorA_Foregrounds");
	m_pUISideThings			= Find_ChildObject(L"SectorA_SideThings");
	m_pUIOthers				= Find_ChildObject(L"SectorA_Others");

	m_pUI_Background		= Find_ChildObject(L"Palette_Background");

	m_pUI_BGFrame			= Find_ChildObject(L"FG_Frame");
	m_pUI_InstBlocks		= Find_ChildObject(L"FG_InstBlocks");
	m_pUI_InstHoverBlocks	= Find_ChildObject(L"FG_InstHoverBlocks");
	m_pUI_InstColorBtns		= Find_ChildObject(L"Side_ColorButton");
	m_pUI_InstSelectedRing	= Find_ChildObject(L"Side_SelectedRing");
	m_pUI_InstHoveredRing	= Find_ChildObject(L"Side_HoveredRing");
	m_pUI_InstResetBtn		= Find_ChildObject(L"Side_Reset");

	m_pUI_ResetHover		= Find_ChildObject(L"Side_ResetHover");
}

void CUI_Ovfl_Palette::Create_ChildText_InfoText()
{
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f - 800.f, g_iWinSizeY / 2.f - 400.f },
		L"남은 횟수",
		TEXT_COLOR_TYPE::TT_NORMAL,
		0.33f,
		L"UI_Text_Palette_Info"
	);

	CCustom_UI* pAttacher = m_pRUI_All;
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

	m_pTextUI_InfoText = pFont;
}

void CUI_Ovfl_Palette::Create_ChildText_LeftChance()
{
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f - 710.f, g_iWinSizeY / 2.f - 405.f },
		L"153",
		TEXT_COLOR_TYPE::TT_TITLE,
		0.5f,
		L"UI_Text_Palette_LeftChance"
	);

	CCustom_UI* pAttacher = m_pRUI_All;
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

	m_pTextUI_LeftChance = pFont;
}

void CUI_Ovfl_Palette::Create_ChildText_Description()
{
	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f - 550.f, g_iWinSizeY / 2.f + 450.f },
		L"모든 색상 블록을              으로 염색하세요.",
		TEXT_COLOR_TYPE::TT_NORMAL,
		0.33f,
		L"UI_Text_Palette_Description"
	);

	CCustom_UI* pAttacher = m_pRUI_All;
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

	m_pTextUI_Description = pFont;
}

void CUI_Ovfl_Palette::Create_ChildText_DestColor()
{
	array<_wstring, 5> arrText = { L"빨간색",  L"초록색" , L"파란색" , L"노란색", L"Nothing"};

	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f - 400.f, g_iWinSizeY / 2.f + 450.f },
		arrText[m_eDestColorIndex],	// 상호작용 글씨
		TEXT_COLOR_TYPE::TT_NORMAL,
		0.33f,
		L"UI_Text_Palette_DestColor"
	);

	CCustom_UI* pAttacher = m_pRUI_All;
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

	m_pTextUI_DestColor = pFont;


	auto& targetTestDesc = pFont->Get_TextUIDesc();
	targetTestDesc.vColor = m_arrColors[m_eGoalColorIndex];
}

void CUI_Ovfl_Palette::Trigger_ResetLevel(_uint iLevelIndex)
{
	m_isGoinChange		= false;
	m_isGoinOpen		= false;
	m_isGoinSuccess		= false;
	m_isGoinFail		= false;
	m_isFinishedEvent	= false;
	m_fChangeRadius		= 0.f;

	Load_LevelData(iLevelIndex);
}

void CUI_Ovfl_Palette::Trigger_ClickEvent()
{
	_uint iClickedIndex = {};
	if (!m_isGoinChange &&
		Check_ClickedBlockInstance(&iClickedIndex))	// if Block Clicked !
	{

		_uint iIndexX = iClickedIndex / m_iPaletteSizeX;
		_uint iIndexY = iClickedIndex % m_iPaletteSizeX;

		UI_PALETTE_DESC& targetDesc = m_arrPalettesInfo[iIndexX][iIndexY];

		//m_eDestColorIndex = m_eDestColorIndex;

		if (targetDesc.eColor == m_eDestColorIndex)
			return;
		
		m_isGoinChange = true;

		m_vChangeStartPos = Calc_InstBlock_ScrnPos(iClickedIndex);
		m_isGoinChange;
		m_fChangeRadius;

		Assign_TargetBlocksQueue(iClickedIndex);

		m_iLeftChance--;
		static_cast<CUI_Text*>(m_pTextUI_LeftChance)->Change_Text(to_wstring(m_iLeftChance));


		// [SOUND] 최초 클릭 시 
	}


	// 남은 횟수 소진 시, 현재 결과를 확인해보고 성공 여부를 체크
	if (!m_isGoinChange && m_iLeftChance == 0)
	{
		// 결과 체크..
		_bool isCheckedFail = false;

		for (auto& palettes : m_arrPalettesInfo)
			for (auto& palette : palettes)
			{
				if (palette.eColor != m_eGoalColorIndex)
				{
					isCheckedFail = true;
					break;
				}

				if (isCheckedFail) break;
			}

		// 모든 블럭이 타겟 색상이면 success, 아니면 fail
		if (isCheckedFail)		m_isGoinFail = true;
		else					m_isGoinSuccess = true;
	}
}

void CUI_Ovfl_Palette::Update_HoverEvent()
{
	if (m_isGoinChange)
		return;

	CCustom_UI* pTargetUI = m_pUI_InstHoverBlocks;
	CCustom_UI* pHoverCheckTargetUI = m_pUI_InstBlocks;

	// 마우스가 올라간 인스턴스를 탐색
	_uint iNumInstHovers = static_cast<_uint>(pTargetUI->Get_UIDesc().vecInstanceDescs.size());

	_bool isHovered = false;
	m_iHoveredIndex = UINT_MAX;

	for (_uint i = 0; i < iNumInstHovers; i++)
	{
		isHovered = pHoverCheckTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVERING), i);
		if (isHovered)
		{
			m_iHoveredIndex = i;
			break;
		}
	}

	// 적용
	auto& targetDesc = pTargetUI->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;

	for (_uint i = 0; i < targetInstDesc.size(); i++)
	{
		auto& instDesc = targetInstDesc[i];

		instDesc.vClipTexcoordX = (m_iHoveredIndex == i) ? _float2{ 0.f, 1.f } : _float2{ 0.f, 0.f }; // 마우스를 올린 게 있으면 {0.f, 1.f} 가 들어가야 함
	}

	//pTargetUI->Set_UIDesc(targetDesc);

	_bool isChanged_HoveredIndex = (m_iHoveredIndex == UINT_MAX)? false : m_iPrevHoveredIndex != m_iHoveredIndex;
	if (isChanged_HoveredIndex)
		m_pGameInstance->Play_Sound(L"UI_OVFL_MouseOver", ENUM_CLASS(CHANNEL::UI_HOVER), 0.5f);
	m_iPrevHoveredIndex = m_iHoveredIndex;
}

HRESULT CUI_Ovfl_Palette::Load_LevelData(_uint iLevelIndex)
{
	// 10 * 8 데이터 csv로 불러온 뒤, 남은 색상 + 목표 색상 으로 불러옴.
	const vector<_string> vecFilePath = { // test
		"../../Client/Bin/Resource/UI/Fcsv/UILevel_OvflPalette/level_Beohr_01_4.csv",
		"../../Client/Bin/Resource/UI/Fcsv/UILevel_OvflPalette/level_Avinoleum_04_3.csv",
		"../../Client/Bin/Resource/UI/Fcsv/UILevel_OvflPalette/level_Avinoleum_01_4.csv",
		"../../Client/Bin/Resource/UI/Fcsv/UILevel_OvflPalette/level_Beohr_03_3.csv",
		"../../Client/Bin/Resource/UI/Fcsv/UILevel_OvflPalette/level_Beohr_02_8.csv",
	};

	array<_wstring, 5> arrText = { L"빨간색",  L"초록색" , L"파란색" , L"노란색", L"Nothing" };

	_uint iTargetLevel = UINT_MAX;
	_bool isNotExistLevel = false;
	if (iLevelIndex >= static_cast<_uint>(vecFilePath.size()))
		isNotExistLevel = true;

	vector<vector<_string>> vecLoadDatas = {};
	_bool isLoaded = false;

	CCustom_UI* pTargetFrameUI = m_pUI_BGFrame;
	auto& frameDesc = pTargetFrameUI->Get_UIDesc();
	auto& frameInstDesc = frameDesc.vecInstanceDescs;

	vector<_float4x4> vecFrameVariantMat = { _float4x4() };


	if (isNotExistLevel)
	{
		std::cout << "[UI_Ovfl_Palette::Load_LevelData] Cannot find pre-defined Level " << iLevelIndex << " Data. Try random loads.." << std::endl;

		_uint iRandIndex = m_pGameInstance->Rand(0.f, static_cast<_float>(vecFilePath.size()) - 0.001f);
		vecLoadDatas = m_pGameSystem->Load_CSV(vecFilePath[iRandIndex].c_str());

		if (vecLoadDatas.empty()) 
			CRASH("CSV Load Failed. Is There File Exist?");
		isLoaded = true;
		iTargetLevel = iRandIndex;
	}
	else
	{
		filesystem::path p(vecFilePath[iLevelIndex]);
		std::cout << "[UI_Ovfl_Palette::Load_LevelData] Trying Load Level.. : " << iLevelIndex << "(" << p.filename().c_str() << ")" << std::endl;

		vecLoadDatas = m_pGameSystem->Load_CSV(vecFilePath[iLevelIndex].c_str());

		if (vecLoadDatas.empty())
			CRASH("CSV Load Failed. Is There File Exist?");
		isLoaded = true;
		iTargetLevel = iLevelIndex;
	}

	for (_uint i = 0; i < static_cast<_uint>(m_arrPalettesInfo.size()); i++)
		for (_uint j = 0; j < static_cast<_uint>(m_arrPalettesInfo[i].size()); j++)
			m_arrPalettesInfo[i][j].eColor = static_cast<PALETTE_COLOR>(stoi(vecLoadDatas[i][j]));

	
	m_iLeftChance		= static_cast<_uint>(stoi(vecLoadDatas[8][0]));
	static_cast<CUI_Text*>(m_pTextUI_LeftChance)->Change_Text(to_wstring(m_iLeftChance));
	m_iMaxChance		= static_cast<_uint>(stoi(vecLoadDatas[8][0]));
	m_eGoalColorIndex	= static_cast<PALETTE_COLOR>(stoi(vecLoadDatas[8][1]));
	static_cast<CUI_Text*>(m_pTextUI_DestColor)->Change_Text(arrText[m_eGoalColorIndex]);
	*reinterpret_cast<_float4*>(&vecFrameVariantMat[0]._11) = m_arrColors[m_eGoalColorIndex];
	CCustom_UI::VARIANTREADY_UI_DESC tFrameVariantDesc = {
		vecFrameVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_SIMPLE_COLORIZE),
		true
	};
	pTargetFrameUI->Set_VariantUIDesc(tFrameVariantDesc);
	auto& targetTestDesc = static_cast<CUI_Text*>(m_pTextUI_DestColor)->Get_TextUIDesc();
	targetTestDesc.vColor = m_arrColors[m_eGoalColorIndex];

	m_iCurTargetLevel	= iTargetLevel;

	return S_OK;
}

void CUI_Ovfl_Palette::Assign_TargetBlocksQueue(_uint iStartBlockIndex)
{
	// 클릭 시 트리거.
	
	m_arrIsVisited.fill(false);
	Calc_NearTarget(iStartBlockIndex);

#ifdef _DEBUG
	_uint iSize = 0;
	for (auto& targets : m_vecTargetsByDepth)
		for (auto& target : targets)
			iSize++;
	std::cout << "[CUI_Ovfl_Palette::Assign_TargetBlocksQueue] Change Queue Calced. (Vector Size : " << m_vecTargetsByDepth.size() << ", Total Size : " << iSize << ")" << std::endl;

	for (_uint i = 0; i < m_vecTargetsByDepth.size(); i++)
	{
		std::cout << "[CUI_Ovfl_Palette::Assign_TargetBlocksQueue] Depth [" << i << "] : ";
		for (_uint j = 0; j < m_vecTargetsByDepth[i].size(); j++)
			std::cout << "(" << m_vecTargetsByDepth[i][j].arrIndex[0] << ", " << m_vecTargetsByDepth[i][j].arrIndex[1] << ") ";
		std::cout << std::endl;
	}
#endif // _DEBUG

			

}

void CUI_Ovfl_Palette::Calc_NearTarget(_uint iBlockIndex)
{
	enum NEXT_TARGET { UP, RIGHT, DOWN, LEFT, END };

	_uint iPaletteSizeX = m_iPaletteSizeX;
	_uint iPaletteSizeY = m_iPaletteSizeY;
	_uint iNumPalettes = m_iNumPalettes;

	m_arrIsVisited.fill(false);
	m_arrDepth.fill(0);

	// [1] 큐 정의 및 최초 위치의 큐 설정
	// [2] 최초 위치 진입 -> 방문 체크, 조건 계산, 주변 인덱스를 큐에 저장 (1회)
	// [3] 다음 깊이 순회 -> 방문 체크, 조건 계산, 주변 인덱스를 큐에 저장 (4회)
	// [4] 다음 깊이 순회.. (8회) 이후 반복
	// [5] 다음 인덱스가 비어있으면 종료
	// [6] 이후, 큐 돌며 진행한 저장 및 계산 결과를 바탕으로 원하는 값 도출


	// 1. 큐 정의, 최초 위치의 큐 설정
	queue<_uint> qTargetIndices = {};

	m_arrIsVisited[iBlockIndex] = true;
	qTargetIndices.push(iBlockIndex);
	m_arrDepth[iBlockIndex]++;

	// 2~5. 반복문 정의, 최초 위치 진입, 반복문 구성
	while (!qTargetIndices.empty())
	{
		// 최초에는 최초 위치, 이후에는 탐색을 통해 다음 큐가 쌓임. 이를 통해 점차 깊어지는 depth 탐색.

		_uint iIndex = qTargetIndices.front();
		qTargetIndices.pop();					// 다음 계산을 위해 인덱스만 뽑고 버림


		// 주변부 탐색..
		//vector<_uint> vecCheckIndices = {};
		_uint iTargets[END] = {};				// >> left, right : +/-를 수행했을 때에 줄바꿈이 일어나지는 않는가의 확인 필요
		iTargets[UP]	=	IS_BETWEEN(iIndex	 - iPaletteSizeX, 0, iNumPalettes)	? iIndex - iPaletteSizeX	: UINT_MAX;
		iTargets[RIGHT]	=	IS_BETWEEN(iIndex	 + 1			, 0, iNumPalettes) &&										
							((iIndex + 1) / iPaletteSizeX == (iIndex / iPaletteSizeX))? iIndex + 1				: UINT_MAX;
		iTargets[DOWN]	=	IS_BETWEEN(iIndex	 + iPaletteSizeX, 0, iNumPalettes)	? iIndex + iPaletteSizeX	: UINT_MAX;
		iTargets[LEFT]	=	IS_BETWEEN(iIndex	 - 1			, 0, iNumPalettes) &&
							((iIndex - 1) / iPaletteSizeX == (iIndex / iPaletteSizeX))? iIndex - 1				: UINT_MAX;

		for (auto& target : iTargets)			// 유효하다면 큐에 삽입
		{
			if (target == UINT_MAX)		continue;		// 유효 X
			if (m_arrIsVisited[target]) continue;		// 이미 탐색한 인덱스 X

			auto iOriginIndex	= m_arrPalettesInfo[iIndex / iPaletteSizeX][iIndex % iPaletteSizeX];
			auto iOtherIndex	= m_arrPalettesInfo[target / iPaletteSizeX][target % iPaletteSizeX];

			_bool isSameColor	= iOriginIndex.eColor == iOtherIndex.eColor;
			if (!isSameColor)			continue;		// 같은 색이 아니면 X


			m_arrDepth[target] = m_arrDepth[iIndex] + 1;
			m_arrIsVisited[target] = true;
			//vecCheckIndices.push_back(target);
			qTargetIndices.push(target);
		}
	}

	_uint iMaxDepth = 0;
	for (auto& depth : m_arrDepth)
		if (iMaxDepth < depth) iMaxDepth = depth;
	m_vecTargetsByDepth.clear();
	m_vecTargetsByDepth.resize(iMaxDepth);


	for (_uint i = 0; i < m_arrDepth.size(); i++)
	{
		_uint iTargetDepth = m_arrDepth[i];
		if (iTargetDepth == 0)	 continue;

		m_vecTargetsByDepth[iTargetDepth - 1].push_back(m_arrPalettesInfo[i / iPaletteSizeX][i % iPaletteSizeX]);
	}

	return;
}

_bool CUI_Ovfl_Palette::Check_ClickedBlockInstance(_uint* OutIndex)
{
	_uint iNumInstBlocks = static_cast<_uint>(m_pUI_InstBlocks->Get_UIDesc().vecInstanceDescs.size());

	_bool isClicked = false;
	_uint iInteractedIndex = UINT_MAX;

	for (_uint i = 0; i < iNumInstBlocks; i++)
	{
		isClicked = m_pUI_InstBlocks->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), i);
		if (isClicked)
		{
			iInteractedIndex = i;
			break;
		}
	}

	*OutIndex = iInteractedIndex;
	return isClicked;
}


_float2 CUI_Ovfl_Palette::Calc_InstBlock_ScrnPos(_uint iInstIndex)
{
	auto& targetInstDesc = m_pUI_InstBlocks->Get_UIDesc().vecInstanceDescs[iInstIndex];

	_float2 vTargetPos = *reinterpret_cast<_float2*>(&targetInstDesc.vSInstTrans);

	_float2 vDebugCenterPos = { vTargetPos.x + (_float)g_iWinSizeX * 0.5f, - vTargetPos.y + (_float)g_iWinSizeY * 0.5f };

	return vTargetPos;
}

void CUI_Ovfl_Palette::Update_ChangeColorBtn()
{
	if (m_isGoinChange)
		return;

	CCustom_UI* pTargetUI = m_pUI_InstColorBtns;

	auto& targetDesc = pTargetUI->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;

	_uint iNumTargetInst = static_cast<_uint>(targetInstDesc.size());
	vector<_float4x4> vecColorBtnVariantMat = {};
	vecColorBtnVariantMat.resize(iNumTargetInst);

	// 항시 색상변경 반영
	for (_uint i = 0; i < vecColorBtnVariantMat.size(); i++)
	{
		// [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
		*reinterpret_cast<_float4*>(&vecColorBtnVariantMat[i]._11) = m_arrColors[i];
		//..
		*reinterpret_cast<_float2*>(&vecColorBtnVariantMat[i]._41) = _float2{1340.f, 1080.f};		// extra texture size.

	}
	
	CCustom_UI::VARIANTREADY_UI_DESC tColorBtnVariantDesc = {
		vecColorBtnVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_OVFL_PALETTE),
		true
	};

	pTargetUI->Set_VariantUIDesc(tColorBtnVariantDesc);



	// {Interact] 클릭 시 색상 변경
	for (_uint i = 0; i < iNumTargetInst; i++)
		if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), i))
		{
			m_eDestColorIndex = static_cast<PALETTE_COLOR>(i);

#ifdef _DEBUG
			_string strDebugText = {};
			switch (m_eDestColorIndex)
			{
			case Client::CUI_Ovfl_Palette::PCOLOR_RED:		strDebugText = "RED";			break;
			case Client::CUI_Ovfl_Palette::PCOLOR_GREEN:	strDebugText = "GREEN";			break;
			case Client::CUI_Ovfl_Palette::PCOLOR_BLUE:		strDebugText = "BLUE";			break;
			case Client::CUI_Ovfl_Palette::PCOLOR_YELLOW:	strDebugText = "YELLOW";		break;
			case Client::CUI_Ovfl_Palette::PCOLOR_END:		strDebugText = "END";			break;
			}

			std::cout << "[CUI_Ovfl_Palette::Update_ChangeColorBtn] Changed Color Index : " << strDebugText << "(" << m_eDestColorIndex << ")" << std::endl;
#endif // _DEBUG

			break;
		}



	// [Interact] 클릭 및 호버 시 피드백
	CCustom_UI* pSelectedRing	= m_pUI_InstSelectedRing;
	auto& selectedDesc			= pSelectedRing->Get_UIDesc();
	auto& selectedInstDesc		= selectedDesc.vecInstanceDescs;

	CCustom_UI* pHoveredRing	= m_pUI_InstHoveredRing; 
	auto& hoveredDesc			= pHoveredRing->Get_UIDesc();
	auto& hoveredInstDesc		= hoveredDesc.vecInstanceDescs;


	
	_bool isHovering = false;
	m_iHoveredColorIndex;
	_bool isEntered = false;
	_bool isExited = false;

	for (_uint i = 0; i < iNumTargetInst; i++)
		if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVERING), i))
		{
			if (m_iHoveredColorIndex == PCOLOR_END)						isEntered = true;// 선택 Enter

			m_iHoveredColorIndex = static_cast<PALETTE_COLOR>(i);
			isHovering = true;

#ifdef _DEBUG
			_string strDebugText = {};
			_uint iDebugIndex = UINT_MAX;
			switch (m_iHoveredColorIndex)
			{
			case Client::CUI_Ovfl_Palette::PCOLOR_RED:		strDebugText = "RED";		iDebugIndex = PCOLOR_RED;	 	break;
			case Client::CUI_Ovfl_Palette::PCOLOR_GREEN:	strDebugText = "GREEN";		iDebugIndex = PCOLOR_GREEN;	 	break;
			case Client::CUI_Ovfl_Palette::PCOLOR_BLUE:		strDebugText = "BLUE";		iDebugIndex = PCOLOR_BLUE;	 	break;
			case Client::CUI_Ovfl_Palette::PCOLOR_YELLOW:	strDebugText = "YELLOW";	iDebugIndex = PCOLOR_YELLOW; 	break;
			case Client::CUI_Ovfl_Palette::PCOLOR_END:		strDebugText = "END";		iDebugIndex = PCOLOR_END;	 	break;
			}

			//std::cout << "[CUI_Ovfl_Palette::Update_ChangeColorBtn] Hovered Color Index : " << strDebugText << "(" << iDebugIndex << ")" << std::endl;
#endif // _DEBUG

			break;
		}
	
	if (!isHovering &&
		m_iHoveredColorIndex != PCOLOR_END)
	{
		isExited = true;
		m_iHoveredColorIndex = PCOLOR_END;
	}

	if (isEntered)
		static_cast<CAnimator_UI*>(pHoveredRing->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Color_FadeIn", true);
	if (isExited)
		static_cast<CAnimator_UI*>(pHoveredRing->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Color_FadeOut", true);

	for (_uint i = 0; i < vecColorBtnVariantMat.size(); i++)
	{
		selectedInstDesc[i].vClipTexcoordX = (m_eDestColorIndex == i) ?
			_float2(0.f, 1.f) : _float2(0.f, 0.f);

		hoveredInstDesc[i].vClipTexcoordX = (m_iHoveredColorIndex == i) ?
			_float2(0.f, 1.f) : _float2(0.f, 0.f);
	}

	_bool isChanged_HoveredColorIndex = (m_iPrevHoveredColorIndex != PCOLOR_END) ? m_iPrevHoveredColorIndex != m_iHoveredColorIndex : false;
	if (isChanged_HoveredColorIndex)
	{
		m_pGameInstance->Play_Sound(L"UI_OVFL_MouseOver", ENUM_CLASS(CHANNEL::UI_HOVER), 0.5f);
		//std::cout << "[UI_Ovfl_Palette::Update_ChangeColorBtn] Curr HoverColorIndex : " << m_iPrevHoveredColorIndex << " / " << m_iHoveredColorIndex << std::endl;

	}
	m_iPrevHoveredColorIndex = m_iHoveredColorIndex;

	_bool isChanged_DestColorIndex = (m_iPrevDestColorIndex != PCOLOR_END)? m_iPrevDestColorIndex != m_eDestColorIndex : false;
	if (isChanged_DestColorIndex)
		m_pGameInstance->Play_Sound(L"UI_OVFL_ClickColor", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);
	m_iPrevDestColorIndex = m_eDestColorIndex;


}

void CUI_Ovfl_Palette::Update_ChangeEvent(_float fTimeDelta)
{
	if (!m_isGoinChange)
		return;


	const _float fChangeSpeed = 300.f;
	m_fChangeRadius = m_fChangeRadius + fTimeDelta * fChangeSpeed;	
	// 이 값 change 끝나면 초기화 필요 및, dest 로 있던 색을 각 블럭에 실제 컬러값으로 변경해야 함



	// Update End
	// End 시, changeRadius 초기화 및 타겟들 현재 선택한 색상으로 실제 값변경 필요
	_bool isChangeEnd = m_fChangeRadius >= static_cast<_float>(g_iWinSizeX);		
	
	// 전부 찼는지를 셰이더용 radius가 충분히 커졌는지로 판단..?
	// 화면 기준으로 가장 먼 좌표까지의 길이보다 radius가 더 커졌을 경우.
	const _float fRadiusOffset = -150.f;		// 너무 늦게 끝나는 거 보정용 offset

	// 그냥 각각의 인스턴스 별 로컬 좌표 기준으로 삼기
	// 인덱스에 해당하는 인스턴스의 좌표 찾는 건 함수 정의해놨으니 그거 루프 돌면 될 것 같음

	static _uint iMostFarInstIndex = {};
	_float fMostFarDist = 0.f;

	for (auto palettes : m_vecTargetsByDepth)
		for (auto palette : palettes)
		{
			_uint iSingleInstIndex = palette.arrIndex[0] * m_arrPalettesInfo.size() + palette.arrIndex[1];
			_float2 vInstSrnPos = Calc_InstBlock_ScrnPos(iSingleInstIndex);

			_float fDistance = FLOAT2_LENGTH_NOLOAD(XMLoadFloat2(&vInstSrnPos) - XMLoadFloat2(&m_vChangeStartPos));
			if (fMostFarDist < fDistance)	
			{
				fMostFarDist = fDistance;
				iMostFarInstIndex = iSingleInstIndex;		// 제일 먼데 구하기..
			}

		}


	if (fMostFarDist < (m_fChangeRadius + fRadiusOffset))
		isChangeEnd = true;


	if (isChangeEnd)
	{
		m_isGoinChange = false;
		m_arrIsVisited_Sound.fill(false);
		m_fChangeRadius = 0.f;

		for (auto& targets : m_vecTargetsByDepth)
			for (auto& target : targets)
			{
				m_arrPalettesInfo[target.arrIndex[0]][target.arrIndex[1]].eColor = m_eDestColorIndex;// target.eColor;
			}

		std::cout << "[CUI_Ovfl_Palette::Update_ChangeEvent] Change Finally Applied!" << std::endl;
	}
	 

	// [SOUND] change


	for (auto palettes : m_vecTargetsByDepth)
		for (auto palette : palettes)
		{
			_uint iInstIndex = palette.arrIndex[0] * 10 + palette.arrIndex[1];
			if (m_arrIsVisited_Sound[iInstIndex])
				continue;
			
			_float2 vInstPos = Calc_InstBlock_ScrnPos(iInstIndex);
			_float fInstDistance = XMVectorGetX(XMVector2Length(
				XMLoadFloat2(&vInstPos) - XMLoadFloat2(&m_vChangeStartPos)));
			
			if (fInstDistance < m_fChangeRadius)
			{
				m_arrIsVisited_Sound[iInstIndex] = true;	// 나중에 싹다 false로 초기화하는거 만들어야.
				m_pGameInstance->Play_Sound(L"UI_OVFL_Click", ENUM_CLASS(CHANNEL::UI_HOVER), 0.5f);
			}

			//m_vChangeStartPos;
			//m_fChangeRadius;
			//Calc_InstBlock_ScrnPos();
		}

}

void CUI_Ovfl_Palette::Update_ResetBtn()
{
	CCustom_UI* pTargetUI = m_pUI_ResetHover;
	auto& targetDesc = pTargetUI->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;

	if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), 0))
		Trigger_ResetLevel(m_iCurTargetLevel);

	
	_bool isFinished = (m_isGoinSuccess || m_isGoinFail);
	if (!isFinished)
	{
		if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), 0))
			static_cast<CAnimator_UI*>(pTargetUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_ResetHover_Click");
		if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVER_ENTER), 0))
			static_cast<CAnimator_UI*>(pTargetUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_ResetHover_FadeIn");
		if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVER_EXIT), 0))
			static_cast<CAnimator_UI*>(pTargetUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_ResetHover_FadeOut");
	}

}

void CUI_Ovfl_Palette::Update_FinishEvent()
{
	if (!(m_isGoinFail || m_isGoinSuccess) || m_isFinishedEvent)
		return;

	m_isFinishedEvent = true;

	if		(m_isGoinFail)
	{
		static_cast<CAnimator_UI*>(m_pUI_ResetHover->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_ResetHover_TickLoop");
	}

	else if	(m_isGoinSuccess)
	{
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), L"Event_Minigame_Palette_Success", MINIGAMEPALETTE_SUCCESS_UI_EVENT(m_isGoinSuccess));
		Req_OffPalette();

		m_pGameInstance->Play_Sound(L"UI_OVFL_Close01", ENUM_CLASS(CHANNEL::UI_INTERACT), 0.5f);
	}

	// ksta : 여기에 실패 / 성공 이벤트?

}

void CUI_Ovfl_Palette::Update_PalettesInstance()
{
	// 인스턴스들을 각종 로컬 변수와 m_arrPalettesInfo 에 저장된 색상대로 채워넣는 함수

	// shader custom matrix info..
	// [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
	// [COLORDEST.x] [COLORDEST.y] [COLORDEST.z] [COLORDEST.w]
	// [CHGFRMPOS.x] [CHGFRMPOS.y] [IS_CHANGING] [CHNG_RADIUS] 

	auto& blocksDesc = m_pUI_InstBlocks->Get_UIDesc();
	
	_uint iNumTargetDesc = static_cast<_uint>(blocksDesc.vecInstanceDescs.size());
	vector<_float4x4> vecPaletteVariantMat = {};
	vecPaletteVariantMat.resize(iNumTargetDesc);



	// 우선 변경되어야 하는 것들 먼저 선적용 후 방문 체크

	array<_bool, 80> arrIsVisited = {};
	arrIsVisited.fill(false);

	for (auto& targetsQueue : m_vecTargetsByDepth)
		for (auto& targetQueue : targetsQueue)
		{
			_uint iTargetIndexX = targetQueue.arrIndex[0];
			_uint iTargetIndexY = targetQueue.arrIndex[1];
			_uint iWidth = static_cast<_uint>(m_arrPalettesInfo[0].size());
			_uint iSingleIndex = iTargetIndexX * iWidth + iTargetIndexY;

			auto& target = m_arrPalettesInfo[iTargetIndexX][iTargetIndexY];

			_float4x4& targetMat = vecPaletteVariantMat[iSingleIndex];

			*reinterpret_cast<_float4*>(&targetMat._11) = m_arrColors[target.eColor];		// 현재 블럭의 색상
			*reinterpret_cast<_float4*>(&targetMat._21) = m_arrColors[m_eDestColorIndex];	// 변하려는 색상
			*reinterpret_cast<_float2*>(&targetMat._31) = m_vChangeStartPos;
			*reinterpret_cast<_float*> (&targetMat._33) = static_cast<_float>(m_isGoinChange);
			*reinterpret_cast<_float*> (&targetMat._34) = m_fChangeRadius;
			*reinterpret_cast<_float2*>(&targetMat._41) = _float2{ 1340.f, 1080.f };		// extra texture size.

			arrIsVisited[iSingleIndex] = true;
		}



	// 방문 체크 후 방문했으면 스킵, 아닌 애들은 그냥 현재 본인 색상으로 두기

	for (_uint i = 0; i < m_arrPalettesInfo.size(); i++)
		for (_uint j = 0; j < m_arrPalettesInfo[i].size(); j++)
		{
			_uint iWidth = static_cast<_uint>(m_arrPalettesInfo[i].size());
			_uint iSingleIndex = i * iWidth + j;

			if (arrIsVisited[iSingleIndex] == true)		//  "방문 체크 후 방문했으면 스킵"
				continue;

			_float4x4& targetMat = vecPaletteVariantMat[i * iWidth + j];

			*reinterpret_cast<_float4*>(&targetMat._11) = m_arrColors[m_arrPalettesInfo[i][j].eColor];
			//*reinterpret_cast<_float4*>(&targetMat._21) = m_arrColors[m_eDestColorIndex];
			//*reinterpret_cast<_float2*>(&targetMat._31) = m_vChangeStartPos;
			//*reinterpret_cast<_float*> (&targetMat._33) = static_cast<_float>(m_isGoinChange);
			//*reinterpret_cast<_float*> (&targetMat._34) = m_fChangeRadius;
			*reinterpret_cast<_float2*>(&targetMat._41) = _float2{ 1340.f, 1080.f };		// extra texture size.
		}

	CCustom_UI::VARIANTREADY_UI_DESC tPaletteVariantDesc = {
		vecPaletteVariantMat,
		ENUM_CLASS(UI_VARIANT_FLAG::UIFLAG_OVFL_PALETTE),
		true
	};

	m_pUI_InstBlocks->Set_VariantUIDesc(tPaletteVariantDesc);
}

void CUI_Ovfl_Palette::Update_GoinDisable(_float fTimeDelta)
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

		m_pGameSystem->Set_MouseFix(true);
		
		return;
	}
	
	// 시간 경과 중 애니메이션 재생 (disable 시작 시 1회)
	if (m_iAnimOrder == 0)
	{
		static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Hide");
		m_iAnimOrder++;
	}
}

CUI_Ovfl_Palette* CUI_Ovfl_Palette::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Ovfl_Palette* pInstance = new CUI_Ovfl_Palette(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Ovfl_Palette");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Ovfl_Palette::Clone(void* pArg)
{
	CUI_Ovfl_Palette* pInstance = new CUI_Ovfl_Palette(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Ovfl_Palette");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Ovfl_Palette::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Ovfl_Palette");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
