#pragma once
#include "UI_Text.h"

class CUI_Text_Damage final : public CUI_Text
{
public:
	typedef struct tagUITextTimedDesc : public TEXT_UI_DESC {

	} TEXT_UI_TIMED_DESC;
public:
	explicit CUI_Text_Damage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_Text_Damage(const CUI_Text_Damage& Prototype);
	virtual ~CUI_Text_Damage() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;


	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;


private:
	void            Update_LifeTime(_float fTimeDelta);           // 내부용
	void			Update_Instances(_float fTimeDelta);

private:
	_float          m_fLifeTime = 0.f;
	_float          m_fLifeElapsed = 0.f;
	_bool           m_isAutoDeactivate = true;

	_uint			m_iNumText = 0;

public:
	static CUI_Text_Damage* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void            Free() override;
};
