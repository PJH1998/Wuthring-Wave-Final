#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CCubeCell final : public CBase
{
public:
	enum class CORNER {
		LBU,	// Left Backward Up
		RBU,	// Right Backward Up
		LFU,	// Left Forward Up
		RFU,	// Right Forward Up
		LBD,	// Left Backward Down
		RBD,	// Right Backward Down
		LFD,	// Left Forward Down
		RFD,	// Right Forward Down
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
	_uint									m_iLODIndex = {};

	_float3								m_Corners[ENUM_CLASS(CORNER::END)] = {};
	_float									m_MinMax[ENUM_CLASS(MINMAX::END)] = {};

	recursive_mutex					m_Mutex;

private:
	void									Compute_MinMax();
	_bool									isIn(const _float* pMinMax);
	void									Compute_LOD(const _fvector& vCamPos);

public:
	static CCubeCell* Create(_float3 vCenter, _float3 vExtent, _uint iDepth);
	virtual void Free() override;
};

NS_END