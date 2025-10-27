#pragma once
#include "Base.h"

NS_BEGIN(Editor)

class CSequencer : public CBase, ImSequencer::SequenceInterface
{
public:
	enum class ITEM_TYPE { CAMERA, SOUND, SCREEN, OBJECT };

	typedef struct tagSequenceItem {
		ITEM_TYPE		eType;								// Sequence Item Type
		_int				iFrameStart{}, iFrameEnd{};		// Frame Start / End
		_bool				isExpanded{};						// Can Expand
	}SEQUENCE_ITEM;

private:
	explicit CSequencer();
	virtual ~CSequencer() = default;

public:
	_int	GetFrameMin() const override { return m_iFrameMin; }
	_int	GetFrameMax() const override { return m_iFrameMax; }
	_int	GetItemCount() const override { return m_Items.size(); }
	void	Get(_int index, _int** start, _int** end, _int* type, _uint* color) override;

public:
	HRESULT							Initialize();
	void								Update(_float fTimeDelta);

private:
	class CGameInstance*		m_pGameInstance = { nullptr };

	// DrawList
	ImDrawList*						m_pDrawList = { nullptr };

	// Frame Min / Max
	_int								m_iFrameMin{}, m_iFrameMax{};
	_int								m_iFrameCnt = {};
	_int								m_iCurrentFrame{};
	vector<SEQUENCE_ITEM>	m_Items;
	_int								m_iFirstFrame = {};

	// Draw Frame
	_float								m_fFramePixelWidth = { 10.f };
	_float								m_fFramePixelWidthTarget = { 10.f };
	_int								m_iItemHeight = { 20 };						// Item 1개 당 Height
	_int								m_iLegendWidth = { 200 };					// 범례(표시 내용) Width

	_int								m_iMovingEntry = { -1 };
	_int								m_iMovingPos = { -1 };
	_int								m_iMovingPart = { -1 };

	_bool								m_isMovingScrollBar = { false };
	_bool								m_isMovingCurrentFrame = { false };

	_bool								m_isPanningView = { false };
	ImVec2							m_vPanningViewSource = {};
	_int								m_iPanningViewFrame = {};

private:
	void								Drawing();
	void								Panning(const _int iVisibleFrameCnt);	//  (Alt + Wheel Click -> Drag => 화면 좌우 이동)

public:
	static		CSequencer*	Create();
	virtual		void				Free() override;
};

NS_END