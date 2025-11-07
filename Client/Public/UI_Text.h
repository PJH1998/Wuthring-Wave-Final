#pragma once
#include "Custom_UI.h"

NS_BEGIN(Client)

class CUI_Text final : public CCustom_UI
{
public:
	typedef struct tagUITextDesc : public CUSTOM_UI_DESC, FONT_SINGLEDESC {// 근데 당장에 Font_SingleDesc 반영하는 코드가 있긴함?


	} TEXT_UI_DESC;
	// 그러면.. 셰이더 내에서 패스나 분기는 따로 받아 사용하고
	// 
	// CUSTOM_UI_DESC 는 단순히 커스텀 행렬 저장용으로써 사용,
	// FONT_SINGLEDESC 가 기존에 CUSTOM_UI_DESC 가 넘겨줬어야 할 기본 정보들을 넘기는 식으로?

	// 근데 그러면 CUSTOM_UI_DESC 가 넘겨야 할 정보는 어떻게 넘김? 그냥 렌더단에서 넘길 수 있었네. 그럼 그냥 커스텀 정보 넘김녀 될 듯.
	// 행렬은 공통 정보가 아닌 인스턴스용 정보임. 이는 명확히 하고

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
	HRESULT					Bind_Description(void* pArg);

	void					Update_Description();


private:

	vector<ID3D11ShaderResourceView*>	m_SRVs;
	_uint								m_iNumTextures = {};

	TEXT_UI_DESC						m_tTextDesc = {};


public:
	static CUI_Text*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END
