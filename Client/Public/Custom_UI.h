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

		_bool		isInverseScreenDiscard = false;	// 그릴 구역 반전
		_float		fCutout = 0.3f;					// (1:컷아웃 사용 시) 알파값 기준

		_uint		iPassType = 2;			// 0 : Normal, 1 : Cutout, 2 : Transparent, 3 : SimpleGradient

		vector<_wstring> vecChildNames = {};

		CGameObject* pParentObject = nullptr;
	} CUSTOM_UI_DESC;



	// for Load

	typedef struct tagUIInfoDesc
	{
		CCustom_UI::CUSTOM_UI_DESC	tUIDesc = {};	// FilePath, FileName, NumTex

		_float3						vPos = {};
		_float3						vRot = {};	// Euler
		_float3						vSca = {};
	} UI_INFO_DESC;

	typedef struct tagCustomUITreeDesc {
		wstring					strTreeName = {};

		vector<UI_INFO_DESC>	vecUIInfoDescs = {};
	} CUSTOM_UITREE_DESC;

	typedef struct tagHierarchyObjectDesc
	{
		_wstring				strObjName = {};
		CCustom_UI*				pCustomUI = nullptr;
	} HIERARCHY_OBJ_DESC;


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

	void					Add_EventFunction(_uint iEventType, function<void()> function);
	void					OnEvent(_uint iEventType);
	
public:
	CUSTOM_UI_DESC			Get_UIDesc()						{ return m_tUIDesc; }
	void					Set_UIDesc(CUSTOM_UI_DESC tUIDesc)	{ m_tUIDesc = tUIDesc; }
	void					Set_CurTexIndex(_uint iIndex)		{ m_iCurTexIndex = iIndex; };

protected:
	//HRESULT					Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Ready_Events();

	HRESULT					Bind_Description(void* pArg);

	void					Update_CombinedMatrix(_matrix* pParentMatrix = nullptr);

protected:
	CShader*				m_pShaderCom			= { nullptr };
	CVIBuffer*				m_pVIBufferCom			= { nullptr };
	CTexture*				m_pTextureCom			= { nullptr };
	CAnimator_UI*			m_pAnimator_UICom		= { nullptr };


	CUSTOM_UI_DESC			m_tUIDesc = {};
	_uint					m_iCurTexIndex = {};

	_float4x4				m_CombinedWorldMatrix = {};
	vector<CCustom_UI*>		m_vecChildObjects = {};

	vector<function<void()>>	m_vecFunctions[ENUM_CLASS(UI_EVENT_TYPE::END)] = {};

	//std::function


public:
	virtual CGameObject*	Clone(void* pArg) = 0;
	virtual void			Free() override;
};

NS_END





inline void from_json(const json& j, CCustom_UI::CUSTOM_UI_DESC& d)
{
	_string strFilePath = j["strFilePath"].get<_string>();
	d.strFilePath = StringToWString(strFilePath);
	_string strFileName = j["strFileName"].get<_string>();
	d.strFileName = StringToWString(strFileName);
	d.iNumFiles = j["iNumFiles"];

	_string strUIName = j["strUIName"].get<_string>();
	d.strUIName = StringToWString(strUIName);
	d.iUIType = j["iUIType"];
	_string strParentName = j["strParentName"].get<_string>();
	d.strParentName = StringToWString(strParentName);

	for (const auto& element : j["vecChildNames"])
	{
		_string strChildName = element.get<_string>();
		d.vecChildNames.push_back(StringToWString(strChildName));
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

// ---- from Level.h

inline void from_json(const json& j, CCustom_UI::UI_INFO_DESC& d)
{
	from_json(j["tUIDesc"], d.tUIDesc);

	d.vPos = { j["vPos"][0], j["vPos"][1], j["vPos"][2] };
	d.vRot = { j["vRot"][0], j["vRot"][1], j["vRot"][2] };
	d.vSca = { j["vSca"][0], j["vSca"][1], j["vSca"][2] };
}

inline void from_json(const json& j, vector<CCustom_UI::UI_INFO_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CCustom_UI::UI_INFO_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}

inline void from_json(const json& j, CCustom_UI::CUSTOM_UITREE_DESC& d)
{
	_string strTreeName = j["strTreeName"].get<_string>();
	d.strTreeName = StringToWString(strTreeName);
	from_json(j["vecUIInfoDescs"], d.vecUIInfoDescs);
}
