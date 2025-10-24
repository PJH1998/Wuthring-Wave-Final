#pragma once
#include "Custom_UI.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CUI_Button final : public CCustom_UI
{
private:
	explicit				CUI_Button(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CUI_Button(const CUI_Button& Prototype);
	virtual					~CUI_Button() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

private:
	HRESULT					Ready_Components(void* pArg);

private:

public:
	static CUI_Button*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END