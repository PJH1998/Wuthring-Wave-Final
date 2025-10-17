#pragma once

#include "Client_Define.h"
#include "UIObject.h"



NS_BEGIN(Engine)
class CShader;
class CTexture;
//class CVIBuffer_Rect; 
class CVIBuffer;		// 나중에 인스턴스같은거 쓸 수 있으므로, 유동 선택 가능하도록?
NS_END

NS_BEGIN(Client)
class CAnimator_UI;
NS_END

NS_BEGIN(Client)

class CCustom_UI abstract : public CUIObject
{
public:
	enum class UI_TYPE {
		NONE, BUTTON, INTERACT, END // TEXT, FROM3D 등도 필요 
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

protected:
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
	CCustom_UI*				Find_ChildObject(_wstring strChildName);
	

public:
	CUSTOM_UI_DESC			Get_UIDesc()						{ return m_tUIDesc; }
	void					Set_UIDesc(CUSTOM_UI_DESC tUIDesc)	{ m_tUIDesc = tUIDesc; }
	void					Set_CurTexIndex(_uint iIndex)		{ m_iCurTexIndex = iIndex; };

protected:
	HRESULT					Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Bind_Description(void* pArg);

	void					Update_CombinedMatrix(_matrix* pParentMatrix);

protected:
	CShader*				m_pShaderCom = { nullptr };
	CVIBuffer*				m_pVIBufferCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };
	CAnimator_UI*			m_pAnimator_UICom = { nullptr };


	CUSTOM_UI_DESC			m_tUIDesc = {};
	_uint					m_iCurTexIndex = {};

	_float4x4				m_CombinedWorldMatrix = {};
	vector<CCustom_UI*>		m_vecChildObjects = {};

	//std::function


public:
	virtual CGameObject*	Clone(void* pArg) = 0;
	virtual void			Free() override;
};

NS_END





inline void from_json(const json& j, CCustom_UI::CUSTOM_UI_DESC& d)
{
	_string strFilePath = j["strFilePath"].get<_string>();
	d.strFilePath = _wstring(strFilePath.begin(), strFilePath.end());
	_string strFileName = j["strFileName"].get<_string>();
	d.strFileName = _wstring(strFileName.begin(), strFileName.end());
	d.iNumFiles = j["iNumFiles"];

	_string strUIName = j["strUIName"].get<_string>();
	d.strUIName = _wstring(strUIName.begin(), strUIName.end());
	d.iUIType = j["iUIType"];
	_string strParentName = j["strParentName"].get<_string>();
	d.strParentName = _wstring(strParentName.begin(), strParentName.end());

	for (const auto& element : j["vecChildNames"])
	{
		_string strChildName = element.get<_string>();
		d.vecChildNames.push_back(_wstring(strChildName.begin(), strChildName.end()));
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

