#pragma once
#include "Base.h"
#include "UI_GrapplePoint.h"

NS_BEGIN(Client)
class CUI_GrapplePoint;

class CUI_GrappleController : public CBase
{
private:
	explicit CUI_GrappleController();
	virtual ~CUI_GrappleController() = default;

public:
	HRESULT				Initialize();

public:
	HRESULT				Create_GrapplePoint(const _float3& vPointPos, UI_GRAPPLE_TYPE eType);
	CUI_GrapplePoint*	Find_NearGrapplePoint(const _float3& vBasePos, UI_GRAPPLE_TYPE eType, _float* pOutDistance);

	//void				SendState_GrapplePoint(CUI_GrapplePoint* pUIPoint);			// send interact, destroy, etc..
	//void				SendState_GrapplePoint(CUI_GrapplePoint* pUIPoint);			// send interact, destroy, etc..
	
	// SendStatus..?
	//void				Remove_GrapplePoint();

private:
	vector<CUI_GrapplePoint*>	m_vecGrapplePoints = {};

	class CGameInstance*		m_pGameInstance = { nullptr };

public:
	static CUI_GrappleController* Create();
	virtual void Free() override;
};

NS_END