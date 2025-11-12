#pragma once
#include "UI_Button.h"
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

private:
	// 상호작용 시에 어떻게함?
	void			Update_MouseFeedback(_float fTimeDelta);

	_bool			m_IsGoindDisabled = false;
	_float			m_fDisableTimer = 0.f;

	_uint			m_iAnimOrder = 0;

public:
	static CUI_Button_Interact* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void            Free() override;
};
