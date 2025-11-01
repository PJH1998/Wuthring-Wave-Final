#pragma once

#include "Client_Define.h"
#include "UIObject.h"

#include "VIBuffer_Rect_Instance_UI.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
class CVIBuffer_Rect_Instance_UI;
NS_END

NS_BEGIN(Client)
class CAnimator_UI;
NS_END

NS_BEGIN(Client)

class CCustom_UI abstract : public CUIObject
{
#pragma region enum class & structs

public:
	enum class UI_TYPE {
		NONE, BUTTON, INTERACT, END
	};

	typedef struct tagCustomUISizeDesc {
		vector<_float2>	vecSize = {};
	} UI_SIZE_DESC;

	typedef struct tagCustomUISectorDesc {
		_float2		vSectorBorder = {}; // pixel
		_float		fUIScale = {};		// ui ����
	} UI_SECTOR_DESC;

	typedef struct tagCustomUIObjectDesc : public CUIObject::UI_DESC, UI_SIZE_DESC, UI_SECTOR_DESC {
		_wstring	strFilePath = {};
		_wstring	strFileName = {};
		_uint		iNumFiles = 1;

		_wstring	strUIName = {};
		_uint		iUIType = {};
		_wstring	strParentName = {};

		_bool		isInverseScreenDiscard = false;
		_float		fCutout = 0.3f;
		_uint		iPassType = 2;

		vector<_wstring> vecChildNames = {};
		CGameObject* pParentObject = nullptr;


		_bool		isInstance = false;
		vector<CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC> vecInstanceDescs = {};
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




	// for Variable Shader
	// 특수한 상황 (pass 5번) 에 비정형 값을 던져주어야 할 때 사용.
	typedef struct tagVariantUIReadyDesc
	{
		vector<_float4x4>	matVariantValues = {};	// per instance
		_uint		iShaderFlag = {};
		_bool		isVariant = false;
	} VARIANTREADY_UI_DESC;
	 

#pragma endregion

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
	void					OnEvent(_uint iEventType)				override;

	_bool					Check_OnInteract(_uint iEventInteractType, _uint iInstanceIndex = 0)	override;
	_bool					Check_OnInteract(_wstring strChildName, _uint iEventInteractType, _uint iInstanceIndex = 0);
	
public:
	void					Update_CombinedMatrix(_matrix* pParentMatrix = nullptr);
private:
	void					Update_CacheTransform(_float fTimeDelta);
	void					Update_InputState()						override;

public:
	CUSTOM_UI_DESC			Get_UIDesc()						{ return m_tUIDesc; }
	void					Set_UIDesc(CUSTOM_UI_DESC tUIDesc)	{ m_tUIDesc = tUIDesc; }
	void					Set_VariantUIDesc(VARIANTREADY_UI_DESC tVarUIDesc)	{ 
		m_cachedVariantUIDesc = tVarUIDesc;  
	}
	void					Set_CurTexIndex(_uint iIndex)		{ m_iCurTexIndex = iIndex; };

	void					Add_Child(CCustom_UI* pChildUI)		{ m_vecChildObjects.push_back(pChildUI); }

protected:
	//HRESULT				Ready_Prototypes(void* pArg);
	HRESULT					Ready_Components(void* pArg);
	HRESULT					Ready_Events();
	HRESULT					Bind_Description(void* pArg);
	
	_bool					Check_IsInSpace()						override;

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


protected:	// UI 인식의 기준이 되는 좌표를 낮은 프레임으로 캐싱하여 그것을 사용. HOVER 등의 비용을 낮추기 위함
	_float					m_cachingTimeElapsed = {};
	enum CACHED_TRANSFORM {POS, ROT, SCA, END};
	vector<array<_float4, CACHED_TRANSFORM::END>>	m_vecCachedUITransform = {};

	_uint					m_iInputState = ENUM_CLASS(UI_EVENT_TYPE::NONE);
	_uint					m_iInputInstanceIndex = 0;

	VARIANTREADY_UI_DESC	m_cachedVariantUIDesc = {};

public:
	virtual CGameObject*	Clone(void* pArg) = 0;
	virtual void			Free() override;
};

NS_END


#pragma region json

inline void from_json(const json& j, CCustom_UI::CUSTOM_UI_DESC& d)
{
#pragma region old
	//_string strFilePath = j["strFilePath"].get<_string>();
	//d.strFilePath = StringToWString(strFilePath);
	//_string strFileName = j["strFileName"].get<_string>();
	//d.strFileName = StringToWString(strFileName);
	//d.iNumFiles = j["iNumFiles"];
	//
	//_string strUIName = j["strUIName"].get<_string>();
	//d.strUIName = StringToWString(strUIName);
	//d.iUIType = j["iUIType"];
	//_string strParentName = j["strParentName"].get<_string>();
	//d.strParentName = StringToWString(strParentName);
	//
	//for (const auto& element : j["vecChildNames"])
	//{
	//	_string strChildName = element.get<_string>();
	//	d.vecChildNames.push_back(StringToWString(strChildName));
	//}
#pragma endregion

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


	d.isInverseScreenDiscard= j["isInverseScreenDiscard"];
	d.fCutout				= j["fCutout"];
	d.iPassType				= j["iPassType"];

	for (const auto& element : j["vecChildNames"])
	{
		_string strChildName = element.get<_string>();
		d.vecChildNames.push_back(StringToWString(strChildName));
	}
	d.isInstance = j["isInstance"];

	for (const auto& element : j["vecInstanceDescs"])
	{
		Client::CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC tDesc = {};
		from_json(element, tDesc);
		d.vecInstanceDescs.push_back(tDesc);
	}

	// descs
	for (const auto& element : j["vecSize"])
		d.vecSize.push_back(_float2{ element[0], element[1] });
	d.vSectorBorder			= _float2( j["vSectorBorder"][0], j["vSectorBorder"][1] );
	d.fUIScale				= j["fUIScale"];

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

#pragma endregion


