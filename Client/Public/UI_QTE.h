#pragma once
#include "UI_Image.h"

NS_BEGIN(Client)

class CUI_QTE final : public CUI_Image
{
public:
	typedef struct tUIQTEDesc {
		_float2		vSpawnPos	= _float2(0.f, 0.f);
		UI_QTE_BTN	eIconIndex	= UI_QTE_BTN::F;
		UI_QTE_TYPE	eQTEType	= UI_QTE_TYPE::FILLGUAGE;
	} UI_QTE_DESC;

public:
	explicit CUI_QTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_QTE(const CUI_QTE& Prototype);
	virtual ~CUI_QTE() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

private:
	void			PreAssign_ChildUIs();
	array<_float2, 2> Calc_SpriteSpace(_uint iIndexX, _uint iIndexY, array<_uint, 2> iNumMax, _float2 vSpriteSize = { 1.f, 1.f });

private:
	void			Update_AnimOrder(_float fTimeDelta);
	void			Update_Instances(_float fTImeDelta);

private:
	void			Update_QTE_Fillguage(_float fTimeDelta);
	void			Update_QTE_Trigger(_float fTimeDelta);
	void			Update_FinishEvent(_float fTimeDelta);
	void			Update_GoinDisabled(_float fTimeDelta);
		
private:
	//CCustom_UI* m_pRUI? = ;
	CCustom_UI*		m_pRUI_All							= nullptr;
	CCustom_UI*		m_pUI_SectorA_KeyGuide				= nullptr;
	CCustom_UI*		m_pUI_SectorA_BG					= nullptr;
	CCustom_UI*		m_pUI_SectorA_FG_Fillguage			= nullptr;
	CCustom_UI*		m_pUI_SectorA_FG_Trigger			= nullptr;

	CAnimator_UI*	m_pAnim_RUI_All						= nullptr;
	CAnimator_UI*	m_pAnim_UI_SectorA_KeyGuide			= nullptr;
	CAnimator_UI*	m_pAnim_UI_SectorA_BG				= nullptr;
	CAnimator_UI*	m_pAnim_UI_SectorA_FG_Fillguage		= nullptr;
	CAnimator_UI*	m_pAnim_UI_SectorA_FG_Trigger		= nullptr;

	CCustom_UI*		m_pUI_KeyButtons					= nullptr;
	CCustom_UI*		m_pUI_BG_QTEFrame					= nullptr;
	CCustom_UI*		m_pUI_AbilityIconBG					= nullptr;
	CCustom_UI*		m_pUI_AbilityIcons					= nullptr;
	CCustom_UI*		m_pUI_FG_QTEFeedbackRing			= nullptr;
	CCustom_UI*		m_pUI_BG_QTEAssemble				= nullptr;
	CCustom_UI*		m_pUI_FG_QTEGuageFrame				= nullptr;
	CCustom_UI*		m_pUI_FG_QTEGuage					= nullptr;
	CCustom_UI*		m_pUI_FG_QTEArrow					= nullptr;
	CCustom_UI*		m_pUI_FG_Trigger					= nullptr;

	CAnimator_UI*	m_pAnim_UI_BG_QTEAssemble			= nullptr;
	CAnimator_UI*	m_pAnim_UI_FG_QTEArrow				= nullptr;
	CAnimator_UI*	m_pAnim_UI_FG_QTEFeedbackRing		= nullptr;

	array<array<_float2, 2>, ENUM_CLASS(UI_QTE_BTN::END)>		m_arrBtnPresets = {};			// image UV
	array<_ubyte, ENUM_CLASS(UI_QTE_BTN::END)>					m_arrBtnMapping = { DIK_F, DIK_E, DIK_Q, DIK_R, DIK_T };
private:
	UI_QTE_BTN		m_eIconIndex		= UI_QTE_BTN::F;
	UI_QTE_TYPE		m_eQTEType			= UI_QTE_TYPE::FILLGUAGE;	// 이거 따라 분기 나눠야함


private:
	_float			m_fQTEDropRate		= 0.25f;	// 초당 떨어지는 정도.
	_float			m_fQTEFillAmount	= 0.1f;		// 조작 1회 당 차는 정도
	_float			m_fQTEMaxTime		= 3.f;		// QTE 제한시간.

	_float			m_fQTEGuage			= 0.f;		// 0 ~ 1
	_float			m_fQTEElapsedTime	= 0.f;
	_bool			m_isQTEMode			= false;

	_bool			m_isGoinSuccess		= false;
	_bool			m_isGoinFail		= false;
	_uint			m_iAnimOrder		= 0;
	_float			m_fElapsedTime		= 0.f;

	// 1. 켜지고
	// 2. 애니메이션 돌고 난 뒤,
	// 3. qte 모드. 남은 시간동안 키 누르면 게이지 차고, 지속적으로 게이지가 감소해야됨

	_bool			m_IsGoinDisabled	= false;

	_float			m_fDisableTimer		= 0.f;
	_uint			m_iDisableAnimOrder = 0;

	class CGameSystem* m_pGameSystem = { nullptr };

public:
	static CUI_QTE*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END