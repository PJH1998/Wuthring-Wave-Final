#pragma once
#include "Custom_UI.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CUI_Logo final : public CCustom_UI
{
private:
	explicit				CUI_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CUI_Logo(const CUI_Logo& Prototype);
	virtual					~CUI_Logo() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

private:
	HRESULT					Ready_Components(void* pArg);
	void					Create_ChildText();

private:
	void					Update_AnimControl(_float fTimeDelta);

private:
	_float					m_fTimeElapsed = 0.f;
	_uint					m_iAnimOrder = 0;

	class CGameSystem*		m_pGameSystem = { nullptr };

public:
	static CUI_Logo*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END