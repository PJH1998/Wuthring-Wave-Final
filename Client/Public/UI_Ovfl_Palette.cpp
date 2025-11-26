#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_Ovfl_Palette.h"
#include "UI_Text.h"
#include "GameSystem.h"

#define	 IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만
#define	 FLOAT2_LENGTH(x)								(XMVectorGetX(XMVector2Length(XMLoadFloat2(x))))


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
	};
	Load_Animations(vecAnimFilePaths);

	// Load Levels.
	Load_LevelData(0);


	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Initialize");

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

	Update_ChangeColorBtn();
	Update_HoverEvent();

	// is KeyDown
	Trigger_ClickEvent();
	Update_ChangeEvent(fTimeDelta);

#ifdef _DEBUG

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) == KEYSTATE::PRESS &&
		m_pGameInstance->Get_DIKeyState(DIK_R) == KEYSTATE::DOWN)
		Load_LevelData();

#endif // _DEBUG


	Update_PalettesInstance();
	Update_GoinDisable(fTimeDelta);

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
	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0;
	m_isActivate = true;

	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Show");
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

	if (FAILED(m_pUI_InstBlocks			->Add_ExtraTexture(strExtraTexName_Noise12)))	return E_FAIL;
	if (FAILED(m_pUI_InstBlocks			->Add_ExtraTexture(strExtraTexName_Noise14)))	return E_FAIL;


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

	m_pUI_InstBlocks		= Find_ChildObject(L"FG_InstBlocks");
	m_pUI_InstHoverBlocks	= Find_ChildObject(L"FG_InstHoverBlocks");
	m_pUI_InstColorBtns		= Find_ChildObject(L"Side_ColorButton");
	m_pUI_InstSelectedRing	= Find_ChildObject(L"Side_SelectedRing");
	m_pUI_InstHoveredRing	= Find_ChildObject(L"Side_HoveredRing");
}

void CUI_Ovfl_Palette::Trigger_ClickEvent()
{
	_uint iClickedIndex = {};
	if (Check_ClickedBlockInstance(&iClickedIndex))	// if Block Clicked !
	{
		m_isGoinChange = true;

		_uint iIndexX = iClickedIndex / m_iPaletteSizeX;
		_uint iIndexY = iClickedIndex % m_iPaletteSizeX;

		UI_PALETTE_DESC& targetDesc = m_arrPalettesInfo[iIndexX][iIndexY];

		m_eDestColorIndex = m_eDestColorIndex;
		m_vChangeStartPos = Calc_InstBlock_ScrnPos(iClickedIndex);
		m_isGoinChange;
		m_fChangeRadius;

		Assign_TargetBlocksQueue(iClickedIndex);
	}
}

void CUI_Ovfl_Palette::Update_HoverEvent()
{
	CCustom_UI* pTargetUI = m_pUI_InstHoverBlocks;
	CCustom_UI* pHoverCheckTargetUI = m_pUI_InstBlocks;

	// 마우스가 올라간 인스턴스를 탐색
	_uint iNumInstHovers = static_cast<_uint>(pTargetUI->Get_UIDesc().vecInstanceDescs.size());

	_bool isHovered = false;
	_uint iHoveredIndex = UINT_MAX;

	for (_uint i = 0; i < iNumInstHovers; i++)
	{
		isHovered = pHoverCheckTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVERING), i);
		if (isHovered)
		{
			iHoveredIndex = i;
			break;
		}
	}

	// 적용
	auto targetDesc = pTargetUI->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;

	for (_uint i = 0; i < targetInstDesc.size(); i++)
	{
		auto& instDesc = targetInstDesc[i];

		instDesc.vClipTexcoordX = (iHoveredIndex == i) ? _float2{ 0.f, 1.f } : _float2{ 0.f, 0.f }; // 마우스를 올린 게 있으면 {0.f, 1.f} 가 들어가야 함
	}

	pTargetUI->Set_UIDesc(targetDesc);
}

