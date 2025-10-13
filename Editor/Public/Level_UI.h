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
	typedef struct tagUIAnimDesc
	{
		CCustom_UI::CUSTOM_UI_DESC	tUIDesc = {};	// FilePath, FileName, NumTex (어떤 텍스쳐용인지를 위함)

		// 키프레임, 키프레임별 행렬정보, 보간방법, 길이 등..
		_uint					iNumKeyFrame = {};

		vector<_float3>			vecPos = {};
		vector<_float3>			vecRot = {};	// Euler
		vector<_float3>			vecSca = {};

		_uint					iLerpType = {};
	} UI_AnimDesc;

	
	typedef struct tagUIInfoOutputDesc
	{
		CCustom_UI::CUSTOM_UI_DESC	tUIDesc = {};	// FilePath, FileName, NumTex

		_float3						vPos = {};
		_float3						vRot = {};	// Euler
		_float3						vSca = {};
	} UI_InfoOutputDesc;

private:
	explicit CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_UI() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual void		Render() override;

private:
	void				Update_Picking();

	void				Update_LoadWindow();
	void				Update_Hierarchy();
	
	void				Update_Inspector();
	void				Update_AnimEditor();

private:
	class CGameObject*			m_pCurObj = { nullptr };
	class CGameObject*			m_pPreObj = { nullptr };

	_float3						m_vCurObjPos = {};
	_float3						m_vCurObjRot = {};	// Euler
	_float3						m_vCurObjSca = {};

	// for Update_Hierarchy
	vector<HIERARCHY_OBJ_DESC>	m_vecCustomUIs = {};

	// for Update_AnimEditor
	_bool						m_isOn_AnimEdit = false;
	vector<UI_AnimDesc>			m_vecUIAnims = {};

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

inline void to_json(json& j, const CLevel_UI::UI_AnimDesc& d)
{
	j = json{
		{ "iNumKeyFrame", d.iNumKeyFrame },
		{ "vecPos", vec_to_json(d.vecPos) },
		{ "vecRot", vec_to_json(d.vecRot) },
		{ "vecSca", vec_to_json(d.vecSca) },
		{ "iLerpType", d.iLerpType }
	};
}

inline void to_json(json& j, const CLevel_UI::UI_InfoOutputDesc& d)
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