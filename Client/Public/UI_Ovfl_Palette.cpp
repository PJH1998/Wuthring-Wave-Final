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
	
	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		// UI Anims..
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Show.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Palette_Hide.json",
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

	Update_HoverEvent();


	// is KeyDown
	Trigger_ClickEvent();

	
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

	static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"Palette_Show");;
}

HRESULT CUI_Ovfl_Palette::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_Ovfl_Palette::PreAssign_Presets()
{
	// Pre-Assign Color Presets
	m_arrColors[PCOLOR_RED]		= _float4( 1.000f, 0.000f, 0.000f, 1.000f );
	m_arrColors[PCOLOR_GREEN]	= _float4( 0.000f, 1.000f, 0.000f, 1.000f );
	m_arrColors[PCOLOR_BLUE]	= _float4( 0.000f, 0.000f, 1.000f, 1.000f );
	m_arrColors[PCOLOR_YELLOW]	= _float4( 1.000f, 1.000f, 0.000f, 1.000f );
	m_arrColors[PCOLOR_END]		= _float4( 1.000f, 0.000f, 1.000f, 1.000f );		// Magenta.
	
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
	m_pRUI_All				 = Find_ChildObject(L"Sub_All");

	m_pUIBackgrounds		 = Find_ChildObject(L"SectorA_Backgrounds");
	m_pUIForegrounds		 = Find_ChildObject(L"SectorA_Foregrounds");
	m_pUISideThings			 = Find_ChildObject(L"SectorA_SideThings");
	m_pUIOthers				 = Find_ChildObject(L"SectorA_Others");
	
	m_pUI_InstBlocks		 = Find_ChildObject(L"FG_InstBlocks");
	m_pUI_InstHoverBlocks	 = Find_ChildObject(L"FG_InstHoverBlocks");
	m_pUI_InstColorBtns		 = Find_ChildObject(L"Side_ColorButton");
}

void CUI_Ovfl_Palette::Trigger_ClickEvent()
{
	_uint iClickedIndex = {};
	if (Check_ClickedBlockInstance(&iClickedIndex))	// if Block Clicked !
	{
		_uint iIndexX = iClickedIndex / m_iPaletteSizeX;
		_uint iIndexY = iClickedIndex % m_iPaletteSizeX;

		UI_PALETTE_DESC& targetDesc = m_arrPalettesInfo[iIndexX][iIndexY];

		m_eDestColorIndex = static_cast<PALETTE_COLOR>(targetDesc.eColor);
		m_vChangeStartPos = Calc_InstBlock_ScrnPos(iClickedIndex);
		m_isChanging;
		m_fChangeRadius;

		Assign_TargetBlocksQueue(iClickedIndex);
		Update_PalettesInstance();
	}
}

void CUI_Ovfl_Palette::Update_HoverEvent()
{
	CCustom_UI* pTargetUI = m_pUI_InstHoverBlocks;
	CCustom_UI* pHoverCheckTargetUI = m_pUI_InstBlocks;

	// 마우스가 올라간 인스턴스를 탐색
	_uint iNumInstHovers = pTargetUI->Get_UIDesc().vecInstanceDescs.size();

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

	static _uint iRandColor = m_pGameInstance->Rand(0.f, 3.999f);
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
	

	// 재귀로 주변 탐색 진행 및 저장
	fill(m_arrIsVisited.begin(), m_arrIsVisited.end(), false);
	Calc_NearTarget(iStartBlockIndex);
}

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

        _bool isSameColor = targetPalette.eColor == originPalette.eColor;
        if (isSameColor)
            vecTargetQueue.push_back(targetPalette);							// 지역 변수에 대상 삽입
    }

	if (vecTargetQueue.empty())
		return false;


	// - [3] 지역의 탐색 결과를 클래스의 로컬 변수로 삽입. 이후 재귀 호출.

	m_vecTargetsQueue.resize(iDepth);
	m_vecTargetsQueue[iDepth - 1] = vecTargetQueue;

	for (auto& target : vecTargetQueue)
	{
		_uint iCurIndex = target.arrIndex[0] * iPaletteSizeX + target.arrIndex[1];
		Calc_NearTarget(iCurIndex, iDepth);
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

_float2 CUI_Ovfl_Palette::Calc_InstBlock_ScrnPos(_uint iInstIndex)
{
	auto targetInstDesc = m_pUI_InstBlocks->Get_UIDesc().vecInstanceDescs[iInstIndex];

	_float2 fTargetPos = *reinterpret_cast<_float2*>(&targetInstDesc.vSInstTrans);
	return fTargetPos;
}

void CUI_Ovfl_Palette::Update_PalettesInstance()
{
	// 인스턴스들을 m_arrPalettesInfo 에 저장된 색상대로 채워넣는 함수


	auto blocksDesc = m_pUI_InstBlocks->Get_UIDesc();
	
	vector<_float4x4> vecPaletteVariantMat = {};
	vecPaletteVariantMat.resize(80);

	for (_uint i = 0; i < m_arrPalettesInfo.size(); i++)
		for (_uint j = 0; j < m_arrPalettesInfo[i].size(); j++)
		{
			_uint iWidth = m_arrPalettesInfo[i].size();
			_uint iHeight = m_arrPalettesInfo.size();

			_float4x4& targetMat = vecPaletteVariantMat[i * iWidth + j];
			
			// [COLORCURR.x] [COLORCURR.y] [COLORCURR.z] [COLORCURR.w]
			// [COLORDEST.x] [COLORDEST.y] [COLORDEST.z] [COLORDEST.w]
			// [CHGFRMPOS.x] [CHGFRMPOS.y] [IS_CHANGING] [CHNG_RADIUS] 

			*reinterpret_cast<_float4*>(&targetMat._11) = m_arrColors[m_arrPalettesInfo[i][j].eColor];
			*reinterpret_cast<_float4*>(&targetMat._21) = m_arrColors[m_eDestColorIndex];
			*reinterpret_cast<_float2*>(&targetMat._31) = m_vChangeStartPos;
			*reinterpret_cast<_float*> (&targetMat._33) = static_cast<_float>(m_isChanging);
			*reinterpret_cast<_float*> (&targetMat._34) = m_fChangeRadius;
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
