#pragma once
#include "Custom_UI.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CUI_Loading final : public CCustom_UI
{
private:
	explicit				CUI_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CUI_Loading(const CUI_Loading& Prototype);
	virtual					~CUI_Loading() = default;

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
	_uint					m_iRandomBGIndex = 0;

public:
	static CUI_Loading*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END