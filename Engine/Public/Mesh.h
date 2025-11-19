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
	_uint					Get_MaterialIndex() { return m_iMaterialIndex; }
	const vector<_float3>&	Get_VerticesPos() { return m_VertexPositions; }
	const vector<_uint>&	Get_Indices() { return m_Indices; }

	const vector<_uint>& Get_BoneIndices() { return m_BoneIndices; }
	const vector<_float4x4>& Get_OffsetMatrices() const { return m_OffsetMatrices; }

	const vector<class CShapeKey*>& Get_ShapeKeys() { return m_ShapeKeys; }
	_uint	Get_NumVertices() const { return m_iNumVertices; }
	_bool Has_MorphData() const { return (m_pMorphPosSRV != nullptr); }
public:
	virtual		HRESULT			Initialize_Prototype(MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos = nullptr, _float* MaxPos = nullptr);
	virtual		HRESULT			Initialize_Clone(void* pArg);

public:
	void Update_Morph_CPU(const vector<_float>& vShapeKeyWeights);
#ifdef _DEBUG
	_bool							Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif

public:
	HRESULT						Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones);
	HRESULT						Bind_MorphSRV(class CShader* pShaderCom);

private:
	_uint							m_iMaterialIndex = {};
	_uint							m_iNumBones = {};

	vector<_uint>				m_BoneIndices;
	_float4x4					m_BoneMatrices[g_iMaxNumBones] = {};

	vector<_float4x4>			m_OffsetMatrices;

	// Mesh Shape
	vector<_float3>				m_VertexPositions;
	vector<_uint>				m_Indices;

#pragma region SHAPE KEY 정보
private:
	_uint m_iNumAnimMeshes = {};
	vector<class CShapeKey*> m_ShapeKeys;
	//map<_string, class CShapeKey*> m_ShapeKeys;

	VTXANIMMESH* m_pRestPoseVertices = {}; // T Pose 정점 데이터

	// [추가] Morph Target용 쉐이더 리소스 뷰 (StructuredBuffer View)
	ID3D11ShaderResourceView* m_pMorphPosSRV    = nullptr;     // t10
	ID3D11ShaderResourceView* m_pMorphNormalSRV = nullptr;  // t11
#pragma endregion


private:
	HRESULT						Ready_Mesh_NonAnim(_fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Character(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Map(_fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos);

private:
	HRESULT						Ready_MorphBuffers(); // 초기화시 Delta 데이터를 GPU 버퍼로 만드는 헬퍼함수.

public:
	static		CMesh*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos=nullptr, _float* MaxPos=nullptr);
	virtual		CComponent*	Clone(void* pArg);
	virtual		void				Free() override;
};

NS_END