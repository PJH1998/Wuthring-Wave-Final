#pragma once
#include "UI_Image.h"


NS_BEGIN(Client)

class CUI_FinalEnd final : public CUI_Image
{
public:
	explicit CUI_FinalEnd(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_FinalEnd(const CUI_FinalEnd& Prototype);
	virtual ~CUI_FinalEnd() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

public:
	void			Trigger_PlayEndImage(_bool isOn)	{	m_isActivate	= isOn; 
															m_isStart		= isOn;		
															m_fElapsedTime	= 0.f;
															m_iAnimOrder	= 0; };

private:
	HRESULT			Ready_Components(void* pArg);
	void			Ready_Presets();
	void			PreAssign_ChildUIs();

private:
	void			Update_AnimOrder(_float fTimeDelta);

private:
	CCustom_UI*		m_pRUI_All				= { nullptr };

	CCustom_UI*		m_pUI_MainImage			= { nullptr };
	CCustom_UI*		m_pUI_FadeAll			= { nullptr };
	CCustom_UI*		m_pUI_FadeGrad			= { nullptr };

	CAnimator_UI*	m_pUIAnim_MainImage		= { nullptr };		
	CAnimator_UI*	m_pUIAnim_FadeAll		= { nullptr };		
	CAnimator_UI*	m_pUIAnim_FadeGrad		= { nullptr };		

private:
	class CGameSystem*	m_pGameSystem		= { nullptr };

private:
	_bool			m_isStart = false;

	_uint			m_iAnimOrder = 0;
	_float			m_fElapsedTime = 0.f;

public:
	static CUI_FinalEnd* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END