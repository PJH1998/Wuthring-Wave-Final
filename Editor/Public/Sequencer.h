#pragma once
#include "Base.h"

NS_BEGIN(Editor)

struct RampEdit : public ImCurveEdit::Delegate
{
	RampEdit()
	{
		mbVisible[0] = true;
		mbVisible[1] = mbVisible[2] = false;
		mMax = ImVec2(1.f, 1.f);
		mMin = ImVec2(0.f, 0.f);
	}
	size_t GetCurveCount()
	{
		return 1;
	}

	bool IsVisible(size_t curveIndex)
	{
		return mbVisible[curveIndex];
	}
	size_t GetPointCount(size_t curveIndex)
	{
		return mPoints.size();
	}

	uint32_t GetCurveColor(size_t curveIndex)
	{
		uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
		return cols[curveIndex];
	}
	ImVec2* GetPoints(size_t curveIndex)
	{
		return mPoints.data();
	}
	virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return ImCurveEdit::CurveSmooth; }
	virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{
		mPoints[pointIndex] = ImVec2(value.x, value.y);
		SortValues();
		for (size_t i = 0; i < GetPointCount(0); i++)
		{
			if (mPoints[i].x == value.x)
				return (int)i;
		}
		return pointIndex;
	}
	virtual void AddPoint(size_t iType, ImVec2 value)
	{
		mPoints.push_back(value);
		if (ENUM_CLASS(ITEM_TYPE::ACTION) == iType)
		{
			CAMERA_FRAME frame = {};
			frame.vTranslation = _float3(0.f, 0.f, 0.f);
			frame.vRotation = _float4(0.f, 0.f, 0.f, 0.f);
			frame.fDistance = 10.f;
			frame.fStartFrame = 0.f;
			mTargetCameraFrames.push_back(frame);
		}

		SortValues();
	}
	virtual ImVec2& GetMax() { return mMax; }
	virtual ImVec2& GetMin() { return mMin; }
	virtual unsigned int GetBackgroundColor() { return 0; }

	void Update_Frame() {
		for (size_t i = 0; i < mPoints.size(); ++i)
			mTargetCameraFrames[i].fStartFrame = mPoints[i].x;
	}

	vector<ImVec2>		mPoints;
	vector<CAMERA_FRAME> mTargetCameraFrames;
	vector<_float3>		mPositions;
	vector<_float3>		mRotations;
	size_t			mPointCount = {};
	_bool			mbVisible[3];
	ImVec2		mMin;
	ImVec2		mMax;

	_int			miSelectCurve = { -1 };
	_int			miSelectPoint = { -1 };

private:
	void SortValues()
	{
		auto b = std::begin(mPoints);
		auto e = std::begin(mPoints) + GetPointCount(0);
		std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
	}
};

class CSequencer : public CBase, ImSequencer::SequenceInterface
{
public:
	typedef struct tagSequenceItem {
		ITEM_TYPE		eType;								// Sequence Item Type
		_int				iFrameStart{}, iFrameEnd{};		// Frame Start / End
		_bool				isExpanded{};						// Can Expand
		_char				szItemLabel[MAX_PATH] = {};
		RampEdit		mRampEdit;
		tagSequenceItem(_int iType , _int _iFrameStart, _int _iFrameEnd, _bool _isExpanded, const _char* pLabel)
			: iFrameStart {_iFrameStart}, iFrameEnd {_iFrameEnd}, isExpanded {_isExpanded}
		{
			strcpy_s(szItemLabel, MAX_PATH, pLabel);
			switch (iType)
			{
			case ENUM_CLASS(ITEM_TYPE::ACTOR):
				eType = ITEM_TYPE::ACTOR;
				break;
			case ENUM_CLASS(ITEM_TYPE::ACTION):
				eType = ITEM_TYPE::ACTION;
				break;
			case ENUM_CLASS(ITEM_TYPE::EFFECT):
				eType = ITEM_TYPE::EFFECT;
				break;
			case ENUM_CLASS(ITEM_TYPE::SCENE):
				eType = ITEM_TYPE::SCENE;
				break;
			case ENUM_CLASS(ITEM_TYPE::SCREEN):
				eType = ITEM_TYPE::SCREEN;
				break;
			case ENUM_CLASS(ITEM_TYPE::SOUND):
				eType = ITEM_TYPE::SOUND;
				break;
			}
		}
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
	virtual size_t					GetCustomHeight(_int iIndex) { return m_iSelectedEntry == iIndex && m_Items[iIndex].isExpanded ? 300 : 0; }

	void							CustomDraw(RampEdit& delegate, _int iIndex, const ImRect& customRect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect);
	void							CustomDrawCompact(RampEdit& delegate, _int iIndex, const ImRect& customRect, const ImRect& clippingRect);

public:
	HRESULT							Initialize();
	void								Update(_float fTimeDelta);

#pragma region Private Varation
private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ImGuiIO							io;

	// Custom Draw Label
	const _char* m_pCustomDrawLabel[3] = { "Translation", "Rotation", "Scale" };

	// Sequence Option
	_int								m_iSequenceOption = {};

	// DrawList
	ImDrawList* m_pDrawList = { nullptr };

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
	_int								m_iSelectedEntry = { -1 };						// Select Entry(항목)
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

#pragma endregion

private:
	_bool								m_isSave = { false };

	_bool								m_isPlay = { false };
	_float								m_fTrackPerSec = {};
	_float								m_fTrackAcc = {};

private:
	// Play
	void								Play(_float fTimeDelta);

	// Selectable Item
	void								Selectable_Item();
	void								SetUp_Point(SEQUENCE_ITEM& item);
	void								SetUp_Camera(SEQUENCE_ITEM& item);
	void								Sorting_Item();

	// Camera Action
	void								Save_CameraAction();
	void								Load_CameraAction();
	
	// GUI
	void								Drawing();
	void								Panning(const _int iVisibleFrameCnt);	//  (Alt + Wheel Click -> Drag => 화면 좌우 이동)
	void								Expand(_int iControllHeight);	// Canvas Expand
	void								DrawFrame();						// Sequence 프레임
	void								DrawLegend();						// List
	void								DrawSlot();							// Slot (Item)
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