#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class CMesh final : public CVIBuffer
{
	enum CORNER { LTN, RTN, RBN, LBN, LTF, RTF, RBF, LBF, END};
private:
	explicit CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMesh(const CMesh& Prototype);
	virtual ~CMesh() = default;

public:
	_uint							Get_MaterialIndex() { return m_iMaterialIndex; }
	const vector<_float3>&	Get_VerticesPos() { return m_VertexPositions; }
	const vector<_uint>&		Get_Indices() { return m_Indices; }

	const vector<_uint>& Get_BoneIndices() { return m_BoneIndices; }
	const vector<_float4x4>& Get_OffsetMatrices() const { return m_OffsetMatrices; }

public:
	virtual		HRESULT			Initialize_Prototype(MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos = nullptr, _float* MaxPos = nullptr);
	virtual		HRESULT			Initialize_Clone(void* pArg);

#ifdef _DEBUG
	_bool							Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif

public:
	HRESULT						Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones);

private:
	_uint							m_iMaterialIndex = {};
	_uint							m_iNumBones = {};

	vector<_uint>				m_BoneIndices;
	_float4x4					m_BoneMatrices[g_iMaxNumBones] = {};

	vector<_float4x4>			m_OffsetMatrices;

	// Mesh Shape??Container
	vector<_float3>				m_VertexPositions;
	vector<_uint>				m_Indices;

private:
	HRESULT						Ready_Mesh_NonAnim(_fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Character(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Map(_fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos);

public:
	static		CMesh*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos=nullptr, _float* MaxPos=nullptr);
	virtual		CComponent*	Clone(void* pArg);
	virtual		void				Free() override;
};

NS_END