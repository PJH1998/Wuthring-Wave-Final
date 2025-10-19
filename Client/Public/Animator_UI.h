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
		_float3			vRot = {};			// Euler
		_float3			vSca = {};

		_float4		    vBlendOuterWidth = { };			// 방향 별 그라디언트 두께. 좌우상하 순. 음수 가능.

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

private:
	_float		Fix_LerpRatio(_float fIn, _uint iLerpType);					// Calc_Lerp 에서 사용할, LerpType에 따른 비율 fIn값의 보정값 반환 (0 -> 1 로 가는 그래프의 곡선화)
	_float		Calc_LerpRatio(_float fStart, _float fEnd, _float Ratio);	// 정말 단순히 Ratio 에 따른 Start와 End 사이의 값을 반환

	_float3		Calc_Lerp_Position_CMR(_uint iKeyframeIndex);				// 키프레임을 넣으면 현재 애니메이션의 현재 position에 맞는 값을 반환 (catmull-rom 적용)

	void		Update_Animation(_float fTimeDelta);

private:
	vector<UI_ANIM_DESC>		m_vecAnimationDescs = {};
	UI_ANIM_DESC*				m_pCurAnimDesc = { nullptr };

	CCustom_UI*					m_pOwner = { nullptr };

	_float						m_fElapsedTime = {};

public:
	static CAnimator_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END