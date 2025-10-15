#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCubeCell final : public CBase
{
private:
	explicit CCubeCell();
	virtual ~CCubeCell() = default;

public:
	HRESULT		Initialize(_float3 vCenter, _float3 vExtent, _uint iDepth);
	void			Update(_float fTimeDelta);
	void			Render();

private:
	BoundingBox*		m_BoundingBox = { nullptr };

public:
	static CCubeCell* Create(_float3 vCenter, _float3 vExtent, _uint iDepth);
	virtual void Free() override;
};

NS_END