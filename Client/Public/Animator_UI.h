#pragma once

#include "Component.h"
#include "GameObject.h"
#include "Custom_UI.h"

NS_BEGIN(Client)

class CAnimator_UI final : public CComponent
{
public:
	enum class UI_LERPTYPE
	{
		LINEAR, LT, RB, CUBIC, END
	};

	typedef struct tagAnimatorUIDesc
	{
		CCustom_UI*		pOwner = nullptr;
	} ANIMATOR_UI_DESC;

	typedef struct tagUIAnimKeyFrameDesc
	{
		_uint			iKeyframeIndex = {};
		_uint			iLerpType = {};

		_uint			iTexIndex = {};
		_float			fAlpha = {};
		_float3			vPos = {};
		_float3			vRot = {};
		_float3			vSca = {};


		_float2			vScreenLT = {};
		_float2			vScreenRB = { g_iWinSizeX, g_iWinSizeY };

		_float4			vBlendToOuterWidth = {};

	} UI_ANIM_KEYFRAME_DESC;


	typedef struct tagUIAnimDesc
	{
		CCustom_UI::CUSTOM_UI_DESC		tUIDesc = {};
		_wstring						strAnimName = {};

		vector<UI_ANIM_KEYFRAME_DESC>	vecKeyFrames = {};
		_bool							isLoop = false;
	} UI_ANIM_DESC;


	private:
	explicit				CAnimator_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit				CAnimator_UI(const CAnimator_UI& Prototype);
	virtual					~CAnimator_UI() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta);
	virtual void			Update(_float fTimeDelta);
	virtual void			Late_Update(_float fTimeDelta);
	virtual HRESULT			Render()								override;

public:
	HRESULT		Insert_Animation(UI_ANIM_DESC& Desc);
	HRESULT		Remove_Animation(_wstring strAnimName);
	HRESULT		Clear_Animation();

	HRESULT		Change_Animation(_wstring strAnimName);
	HRESULT		Change_Animation(_uint iAnimIndex);
	HRESULT		Deselect_Animation();

	UI_ANIM_DESC* Find_Animation(_wstring strAnimName);
	UI_ANIM_DESC* Find_Animation(_uint iAnimIndex);

	UI_ANIM_DESC* Get_CurAnimation() { return m_pCurAnimDesc; }
	UI_ANIM_KEYFRAME_DESC* Get_CurAnimKeyframeDesc() { return m_pCurKeyFrameDesc; }
	UI_ANIM_KEYFRAME_DESC* Get_CurCombinedAnimKeyframeDesc() { return &m_tCombinedKeyFrameDesc; }

private:
	_float		Fix_LerpRatio(_float fIn, _uint iLerpType);					// Calc_Lerp ���� �����, LerpType�� ���� ���� fIn���� ������ ��ȯ (0 -> 1 �� ���� �׷����� �ȭ)
	_float		Calc_LerpRatio(_float fStart, _float fEnd, _float Ratio);	// ���� �ܼ��� Ratio �� ���� Start�� End ������ ���� ��ȯ

	_float3		Calc_Lerp_Position_CMR(_uint iKeyframeIndex);				// Ű�������� ������ ���� �ִϸ��̼��� ���� position�� �´� ���� ��ȯ (catmull-rom ����)

	void		Update_Animation();

private:
	const _uint					m_iKeyFrameRate = 60;
	vector<UI_ANIM_DESC>		m_vecAnimationDescs = {};
	UI_ANIM_DESC*				m_pCurAnimDesc = { nullptr };

	UI_ANIM_KEYFRAME_DESC*		m_pCurKeyFrameDesc = { nullptr };
	UI_ANIM_KEYFRAME_DESC		m_tCombinedKeyFrameDesc = {};

	CCustom_UI*					m_pOwner = { nullptr };

	_float						m_fElapsedTime = {};

public:
	static CAnimator_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END


#pragma region json

inline void from_json(const json& j, CAnimator_UI::UI_ANIM_KEYFRAME_DESC& d);
inline void from_json(const json& j, vector<CAnimator_UI::UI_ANIM_KEYFRAME_DESC>& vec);
inline void from_json(const json& j, CAnimator_UI::UI_ANIM_DESC& d);

inline void from_json(const json& j, CAnimator_UI::UI_ANIM_KEYFRAME_DESC& d)
{
	d.iKeyframeIndex = j["iKeyframeIndex"];

	d.iLerpType = j["iLerpType"];
	d.iTexIndex = j["iTexIndex"];
	d.fAlpha = j["fAlpha"];

	d.vPos = { j["vecPos"][0], j["vecPos"][1], j["vecPos"][2] };
	d.vRot = { j["vecRot"][0], j["vecRot"][1], j["vecRot"][2] };
	d.vSca = { j["vecSca"][0], j["vecSca"][1], j["vecSca"][2] };

	d.vScreenLT = _float2(j["vScreenLT"][0], j["vScreenLT"][1]);
	d.vScreenRB = _float2(j["vScreenRB"][0], j["vScreenRB"][1]);

	d.vBlendToOuterWidth = _float4(
		j["vBlendToOuterWidth"][0], j["vBlendToOuterWidth"][1],
		j["vBlendToOuterWidth"][2], j["vBlendToOuterWidth"][3]
	);
}

inline void from_json(const json& j, vector<CAnimator_UI::UI_ANIM_KEYFRAME_DESC>& vec)
{
	vec.clear();
	vec.reserve(j.size());

	for (const auto& element : j)
	{
		CAnimator_UI::UI_ANIM_KEYFRAME_DESC desc = {};
		from_json(element, desc);
		vec.push_back(desc);
	}
}

inline void from_json(const json& j, CAnimator_UI::UI_ANIM_DESC& d)
{
	from_json(j["vecKeyFrames"], d.vecKeyFrames);
	from_json(j["tUIDesc"], d.tUIDesc);

	//d.iLerpType		= j["iLerpType"];
	d.isLoop = j["isLoop"];
	_string strAnimName = j["strAnimName"].get<_string>();
	d.strAnimName = StringToWString(strAnimName);
}

#pragma endregion