HRESULT CUI_Ovfl_Palette::Load_LevelData(_uint iLevelIndex)
{
	// ksta : 임시 랜덤 생성. 나중에 패턴 추가 필요
	iLevelIndex; // 이거 써서 로드 분기화 !!!

	static _uint iRandColor = static_cast<_uint>(m_pGameInstance->Rand(0.f, 3.999f));
	const _float fColorChangeChance = 0.1f;

	for (auto& palettes : m_arrPalettesInfo)
		for (auto& palette : palettes)
		{
			if (m_pGameInstance->Rand_Normal() <= fColorChangeChance)
				iRandColor = static_cast<_uint>(m_pGameInstance->Rand(0.f, 3.999f));

			palette.eColor = static_cast<PALETTE_COLOR>(iRandColor);
		}

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


/*
_bool CUI_Ovfl_Palette::Calc_NearTarget(_uint iBlockIndex, _uint iDepth)
{
	if (m_arrIsVisited[iBlockIndex])
		return false;


	m_arrIsVisited[iBlockIndex] = true;

	_uint iPaletteSizeX = m_iPaletteSizeX;
	_uint iPaletteSizeY = m_iPaletteSizeY;
	_uint iNumPalettes = m_iNumPalettes;

	// - [0] 최초 호출인 경우, 클릭한 블럭을 클래스 로컬 변수에 삽입 후 리턴 (이 경우에는 주변부 블럭의 검사 필요 X)

	if (iDepth == 0)
	{
		m_vecTargetsQueue.clear();		// 재귀 최초 시작 시 초기화
		m_vecTargetsQueue.resize(1);

		m_vecTargetsQueue[iDepth].push_back(m_arrPalettesInfo[iBlockIndex / iPaletteSizeX][iBlockIndex % iPaletteSizeX]);
		m_arrIsVisited[iBlockIndex] = false;
		iDepth++;
		return Calc_NearTarget(iBlockIndex, iDepth);
	}


	// - [1] 상하좌우 확인, 유효한 인덱스면 해당 인덱스 삽입, 아니면 UINT_MAX 삽입

	iDepth++;
	enum NEXT_TARGET {UP, RIGHT, DOWN, LEFT, END};

	_uint iTargets[END] = {};
	iTargets[UP]		= IS_BETWEEN(iBlockIndex - iPaletteSizeX, 0, iNumPalettes) ? iBlockIndex - iPaletteSizeX	: UINT_MAX;
	iTargets[RIGHT]		= IS_BETWEEN(iBlockIndex + 1			, 0, iNumPalettes) ? iBlockIndex + 1				: UINT_MAX;
	iTargets[DOWN]		= IS_BETWEEN(iBlockIndex + iPaletteSizeX, 0, iNumPalettes) ? iBlockIndex + iPaletteSizeX	: UINT_MAX;
	iTargets[LEFT]		= IS_BETWEEN(iBlockIndex - 1			, 0, iNumPalettes) ? iBlockIndex - 1				: UINT_MAX;


	// - [2] 주변이 같은 색상인지 판별 후 같다면 탐색 결과를 지역에 삽입. 전부 아니면 return false.			(재귀를 끊는 단계)
	
	vector<UI_PALETTE_DESC> vecTargetQueue = {};

	for (_uint i = 0; i < END; i++)
    {
        _uint target = iTargets[i];
        if (target == UINT_MAX)		// 만약 해당 블럭이 유효치 않다면 skip.
            continue;

        _uint iIndexX = target / iPaletteSizeX;					
        _uint iIndexY = target % iPaletteSizeX;

        _uint iOriginIndexX = iBlockIndex / iPaletteSizeX;		
        _uint iOriginIndexY = iBlockIndex % iPaletteSizeX;

        auto& targetPalette = m_arrPalettesInfo[iIndexX][iIndexY];				// 주변 블럭 (비교대상)
        auto& originPalette = m_arrPalettesInfo[iOriginIndexX][iOriginIndexY];	// 중앙 블럭

		_bool isSameColor = (targetPalette.eColor == originPalette.eColor);
        if (isSameColor)
            vecTargetQueue.push_back(targetPalette);							// 지역 변수에 대상 삽입
    }

	if (vecTargetQueue.empty())
		return false;


	// - [3] 지역의 탐색 결과를 클래스의 로컬 변수로 삽입. 이후 재귀 호출.
	// 
	// 이렇게 하면 이미 위, 자식의 위, 그 자식의 위.. 으로 꽉 채워서 돌고,
	// 다시 0, 0으로 돌아와서 오른쪽을 보려 할 떈 이미 방문해서 스킵되는 식으로 로직이 망가져있음 

	m_vecTargetsQueue.resize(iDepth);
	m_vecTargetsQueue[iDepth - 1].insert(m_vecTargetsQueue[iDepth - 1].end(), vecTargetQueue.begin(), vecTargetQueue.end());

	for (auto& target : vecTargetQueue)
	{
		_uint iFixedDepth = iDepth;
		_uint iCurIndex = target.arrIndex[0] * iPaletteSizeX + target.arrIndex[1];
		Calc_NearTarget(iCurIndex, iFixedDepth);
	}

	return true;
}

_bool CUI_Ovfl_Palette::Check_ClickedBlockInstance(_uint* OutIndex)
{
	_uint iNumInstBlocks = m_pUI_InstBlocks->Get_UIDesc().vecInstanceDescs.size();

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
*/


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
							((iIndex + 1) / iNumPalettes == (iIndex / iNumPalettes))? iIndex + 1				: UINT_MAX;
		iTargets[DOWN]	=	IS_BETWEEN(iIndex	 + iPaletteSizeX, 0, iNumPalettes)	? iIndex + iPaletteSizeX	: UINT_MAX;
		iTargets[LEFT]	=	IS_BETWEEN(iIndex	 - 1			, 0, iNumPalettes) &&
							((iIndex - 1) / iNumPalettes == (iIndex / iNumPalettes))? iIndex - 1				: UINT_MAX;

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
	auto targetInstDesc = m_pUI_InstBlocks->Get_UIDesc().vecInstanceDescs[iInstIndex];

	_float2 vTargetPos = *reinterpret_cast<_float2*>(&targetInstDesc.vSInstTrans);

	_float2 vDebugCenterPos = { vTargetPos.x + (_float)g_iWinSizeX * 0.5f, - vTargetPos.y + (_float)g_iWinSizeY * 0.5f };

	return vTargetPos;
}

