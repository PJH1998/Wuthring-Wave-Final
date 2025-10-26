#pragma once
#include "Base.h"

NS_BEGIN(Editor)

class CSequencer : public CBase, ImSequencer::SequenceInterface
{
public:
	enum class ITEM_TYPE { CAMERA, SOUND, SCREEN, OBJECT, END };

	typedef struct tagSequenceItem {
		ITEM_TYPE		eType;								// Sequence Item Type
		_int				iFrameStart{}, iFrameEnd{};		// Frame Start / End
		_bool				isExpanded{};						// Can Expand
		_char				szItemLabel[MAX_PATH] = {};
	}SEQUENCE_ITEM;

	typedef struct tagCustomDraw {
		_int iIndex = {};
		ImRect CustomRect;
		ImRect LegendRect;
		ImRect ClippingRect;
		ImRect LegendClippingRect;
		tagCustomDraw(_int _iIndex, const ImRect& _CustomRect, const ImRect& _LegendRect, const ImRect& _ClippingRect, const ImRect& _LegendClippingRect)
			: iIndex { _iIndex }, CustomRect { _CustomRect }, LegendRect { _LegendRect }, ClippingRect { _ClippingRect }, LegendClippingRect { _LegendClippingRect }
		{}
	}CUSTOM_DRAW;

private:
	explicit CSequencer();
	virtual ~CSequencer() = default;

public:
	_int							GetFrameMin() const override { return m_iFrameMin; }
	_int							GetFrameMax() const override { return m_iFrameMax; }
	_int							GetItemCount() const override { return m_Items.size(); }
	// Item Info Get
	void							Get(_int index, _int** start, _int** end, _int* type, _uint* color) override;

	virtual void					Add(_int iType) override;
	virtual const _char*		GetItemTypeName(_int iIndex) const override;

public:
	HRESULT							Initialize();
	void								Update(_float fTimeDelta);

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ImGuiIO							io;

	// Sequence Option
	_int								m_iSequenceOption = {};

	// DrawList
	ImDrawList*						m_pDrawList = { nullptr };

	// Canvas
	_bool								m_isExpanded = { true };
	ImVec2							m_vCanvasPos = {};
	ImVec2							m_vCanvasSize = {};

	// Frame Min / Max
	_int								m_iFrameMin{}, m_iFrameMax{};
	_int								m_iFrameCnt = {};
	_int								m_iCurrentFrame{};
	vector<SEQUENCE_ITEM>	m_Items;
	_int								m_iFirstFrame = {};

	// Content
	ImVec2							m_vContentMin = {};
	ImVec2							m_vContentMax = {};
	ImRect							m_ContentRect = {};
	_float								m_fContentHeight = {};

	// Draw Frame
	_float								m_fFramePixelWidth = { 10.f };
	_float								m_fFramePixelWidthTarget = { 10.f };
	_int								m_iItemHeight = { 20 };						// Item 1개 당 Height
	_int								m_iLegendWidth = { 200 };					// 범례(표시 내용) Width

	ImVec2							m_vChildFramePos = {};
	ImVec2							m_vChildFrameSize = {};

	// Moving
	_int								m_iMovingEntry = { -1 };
	_int								m_iMovingPos = { -1 };
	_int								m_iMovingPart = { -1 };
	_bool								m_isMovingScrollBar = { false };
	_bool								m_isMovingCurrentFrame = { false };
	_bool								m_isRet = { false };

	// Panning
	_bool								m_isPanningView = { false };
	ImVec2							m_vPanningViewSource = {};
	_int								m_iPanningViewFrame = {};

	// Entry
	_int								m_iSelectedEntry = {-1};
	_int								m_iDelEntry = { -1 };
	_int								m_iDupEntry = { -1 };
	
	// PopUp
	_bool								m_isPopUp = { false };

	// Header
	_int								m_iModFrameCnt = { 10 };
	_int								m_iHalfModFrameCnt = {};
	_int								m_iFrameStep = { 1 };

	// Custom Draw
	vector<CUSTOM_DRAW>	m_CustomDraws;
	vector<CUSTOM_DRAW>	m_CompactCustomDraws;

	// Cursor
	_float								m_fCursorWidth = { 8.f };

	// ScrollBar
	_bool								m_isScrollBar = { true };
	_int								m_iVisibleFrameCnt = {};
	_float								m_fBarWidthRatio = {};
	_float								m_fBarWidthInPixels = {};
	_bool								m_isSizingRightBar = { false };
	_bool								m_isSizingLeftBar = { false };
	_float								m_fMinBarWidth = {};

private:
	void								Drawing();
	void								Panning(const _int iVisibleFrameCnt);	//  (Alt + Wheel Click -> Drag => 화면 좌우 이동)
	void								Expand(_int iControllHeight);	// Canvas Expand
	void								DrawFrame();						// Sequence 프레임
	void								DrawLegend();						// List
	void								DrawSlot();							// Slot
	void								Moving();							// Item Duration 조절 (Left, Right 잡아당기기)		
	void								Cursor();								// Cursor (Frame 화면 붉은 선)
	void								CopyPaste();							// Copy / Paste
	void								ScrollBar();							// ScrollBar

	// Header
	void								DrawLine(_int iFrame, _int iRegionHeight);
	void								DrawLineContent(_int iFrame, _int iRegionHeight);

private:
	_bool								SequencerAddDelButton(ImVec2 vPos, _bool isAdd = true);

public:
	static		CSequencer*	Create();
	virtual		void				Free() override;
};

NS_END