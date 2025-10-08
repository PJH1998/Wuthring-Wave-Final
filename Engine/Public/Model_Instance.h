#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class ENGINE_DLL CModel_Instance : public CComponent
{
private:
	explicit CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CModel_Instance(const CModel_Instance& Prototype);
	virtual ~CModel_Instance() = default;

public:
	_uint								Get_NumMesh() { return m_iNumMeshes; }
	void								Sync_RootNode(class CTransform* pOwnerTransform, class CNavigation* pOwnerNavigation, _float fTimeDelta);

public:
	void								Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions);

public:
	virtual		HRESULT				Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual		HRESULT				Initialize_Clone(void* pArg);
	HRESULT							Render(_uint iMeshIndex);

#ifdef _DEBUG
	_bool								Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif

public:
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType);

private:
	MODELTYPE							m_eType = { MODELTYPE::NONANIM };

	_uint									m_iNumMeshes = {};
	vector<class CMesh_Instance*>			m_Meshes;

	_uint									m_iNumMaterials = {};
	vector<class CMeshMaterial*>	m_Materials;

	_float4x4								m_PreTransformMatrix = {};

private:
	HRESULT							Ready_Mesh(ifstream& InputFile);
	HRESULT							Ready_Material(const _char* pFilePath);

public:
	static		CModel_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, const _char* pFilePath);
	virtual		CComponent* Clone(void* pArg)override;
	virtual		void					Free() override;
};

NS_END
