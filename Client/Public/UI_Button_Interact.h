#pragma once
#include "UI_Button.h"

NS_BEGIN(Client)

class CUI_Button_Interact final : public CUI_Button
{
public:
	explicit CUI_Button_Interact(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_Button_Interact(const CUI_Button_Interact& Prototype);
	virtual ~CUI_Button_Interact() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;
public:
	void			Req_OffInteract()	{ m_IsGoindDisabled = true; }

private:
	void			PreAssign_ChildUIs();

	void			Update_MouseFeedback(_float fTimeDelta);
	void			Create_ChildText();

private:
	CCustom_UI*		m_pRUI_Interact_Multiplier = nullptr;
	CCustom_UI*		m_pUI_Interact_Focused = nullptr;
	CCustom_UI*		m_pUI_Interact_Pressed = nullptr;
	CCustom_UI*		m_pUI_Interact_Normal = nullptr;


	_bool			m_IsGoindDisabled = false;
	_float			m_fDisableTimer = 0.f;

	_uint			m_iAnimOrder = 0;

	class CGameSystem*	m_pGameSystem = { nullptr };

public:
	static CUI_Button_Interact* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END