#pragma once
#include "VIBuffer_Instance.h"
NS_BEGIN(Engine)

class CMesh_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagInstanceMesh {
		_uint iNumInstance;
		_float4x4* pTransformMatrix = { nullptr };
	}MESH_INST_DESC;
public:
	_uint							Get_MaterialIndex() { return m_iMaterialIndex; }

private:
	CMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh_Instance(const CMesh_Instance& Prototype);
	CMesh_Instance() = default;

public:
	virtual HRESULT		Initialize_Prototype(_fmatrix PreTransformMatrix, ifstream& InputFile);
	virtual HRESULT		Initialize_Clone(void* pArg);
	//virtual HRESULT		Render();

	//virtual HRESULT		Bind_Resources();

private:
	_float4x4* m_TransformMatrices = { nullptr };

private:
	_uint							m_iMaterialIndex = {};
	//vector<_float4x4>				m_OffsetMatrices;

#ifdef _DEBUG
	vector<_float3>			m_VertexPositions;
	vector<_uint>				m_Indices;
#endif

public:
	static CMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, ifstream& InputFile);
	virtual CComponent* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END