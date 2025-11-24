#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_Ovfl_Palette final : public CCustom_UI
{
public:


private:
	enum PALETTE_COLOR : _uint { PCOLOR_RED, PCOLOR_GREEN, PCOLOR_BLUE, PCOLOR_YELLOW, PCOLOR_END };

	typedef struct tUIPaletteDesc {
		array<_uint, 2>		arrIndex = {};
		PALETTE_COLOR		eColor = PCOLOR_END;
	} UI_PALETTE_DESC;

public:
	explicit CUI_Ovfl_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_Ovfl_Palette(const CUI_Ovfl_Palette& Prototype);
	virtual ~CUI_Ovfl_Palette() = default;

public:
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

	HRESULT			Ready_Components(void* pArg);

public:
	void			Req_OffPalette()		{ m_IsGoinDisabled = true;}

private:
	void			PreAssign_Presets();
	void			PreAssign_ChildUIs();

private:
	// 좀 정제된 함수들은 여기로..
	void			Trigger_ClickEvent();
	void			Update_HoverEvent();

private:
	HRESULT			Load_LevelData(_uint iLevelIndex);				// [임시] 외부든 내부든, 퍼즐의 패턴을 정의 및 로드 할 필요가 있음

	// 클릭 시,
	// 1. 주변 블록 탐색 및 동일 색상이면 저장을 반복. 이는 유사 재귀식으로 작용할 필요 있음
	// 2. 해당 탐색 결과를 로컬 vector에 담음. 이후 size 기반으로 count 넘어가면서 순차적 변화 + 애니메이션 재생.
	void			Assign_TargetBlocksQueue(_uint iStartBlockIndex);			// 주변 박스를 순회하며, 같은 색인지 확인하고, 결과를 m_vecTargetsQueue 에 저장한다.
	_bool			Calc_NearTarget(_uint iBlockIndex, _uint iDepth = 0);		// 재귀용. 직접 호출 X

	_bool			Check_ClickedBlockInstance(_uint* OutIndex);	// 몇 번째 인스턴스가 눌림?
	_float2			Calc_InstBlock_ScrnPos(_uint iInstIndex);		// 그 인스턴스의 스크린 좌표가 어디임?
	

	void			Update_PalettesInstance();						// [임시] 로컬에 저장된 변수를 기반으로, 블럭들의 variantInstDesc 에 값 할당

	void			Update_GoinDisable(_float fTimeDelta);



private:
	const _uint		m_iPaletteSizeX			= 10;
	const _uint		m_iPaletteSizeY			= 8;
	const _uint		m_iNumPalettes			= 80;


	// UI Caching..
	CCustom_UI*		m_pRUI_All				= { nullptr };				//			Sub_All

	CCustom_UI*		m_pUIBackgrounds		= { nullptr };				//			SectorA_Backgrounds
	CCustom_UI*		m_pUIForegrounds		= { nullptr };				//			SectorA_Foregrounds
	CCustom_UI*		m_pUISideThings			= { nullptr };				//			SectorA_SideThings
	CCustom_UI*		m_pUIOthers				= { nullptr };				//			SectorA_Others

	CCustom_UI*		m_pUI_InstBlocks		= { nullptr };				// [FG]		FG_InstBlocks
	CCustom_UI*		m_pUI_InstHoverBlocks	= { nullptr };				// [FG]		FG_InstHoverBlocks
	CCustom_UI*		m_pUI_InstColorBtns		= { nullptr };				// [Side]	Side_ColorButton


private:
	array<_float4, 5>						m_arrColors = {};
	array<array<UI_PALETTE_DESC, 10>, 8>	m_arrPalettesInfo = {};		
	vector<vector<UI_PALETTE_DESC>>			m_vecTargetsQueue = {};



private:
	// local variables for shader.
	PALETTE_COLOR				m_eDestColorIndex	= PALETTE_COLOR::PCOLOR_END;
	_float2						m_vChangeStartPos	= {};
	_bool						m_isChanging		= false;
	_float						m_fChangeRadius		= 0.f;
	
	// local variables for gameplay.
	_uint						m_iLeftChance		= 0;
	_uint						m_iMaxChance		= 0;

	// local variables for animation.
	_bool						m_isGoinChange		= false;

	// local variables for fade-out
	_bool						m_IsGoinDisabled	= false;
	_float						m_fDisableTimer		= { };
	_uint						m_iAnimOrder		= { };

	class CGameSystem*			m_pGameSystem		= { nullptr };

public:
	static CUI_Ovfl_Palette*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END