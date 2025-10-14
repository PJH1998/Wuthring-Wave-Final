#pragma once

#include "Editor_Define.h"
#include "UIObject.h"



NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Editor)
class CAnimator_UI;
NS_END

NS_BEGIN(Editor)

class CCustom_UI final : public CUIObject
{
public:
	typedef struct tagCustomUIObjectDesc : public CUIObject::UI_DESC {
		_wstring	strFilePath = {};
		_wstring	strFileName = {};
		_uint		iNumFiles = 1;
	} CUSTOM_UI_DESC;

private:
	explicit				CCustom_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CCustom_UI(const CCustom_UI& Prototype);
	virtual					~CCustom_UI() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)					override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

public:
	CUSTOM_UI_DESC			Get_UIDesc()	{ return m_tUIDesc; }
	void					Set_CurTexIndex(_uint iIndex) { m_iCurTexIndex = iIndex; };

private:
	HRESULT					Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Bind_Description(void* pArg);

private:
	CShader*				m_pShaderCom				= { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom				= { nullptr };
	CTexture*				m_pTextureCom				= { nullptr };

	CAnimator_UI*			m_pAnimator_UICom			= { nullptr };

	CUSTOM_UI_DESC			m_tUIDesc					= {};


	_uint					m_iCurTexIndex				= {};

	// 현재 사용중일 텍스쳐 정보, texcoord 값, 나인섹터 기준점 등의 정보.. 필요할수도 있음

public:
	static CCustom_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

NS_END





inline void to_json(json& j, const CCustom_UI::CUSTOM_UI_DESC& d)
{
	j = {
		{ "strFilePath", _string(d.strFilePath.begin(), d.strFilePath.end()) },
		{ "strFileName", _string(d.strFileName.begin(), d.strFileName.end()) },
		{ "iNumFiles", d.iNumFiles }
	};
}

inline void from_json(const json& j, CCustom_UI::CUSTOM_UI_DESC& d)
{
	_string strFilePath = j["strFilePath"].get<_string>();
	d.strFilePath	= _wstring(strFilePath.begin(), strFilePath.end());
	_string strFileName = j["strFileName"].get<_string>();
	d.strFileName   = _wstring(strFileName.begin(), strFileName.end());
	d.iNumFiles		= j["iNumFiles"];
}