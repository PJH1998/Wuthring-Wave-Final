#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCubeCell final : public CBase
{
	enum class CORNER {
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

	enum class MINMAX {
		MIN_X,
		MAX_X,
		MIN_Y,
		MAX_Y,
		MIN_Z,
		MAX_Z,
		END
	};
private:
	explicit CCubeCell();
	virtual ~CCubeCell() = default;

public:
	HRESULT		Initialize(_float3 vCenter, _float3 vExtent, _uint iDepth);
	void			Priority_Update(_float fTimeDelta);
	void			Update(_float fTimeDelta);
	void			Late_Update(_float fTimeDelta);

	void			Add_Object(class CStaticObject* pObject, const _float* pMinMax);
	_bool			isIn(const _float* pMinMax);

private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	BoundingBox*						m_pBoundingBox = { nullptr };
	vector<CCubeCell*>				m_ChildCells;
	vector<class CStaticObject*>	m_Objects;

	_float3								m_Corners[ENUM_CLASS(CORNER::END)] = {};
	_float									m_MinMax[ENUM_CLASS(MINMAX::END)] = {};

private:
	void									Compute_MinMax();

public:
	static CCubeCell* Create(_float3 vCenter, _float3 vExtent, _uint iDepth);
	virtual void Free() override;
};

NS_END