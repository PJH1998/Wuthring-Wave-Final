#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCubeCell final : public CBase
{
public:
	enum class CORNER {
		LFD,	// 醫뚰븯???욎そ
		RFD,	// ?고븯???욎そ
		RFU,	// ?곗긽???욎そ
		LFU,	// 醫뚯긽???욎そ
		LBD,	// 醫뚰븯???ㅼそ
		RBD,	// ?고븯???ㅼそ
		RBU,	// ?곗긽???ㅼそ
		LBU,	// 醫뚯긽???ㅼそ
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
	void			Update(const _fvector& vCamPos);

	void			Add_Object(class CStaticObject* pObject, const _float* pMinMax);

private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	BoundingBox*						m_pBoundingBox = { nullptr };
	vector<CCubeCell*>				m_ChildCells;
	vector<class CStaticObject*>	m_Objects;
	_uint									m_iDepth = {};

	_float3								m_Corners[ENUM_CLASS(CORNER::END)] = {};
	_float									m_MinMax[ENUM_CLASS(MINMAX::END)] = {};

private:
	void									Compute_MinMax();
	_bool									isIn(const _float* pMinMax);

public:
	static CCubeCell* Create(_float3 vCenter, _float3 vExtent, _uint iDepth);
	virtual void Free() override;
};

NS_END