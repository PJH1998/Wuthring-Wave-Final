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
	enum class UI_TYPE {
		NONE, BUTTON, INTERACT, END
	};

	typedef struct tagCustomUIObjectDesc : public CUIObject::UI_DESC {
		_wstring	strFilePath = {};
		_wstring	strFileName = {};
		_uint		iNumFiles = 1;

		_wstring	strUIName = {};
		_uint		iUIType = {};			// 단순 창인지, 버튼인지, 최상위 구현부인지 구분?
		_wstring	strParentName = {};

		vector<_wstring> vecChildNames = {};

		CGameObject* pParentObject = nullptr;
	} CUSTOM_UI_DESC;



private:
	explicit				CCustom_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CCustom_UI(const CCustom_UI& Prototype);
	virtual					~CCustom_UI() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;

public:
	CUSTOM_UI_DESC			Get_UIDesc()						{ return m_tUIDesc; }
	void					Set_UIDesc(CUSTOM_UI_DESC tUIDesc)	{ m_tUIDesc = tUIDesc; }
	void					Set_CurTexIndex(_uint iIndex)		{ m_iCurTexIndex = iIndex; };

private:
	HRESULT					Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Bind_Description(void* pArg);

	void					Update_CombinedMatrix();

private:
	CShader*				m_pShaderCom				= { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom				= { nullptr };
	CTexture*				m_pTextureCom				= { nullptr };

	CAnimator_UI*			m_pAnimator_UICom			= { nullptr };

	CUSTOM_UI_DESC			m_tUIDesc					= {};
	_uint					m_iCurTexIndex				= {};

	_float4x4				m_CombinedWorldMatrix		= {};


	// 현재 사용중일 텍스쳐 정보, texcoord 값, 나인섹터 기준점 등의 정보.. 필요할수도 있음

public:
	static CCustom_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END





inline void to_json(json& j, const CCustom_UI::CUSTOM_UI_DESC& d)
{
	json childNames = json::array();
	for (const auto& v : d.vecChildNames)
	{
		json data = {};
		to_json(data, WStringToString(v));
		childNames.push_back(data);
	}

	j = {
		{ "strFilePath", WStringToString(d.strFilePath) },
		{ "strFileName", WStringToString(d.strFileName) },
		{ "iNumFiles", d.iNumFiles },

		{ "strUIName",  WStringToString(d.strUIName) },
		{ "iUIType", d.iUIType },
		{ "strParentName", WStringToString(d.strParentName) },

		{ "vecChildNames", childNames }
	};
}

inline void from_json(const json& j, CCustom_UI::CUSTOM_UI_DESC& d)
{
	_string strFilePath		= j["strFilePath"].get<_string>();
	d.strFilePath			= StringToWString(strFilePath);
	_string strFileName		= j["strFileName"].get<_string>();
	d.strFileName			= StringToWString(strFileName);
	d.iNumFiles				= j["iNumFiles"];

	_string strUIName		= j["strUIName"].get<_string>();
	d.strUIName				= StringToWString(strUIName);
	d.iUIType				= j["iUIType"];
	_string strParentName	= j["strParentName"].get<_string>();
	d.strParentName			= StringToWString(strParentName);

	for (const auto& element : j["vecChildNames"])
	{
		_string strChildName = element.get<_string>();
		d.vecChildNames.push_back(StringToWString(strChildName));
	}
}

inline void to_json(json& j, const vector<CCustom_UI::CUSTOM_UI_DESC>& vec)
{
	j = json::array();
	for (const auto& v : vec)
	{
		json data = {};
		to_json(data, v);
		j.push_back(data);
	}
}

inline void from_json(const json& j, vector<CCustom_UI::CUSTOM_UI_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CCustom_UI::CUSTOM_UI_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}