void CUI_Ovfl_Palette::Update_ChangeColorBtn()
{
	CCustom_UI* pTargetUI = m_pUI_InstColorBtns;

	auto targetDesc = pTargetUI->Get_UIDesc();
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
	auto selectedDesc			= pSelectedRing->Get_UIDesc();
	auto& selectedInstDesc		= selectedDesc.vecInstanceDescs;

	CCustom_UI* pHoveredRing	= m_pUI_InstHoveredRing; 
	auto hoveredDesc			= pHoveredRing->Get_UIDesc();
	auto& hoveredInstDesc		= hoveredDesc.vecInstanceDescs;


	
	_bool isHovering = false;
	static _uint iHoveredIndex = Client::CUI_Ovfl_Palette::PCOLOR_END;
	_bool isEntered = false;
	_bool isExited = false;

	for (_uint i = 0; i < iNumTargetInst; i++)
		if (pTargetUI->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::HOVERING), i))
		{
			if (iHoveredIndex == PCOLOR_END)						isEntered = true;// 선택 Enter
			iHoveredIndex = static_cast<PALETTE_COLOR>(i);

			isHovering = true;

#ifdef _DEBUG
			_string strDebugText = {};
			switch (iHoveredIndex)
			{
			case Client::CUI_Ovfl_Palette::PCOLOR_RED:		strDebugText = "RED";			break;
			case Client::CUI_Ovfl_Palette::PCOLOR_GREEN:	strDebugText = "GREEN";			break;
			case Client::CUI_Ovfl_Palette::PCOLOR_BLUE:		strDebugText = "BLUE";			break;
			case Client::CUI_Ovfl_Palette::PCOLOR_YELLOW:	strDebugText = "YELLOW";		break;
			case Client::CUI_Ovfl_Palette::PCOLOR_END:		strDebugText = "END";			break;
			}

			std::cout << "[CUI_Ovfl_Palette::Update_ChangeColorBtn] Hovered Color Index : " << strDebugText << "(" << m_eDestColorIndex << ")" << std::endl;
#endif // _DEBUG

			break;
		}
	
	if (!isHovering &&
		iHoveredIndex != PCOLOR_END)
	{
		isExited = true;
		iHoveredIndex = PCOLOR_END;
	}

	if (isEntered)
		static_cast<CAnimator_UI*>(pHoveredRing->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Color_FadeIn", true);
	if (isExited)
		static_cast<CAnimator_UI*>(pHoveredRing->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Color_FadeOut", true);

	for (_uint i = 0; i < vecColorBtnVariantMat.size(); i++)
	{
		selectedInstDesc[i].vClipTexcoordX = (m_eDestColorIndex == i) ?
			_float2(0.f, 1.f) : _float2(0.f, 0.f);

		hoveredInstDesc[i].vClipTexcoordX = (iHoveredIndex == i) ?
			_float2(0.f, 1.f) : _float2(0.f, 0.f);
	}


	pSelectedRing->Set_UIDesc(selectedDesc);
	pHoveredRing->Set_UIDesc(hoveredDesc);
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
	_bool isChangeEnd = m_fChangeRadius >= static_cast<_float>(g_iWinSizeX);		// 전부 찼는지를 셰이더용 radius가 충분히 커졌는지로 판단..?

	if (isChangeEnd)
	{
		m_isGoinChange = false;
		m_fChangeRadius = 0.f;

		for (auto& targets : m_vecTargetsByDepth)
			for (auto& target : targets)
			{
				m_arrPalettesInfo[target.arrIndex[0]][target.arrIndex[1]].eColor = m_eDestColorIndex;// target.eColor;
			}

		std::cout << "[CUI_Ovfl_Palette::Update_ChangeEvent] Change Finally Applied!" << std::endl;
	}
	 

}

void CUI_Ovfl_Palette::Update_PalettesInstance()
{
	// 인스턴스들을 각종 로컬 변수와 m_arrPalettesInfo 에 저장된 색상대로 채워넣는 함수

	// shader custom matrix info..
	// [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
	// [COLORDEST.x] [COLORDEST.y] [COLORDEST.z] [COLORDEST.w]
	// [CHGFRMPOS.x] [CHGFRMPOS.y] [IS_CHANGING] [CHNG_RADIUS] 

	auto blocksDesc = m_pUI_InstBlocks->Get_UIDesc();
	
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
