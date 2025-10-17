#pragma once
#include "Component.h"
#include "Level_UI.h"

// UI의 애니메이션 정보를 바탕으로 실질적인 애니메이션을 돌리는 클래스
NS_BEGIN(Editor)

class CAnimator_UI final : public CComponent
{
public:
	enum class UI_LERPTYPE
	{
		LINEAR,	// 같은 속도로.
		LT,		// 처음엔 빠름, 그 뒤에 느려짐. / 그래프 모양이 왼쪽 위 방향을 봄 (Left-Top)
		RB,		// 처음엔 느림, 그 뒤에 빨라짐. / 그래프 모양이 오른쪽 아래 방향을 봄 (Right-Bottom)
		CUBIC,	// 처음과 끝은 느림, 중간 부분만 자연스럽게 빠름. / 기존.
			
		END
	};

	typedef struct tagAnimatorUIDesc
	{
		CCustom_UI*		pOwner = nullptr;
	} ANIMATOR_UI_DESC;

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
	HRESULT		Insert_Animation(CLevel_UI::UI_ANIM_DESC& Desc);
	HRESULT		Remove_Animation(_wstring strAnimName);
	HRESULT		Clear_Animation();

	HRESULT		Change_Animation(_wstring strAnimName);
	HRESULT		Change_Animation(_uint iAnimIndex);
	HRESULT		Deselect_Animation();

	CLevel_UI::UI_ANIM_DESC* Find_Animation(_wstring strAnimName);
	CLevel_UI::UI_ANIM_DESC* Find_Animation(_uint iAnimIndex);

	CLevel_UI::UI_ANIM_DESC* Get_CurAnimation() { return m_pCurAnimDesc; }

private:
	_float		Fix_LerpRatio(_float fIn, _uint iLerpType);					// Calc_Lerp 에서 사용할, LerpType에 따른 비율 fIn값의 보정값 반환 (0 -> 1 로 가는 그래프의 곡선화)
	_float		Calc_LerpRatio(_float fStart, _float fEnd, _float Ratio);	// 정말 단순히 Ratio 에 따른 Start와 End 사이의 값을 반환

	_float3		Calc_Lerp_Position_CMR(_uint iKeyframeIndex);				// 키프레임을 넣으면 현재 애니메이션의 현재 position에 맞는 값을 반환 (catmull-rom 적용)

	void		Update_Animation(_float fTimeDelta);

private:
	vector<CLevel_UI::UI_ANIM_DESC>		m_vecAnimationDescs = {};
	CLevel_UI::UI_ANIM_DESC*			m_pCurAnimDesc = { nullptr };

	CCustom_UI*							m_pOwner = { nullptr };



	_float								m_fElapsedTime = {};

public:
	static CAnimator_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END