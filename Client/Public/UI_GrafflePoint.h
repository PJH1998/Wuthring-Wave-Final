#pragma once
#include "UI_Image.h"


NS_BEGIN(Client)

class CUI_GrafflePoint final : public CUI_Image
{
public:
	typedef struct tUI_GrafflePointDesc {
		_float3* pTargetPos = nullptr;
	} UI_GRAFFLEINFO_DESC;

private:
	typedef struct tUI_GraffleRTDesc {
		UI_GRAFFLEINFO_DESC tInfoDesc = {};

		_float		fCurElapsedTime = 0.f;
		_float		fCurStackedTime = 0.f;

		_float		fAtkedElapsedTime = {};
		_bool		isTimerActived = {};
	} UI_GRAFFLE_RT_DESC;
	
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

private:
	void			Update_Instances();

private:
	_float3			Calc_PosToScreen(const _float3* v3DPos);
	_float3			Calc_CamDistScale_PerInst(const _float3* vInstSca, _float fPivotDistance);	// 이거 인스턴스별로..

private:
	CCustom_UI*			m_pGrafflePointUI		= nullptr;
	//CCustom_UI*			m_pDynamicPoint		= nullptr;

	vector<UI_GRAFFLE_RT_DESC>	m_vecGraffleInfo;

public:
	static CUI_GrafflePoint*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

NS_END