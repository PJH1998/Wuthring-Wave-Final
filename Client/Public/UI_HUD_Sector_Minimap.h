#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_HUD_Sector_Minimap final : public CCustom_UI
{
public:
	typedef struct tHUDMinimapUIIDesc {
		//?
	} UI_HUD_MINIMAP_DESC;

private:
	typedef struct tUIMinimapObjDesc {
		UI_MINIMAP_OBJTYPE	eType = UI_MINIMAP_OBJTYPE::END;
		_float3				vTargetPos = {};
	} UI_MINIMAP_OBJ_DESC;

public:
	explicit CUI_HUD_Sector_Minimap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_HUD_Sector_Minimap(const CUI_HUD_Sector_Minimap& Prototype);
	virtual ~CUI_HUD_Sector_Minimap() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

public:
	void			Bind_ObjectPos_PerFrame(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType);
	void			Attach_ObjectPos(const _float3& vPosition, UI_MINIMAP_OBJTYPE eType, void* pOwner);	// 객체 저장 방식 사용 시에 객체의 고유 key 필요
	void			Detach_ObjectPos(void* pOwner);

private:
	HRESULT			Ready_Components(void* pArg);

	void			PreAssign_ChildUIs();
	void			PreAssign_Presets();

private:
	void			Update_TargetDegrees();
	void			Update_RelativePos();

	void			Update_Instances();

private:
	_float2			Calc_RelativePos(_float3* pTargetPos, _float fMultiplierRatio);

private:
	// 매 프레임 돌릴만한 건 캐싱..
	CCustom_UI*			m_pRUI_All					= { nullptr };
	
	CCustom_UI*			m_pUI_SectorLT_IconProps	= { nullptr };
	CCustom_UI*			m_pUI_SectorLT_Minimap		= { nullptr };
	
	CCustom_UI*			m_pUI_InstIcons				= { nullptr };
	CCustom_UI*			m_pUI_InstMinimapBG			= { nullptr };
	CCustom_UI*			m_pUI_InstCamAndPlayer		= { nullptr };

	CCustom_UI*			m_pUI_InstObjectIndicator	= { nullptr };
	
	//CCustom_UI*			m_pUI_LT_Minimap_StaticBG	= { nullptr };
	//CCustom_UI*			m_pUI_LT_Minimap_TurnPoint	= { nullptr };



private:
	_float				m_fCamDirDegree			= 0;
	_float				m_fPlayerDirDegree		= 0;
	
	vector<_float3>		m_vecTmpRelativeObjects = {};
	vector<_float2>		m_vecTmpCacledRelativeObjects = {};
	

	vector<UI_MINIMAP_OBJ_DESC>					m_vecObjectPos_PerFrame = {};
	unordered_map<void*, UI_MINIMAP_OBJ_DESC>	m_mapObjectPos_Attached = {};



	class CGameSystem*	m_pGameSystem		= { nullptr };

public:
	static CUI_HUD_Sector_Minimap*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*				Clone(void* pArg) override;
	virtual void						Free() override;
};

NS_END