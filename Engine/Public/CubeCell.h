#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCubeCell final : public CBase
{
	enum class TYPE {
		LBU,	// 좌상단 뒤쪽
		RBU,	// 우상단 뒤쪽
		LFU,	// 좌상단 앞쪽
		RFU,	// 우상단 앞쪽
		LBD,	// 좌하단 뒤쪽
		RBD,	// 우하단 뒤쪽
		LFD,	// 좌하단 앞쪽
		RFD,	// 우하단 앞쪽
		END
	};
private:
	explicit CCubeCell();
	virtual ~CCubeCell() = default;

public:
	HRESULT		Initialize(_float3 vCenter, _float3 vExtent, _uint iDepth);
	void			Update(_float fTimeDelta);
	void			Render();

private:
	BoundingBox*						m_pBoundingBox = { nullptr };
	vector<CCubeCell*>				m_ChildCells;
	vector<class CStaticObject*>	m_Objects;

public:
	static CCubeCell* Create(_float3 vCenter, _float3 vExtent, _uint iDepth);
	virtual void Free() override;
};

NS_END