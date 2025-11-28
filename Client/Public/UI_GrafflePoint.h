#pragma once
#include "UI_Image.h"

#define KSTA_UITEST_GRAFFLE_TOZERO

NS_BEGIN(Client)

class CUI_GrafflePoint final : public CUI_Image
{
public:
	typedef struct tUILockOnDesc {
		_float3* pTargetPos = nullptr;
	} UI_GRAFFLEPOINT_DESC;

private:
	enum UI_GRAFFLE_TRIGGER { ENTER, EXIT, SEMIENTER, SEMIEXIT, NONE };
	enum UI_GRAFFLE_STATE { UNVISIBLE, OUTER, INNER };

public:
	explicit CUI_GrafflePoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUI_GrafflePoint(const CUI_GrafflePoint& Prototype);
	virtual ~CUI_GrafflePoint() = default;

public: // 생성/복제
	virtual HRESULT Initialize_Prototype()							override;
	virtual HRESULT Initialize_Clone(void* pArg)					override;
	virtual void    Priority_Update(_float fTimeDelta)				override;
	virtual void    Update(_float fTimeDelta)						override;
	virtual void    Late_Update(_float fTimeDelta)					override;
	virtual void    Render()										override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg)	override;

private:
	void			PreAssign_ChildUIs();
	void			Ready_Presets();
	void			Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos);
	void			Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance);

private:
	void			Update_AnimOrder(_float fTimeDelta);

private:
	const _float	m_fPivotDistance = 10.f;		// 거리에 따른 크기 조절용. 이 거리일 때 최대 크기로 보임.

	const _float	m_fTriggerDistance = 50.f;		// 상호작용 가이드가 뜰 범위
	const _float	m_fVisibleDistance = 80.f;		// 보이기 시작할 범위
	
	CCustom_UI*		m_pRUI_All		= nullptr;
	CCustom_UI*		m_pStaticUI		= nullptr;
	CCustom_UI*		m_pDynamicUI	= nullptr;

	CAnimator_UI*	m_pSubAnimUI	= nullptr;	
	CAnimator_UI*	m_pStaticAnimUI	= nullptr;	
	CAnimator_UI*	m_pDynamicAnimUI= nullptr;

	_float3*		m_pTargetPos = { nullptr };


	//_bool			m_i

#ifdef KSTA_UITEST_GRAFFLE_TOZERO
	_bool			m_DEBUG_isAssignedPosition = false;
#endif // KSTA_UITEST_GRAFFLE_TOZERO



	// Animation Control

	UI_GRAFFLE_TRIGGER  m_eTriggerState = NONE;

	UI_GRAFFLE_STATE	m_eCurDistState = UNVISIBLE;
	UI_GRAFFLE_STATE	m_ePrevDistState = UNVISIBLE;

	_bool				m_isUnvisible = false;
	_bool				m_isUnvisibleStandby = false;
	_float				m_fGoinUnvisibleTime = 0.f;
	const _float		m_fUnvisibledTime = 0.25f;
	
public:
	static CUI_GrafflePoint* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END