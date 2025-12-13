#pragma once
#include "Custom_UI.h"


NS_BEGIN(Client)

class CUI_QuestIndicator final : public CCustom_UI
{

public:
	explicit CUI_QuestIndicator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_QuestIndicator(const CUI_QuestIndicator& Prototype);
	virtual ~CUI_QuestIndicator() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	//virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

public:
	void			Trigger_ActivateQuest()			{ m_isQuestActive = true; }
	void			Trigger_AddQuestProgress();

#ifdef _DEBUG
	void			Trigger_AllReset() {
		m_isStarted = false;			m_isEnded = false;				m_isGoinDisable = false;
		m_isOnEvent_GoinStart = false;	m_isOnEvent_GoinEnd = false;	m_fDisableTimer = 0.f;
		m_fStartTimer = 0.f;			m_fEndTimer = 0.f;				m_iProgress = 0;
		m_iStartEventOrder = 0.f;		m_iEndEventOrder = 0;			m_iAnimOrder = 0;

		m_isActivate = true;
		m_isQuestActive = true;
	}
#endif

private:
	//void			Update_Progress(_float fTimeDelta);
	void			Update_AnimOrder(_float fTimeDelta);
	void			Update_GoinDisable(_float fTimeDelta);

private:
	void			Update_StartEvent(_float fTimeDelta);
	void			Update_EndEvent(_float fTimeDelta);

private:
	HRESULT			Ready_Components(void* pArg);

	void			PreAssign_ChildUIs();
	void			PreAssign_Presets();

	void			Create_ChildText();

private:
	// 매 프레임 돌릴만한 건 캐싱..
	_bool				m_isQuestActive			= false;

	CCustom_UI*			m_pRUI_All				= { nullptr };
	CCustom_UI*			m_UI_BG					= { nullptr };
	CCustom_UI*			m_UI_Noti				= { nullptr };
	CCustom_UI*			m_UI_Comp				= { nullptr };
	CCustom_UI*			m_UI_Side				= { nullptr };

	CAnimator_UI*		m_AnimUI_BG				= { nullptr };
	CAnimator_UI*		m_AnimUI_Noti			= { nullptr };
	CAnimator_UI*		m_AnimUI_Comp			= { nullptr };
	CAnimator_UI*		m_AnimUI_Side			= { nullptr };

	CCustom_UI*			m_pTextUI_SideTitle		= { nullptr };
	CCustom_UI*			m_pTextUI_SideDesc		= { nullptr };
	CCustom_UI*			m_pTextUI_SideDesc2		= { nullptr };

	CCustom_UI*			m_pTextUI_NotiTitle		= { nullptr };
	CCustom_UI*			m_pTextUI_NotiDesc		= { nullptr };
	CCustom_UI*			m_pTextUI_CompTitle		= { nullptr };

private:
	_bool				m_isStarted = false;
	_bool				m_isOnEvent_GoinStart = false;
	_float				m_fStartTimer = 0.f;
	_uint				m_iStartEventOrder = 0;

	_bool				m_isEnded = false;
	_bool				m_isOnEvent_GoinEnd = false;
	_float				m_fEndTimer = 0.f;
	_uint				m_iEndEventOrder = 0;


	_bool				m_isGoinDisable = false;
	_float				m_fDisableTimer = 0.f;
	const _float		m_fDisableTime = (30.f) * 1.f / 60.f;
	_uint				m_iAnimOrder = 0;

private:
	_uint				m_iProgress = 0;
	const _uint			m_iMaxProgress = 7;

private:
	class CGameSystem*	m_pGameSystem = { nullptr };

public:
	static CUI_QuestIndicator*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END
