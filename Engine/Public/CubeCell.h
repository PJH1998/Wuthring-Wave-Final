#pragma once
#include "Base.h"

#define MAX_OBJECT_PER_LOD 1000
#define LOOSE_CELL_SCALE 1.2f

NS_BEGIN(Engine)

class CCubeCell final : public CBase
{
public:
	enum class CORNER {
		LFD,	// Left Forward Down
		RFD,	// Right Forward Down
		RFU,	// Right Forward Up
		LFU,	// Left Forward Up
		LBD,	// Left Backward Down
		RBD,	// Right Backward Down
		RBU,	// Right Backward Up
		LBU,	// Left Backward Up
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
	HRESULT		Initialize(const _float3& vCenter, const _float3& vExtent, _uint iDepth);
	void			Update(const _fvector& vCamPos, vector<class CStaticObject*>(&Container)[4]);

	void			Add_Object(class CStaticObject* pObject, const _float* pMinMax);

private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	BoundingBox*						m_pBoundingBox = { nullptr };
	vector<CCubeCell*>				m_ChildCells;
	_float									m_MinMax[ENUM_CLASS(MINMAX::END)] = {};
	_uint									m_iDepth = {};

	vector<class CStaticObject*>	m_Objects;
	mutex									m_Mutex;
	_uint									m_iLODIndex = {};

	_float3								m_Corners[ENUM_CLASS(CORNER::END)] = {};


private:
	void									Compute_MinMax();
	_bool									isIn(const _float* pMinMax);
	void									Compute_Cell_LOD(const _fvector& vCamPos);
	_uint									Compute_Object_LOD(class CStaticObject* pObject, const _fvector& vCamPos);

public:
	static CCubeCell* Create(const _float3& vCenter, const _float3& vExtent, _uint iDepth);
	virtual void Free() override;
};

NS_END