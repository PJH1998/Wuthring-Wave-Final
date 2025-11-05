#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_Text final : public CCustom_UI
{
public:
	typedef struct tagUITextDesc : public CUSTOM_UI_DESC {

		_wstring strFontTag = {};

	} TEXT_UI_DESC;


private:
	explicit				CUI_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CUI_Text(const CUI_Text& Prototype);
	virtual					~CUI_Text() = default;

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

	vector<ID3D11ShaderResourceView*>	m_SRVs;
	_uint								m_iNumTextures = {};


public:
	static CUI_Text*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END
