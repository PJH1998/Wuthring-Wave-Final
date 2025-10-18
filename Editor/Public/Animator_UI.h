#pragma once
#include "Component.h"
#include "Level_UI.h"

// UI???醫딅빍筌롫뗄????類ｋ궖??獄쏅?源??곗쨮 ??쇱춳?怨몄뵥 ?醫딅빍筌롫뗄???륁뱽 ???봺???????
NS_BEGIN(Editor)

class CAnimator_UI final : public CComponent
{
public:
	enum class UI_LERPTYPE
	{
		LINEAR,	// 揶쏆늿? ??얜즲嚥?
		LT,		// 筌ｌ꼷?????쥓已? 域???쇰퓠 ?癒?젻筌? / 域밸챶???筌뤴뫁堉????긱걹 ??獄쎻뫚堉????(Left-Top)
		RB,		// 筌ｌ꼷????癒?뵝, 域???쇰퓠 ??뫀?わ쭪? / 域밸챶???筌뤴뫁堉????삘뀲筌??袁⑥삋 獄쎻뫚堉????(Right-Bottom)
		CUBIC,	// 筌ｌ꼷?ф???? ?癒?뵝, 餓λ쵌而??봔?브쑬彛??癒?염??살쓦野???쥓已? / 疫꿸퀣??
			
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
	_float		Fix_LerpRatio(_float fIn, _uint iLerpType);					// Calc_Lerp ?癒?퐣 ????? LerpType???怨뺚뀲 ??쑴??fIn揶쏅???癰귣똻?쇿첎?獄쏆꼹??(0 -> 1 嚥?揶쎛??域밸챶??袁⑹벥 ?ⓥ돦苑??
	_float		Calc_LerpRatio(_float fStart, _float fEnd, _float Ratio);	// ?類ｌ춾 ??λ떄??Ratio ???怨뺚뀲 Start?? End ?????揶쏅???獄쏆꼹??

	_float3		Calc_Lerp_Position_CMR(_uint iKeyframeIndex);				// ??쎈늄??됱뿫???節뚯몵筌??袁⑹삺 ?醫딅빍筌롫뗄???륁벥 ?袁⑹삺 position??筌띿쉶??揶쏅???獄쏆꼹??(catmull-rom ?怨몄뒠)

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