#pragma once
#include "Level.h"
#include "Custom_UI.h"

NS_BEGIN(Editor)

class CLevel_UI final : public CLevel
{
public:
	// for Editor
	typedef struct tagHierarchyObjectDesc
	{
		_wstring				strObjName	= {};
		CCustom_UI*				pCustomUI	= nullptr;
	} HIERARCHY_OBJ_DESC;


	// for Output
	// 나중에 *_Struct.h 로 옮겨야 할 듯?
	typedef struct tagUIAnimKeyFrameDesc
	{
		_uint			iKeyframeIndex = {};			// 정보가 담길 키프레임 정보

		_uint			iTexIndex = {};
		_float			fAlpha = {};			// 0 ~ 1
		_float3			vPos = {};
		_float3			vRot = {};			// Euler
		_float3			vSca = {};

	} UI_ANIM_KEYFRAME_DESC;

	typedef struct tagUIAnimDesc
	{
		CCustom_UI::CUSTOM_UI_DESC	tUIDesc = {};	// FilePath, FileName, NumTex (어떤 텍스쳐용인지를 위함)

		// 키프레임, 키프레임별 행렬정보, 보간방법, 길이 등..
		_wstring				strAnimName = {};
		//_uint					iNumKeyFrame = {};

		vector<UI_ANIM_KEYFRAME_DESC> vecKeyFrames = {};

		_uint					iLerpType = {};
		_bool					isLoop = false;
	} UI_ANIM_DESC;

	
	typedef struct tagUIInfoDesc
	{
		CCustom_UI::CUSTOM_UI_DESC	tUIDesc = {};	// FilePath, FileName, NumTex

		_float3						vPos = {};
		_float3						vRot = {};	// Euler
		_float3						vSca = {};
	} UI_INFO_DESC;

private:
	explicit CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_UI() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Render() override;

private:
	void				Update_Picking();

	void				Update_MenuWindow();
	void				Update_Hierarchy();
	void				Update_Hierarchy_CheckTree(CCustom_UI* pParentUI, ImGuiTreeNodeFlags flags);
   
	void				Update_SaveLoad();
	void				Update_Inspector();
	void				Update_AnimEditor(_float fTimeDelta);

private:
	class CGameObject*			m_pCurObj = { nullptr };
	class CGameObject*			m_pPreObj = { nullptr };

	_float3						m_vCurObjPos = {};
	_float3						m_vCurObjRot = {};	// Euler
	_float3						m_vCurObjSca = {};



	// for Update_SaveLoad
	_bool						m_isOn_SaveLoad = false;


	// for Update_Hierarchy
	vector<HIERARCHY_OBJ_DESC>	m_vecCustomUIs = {};


	// for Update_AnimEditor
	// ㄴAnimEditor
	_bool						m_isOn_AnimEdit = false;
	vector<UI_ANIM_DESC>		m_vecUIAnims = {};

	vector<UI_ANIM_KEYFRAME_DESC> m_vecUIKeyFrameDescs = {};	// Temp Keyframe
	UI_ANIM_KEYFRAME_DESC*		m_pSelectedKeyFrameDesc = { nullptr };
	_int						m_iLerpType = 0;
	_bool						m_isAnimLoop = true;

	// ㄴAnimList
	_bool						m_isPlayAnimation = false;

	UI_ANIM_DESC*				m_pSelectedUIAnim = { nullptr };


public:
	static CLevel_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;
};

NS_END


inline json vec_to_json(const std::vector<_float3>& vec)
{
	json j = json::array();
	for (const auto& v : vec)
		j.push_back({ {"x", v.x}, {"y", v.y}, {"z", v.z} });
	return j;
}


inline void to_json(json& j, const CLevel_UI::UI_ANIM_KEYFRAME_DESC& d)
{
	j = json{
		{ "iKeyframeIndex", d.iKeyframeIndex },

		{ "iTexIndex", d.iTexIndex },
		{ "fAlpha", d.fAlpha },
		{ "vecPos", { d.vPos.x, d.vPos.y, d.vPos.z } },
		{ "vecRot", { d.vRot.x, d.vRot.y, d.vRot.z } },
		{ "vecSca", { d.vSca.x, d.vSca.y, d.vSca.z } },
	};
}

inline void from_json(const json& j, CLevel_UI::UI_ANIM_KEYFRAME_DESC& d)
{
	d.fAlpha			= j["fAlpha"];
	d.iKeyframeIndex	= j["iKeyframeIndex"];
	d.iTexIndex			= j["iTexIndex"];

	d.vPos = { j["vecPos"][0], j["vecPos"][1], j["vecPos"][2] };
	d.vRot = { j["vecRot"][0], j["vecRot"][1], j["vecRot"][2] };
	d.vSca = { j["vecSca"][0], j["vecSca"][1], j["vecSca"][2] };
}

inline void to_json(json& j, const vector<CLevel_UI::UI_ANIM_KEYFRAME_DESC>& vec)
{
	j = json::array();
	for (const auto& v : vec)
	{
		json data = {};
		to_json(data, v);
		j.push_back(data);
	}
}

inline void from_json(const json& j, vector<CLevel_UI::UI_ANIM_KEYFRAME_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CLevel_UI::UI_ANIM_KEYFRAME_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}

inline void to_json(json& j, const CLevel_UI::UI_ANIM_DESC& d)
{
	json vecKeyFrames = {};
	to_json(vecKeyFrames, d.vecKeyFrames);

	json j_tUIDesc = {};
	to_json(j_tUIDesc, d.tUIDesc);

	j = json{
		{ "tUIDesc", j_tUIDesc },
		{ "strAnimName", _string(d.strAnimName.begin(), d.strAnimName.end()) },
		{ "iNumKeyFrame", d.vecKeyFrames.size()},
		{ "vecKeyFrames", vecKeyFrames },
		{ "iLerpType", d.iLerpType },
		{ "isLoop", d.isLoop }
	};
}

inline void from_json(const json& j, CLevel_UI::UI_ANIM_DESC& d)
{
	from_json(j["vecKeyFrames"], d.vecKeyFrames);
	from_json(j["tUIDesc"], d.tUIDesc);

	d.iLerpType		= j["iLerpType"];
	d.isLoop		= j["isLoop"];
	_string strAnimName = j["strAnimName"].get<_string>();
	d.strAnimName	= _wstring(strAnimName.begin(), strAnimName.end());
}

inline void to_json(json& j, const CLevel_UI::UI_INFO_DESC& d)
{
	json j_tUIDesc = {};
	to_json(j_tUIDesc, d.tUIDesc);

	j = json{
		{ "tUIDesc", j_tUIDesc },
		{ "vPos", {d.vPos.x, d.vPos.y, d.vPos.z} },
		{ "vRot", {d.vRot.x, d.vRot.y, d.vRot.z} },
		{ "vSca", {d.vSca.x, d.vSca.y, d.vSca.z} }
	};
}

inline void from_json(const json& j, CLevel_UI::UI_INFO_DESC& d)
{
	from_json(j["tUIDesc"], d.tUIDesc);

	d.vPos = {j["vPos"][0], j["vPos"][1], j["vPos"][2]};
	d.vRot = {j["vRot"][0], j["vRot"][1], j["vRot"][2]};
	d.vSca = {j["vSca"][0], j["vSca"][1], j["vSca"][2]};
}