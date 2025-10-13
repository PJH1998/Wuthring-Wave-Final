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
		LINEAR,
		CUBIC,
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
	// 애니메이션 전환 관련 함수..?
	// Play_Animation (인자) 등..
	HRESULT		Insert_Animation(CLevel_UI::UI_ANIM_DESC* pDesc);

	HRESULT		Change_Animation(_wstring strAnimName);
	HRESULT		Change_Animation(_uint iAnimIndex);

private:
	_float		Fix_LerpRatio(_float fIn, _uint iLerpType);
	_float		Calc_Lerp(_float fStart, _float fEnd, _float Ratio);

	void		Update_Animation(_float fTimeDelta);

private:
	// UI 담은 컨테이너..?
	// UI는 애니메이션 정보를 담도록..?

	// 애니메이션 클래스를 저장하는 게 아니라 그냥 애니메이션 Desc를 저장하고 불러와서 쓰면 되는 것이 아닌지?
	// 그러면 컨테이너로 Map 사용
	vector<CLevel_UI::UI_ANIM_DESC*>	m_vecAnimationDescs = {};
	CLevel_UI::UI_ANIM_DESC*			m_pCurAnimDesc = { nullptr };

	CCustom_UI*							m_pOwner = { nullptr };



	_float								m_fElapsedTime = {};

public:
	static CAnimator_UI*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END