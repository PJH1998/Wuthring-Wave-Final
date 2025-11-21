#pragma once
#include "Component.h"
#include"Mesh_Streaming.h"

NS_BEGIN(Engine)
class ENGINE_DLL CModel_Streaming final : public CComponent
{
private:
	explicit CModel_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CModel_Streaming(const CModel_Streaming& Prototype);
	virtual ~CModel_Streaming() = default;
public:

	_uint								Get_NumMesh(_uint iMeshIndex) { return m_pModelPrototype->m_iNumMeshes[iMeshIndex]; }
public:
	virtual		HRESULT				Initialize_Prototype(const _char* pFilePath);
	virtual		HRESULT				Initialize_Clone(void* pArg);
	HRESULT							Render(_uint iLODIndex, _uint iMeshIndex);
	HRESULT							Render(_uint iLODIndex, _uint iMeshIndex, ID3D11DeviceContext* pDC);

public:
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);

	void							Ready_BoundingBox(_float* pMinPos, _float* pMaxPos);

	atomic<LOADSTATE>& Get_MeshState(_uint iLODIndex) { return m_pModelPrototype->m_LodState[iLODIndex]; }
public:
	vector<CModel_Manager::SHARED_DATA_DESC>* Get_MeshDesc(_uint iLODIndex);
	void									  RequestModel(_uint iLODIndex = 99);
	void									  Set_RenderTime(_uint iLODIndex, _float fTimeDelta) { m_pModelPrototype->m_fRenderTime[iLODIndex] = fTimeDelta; }
	_bool									  Is_RenderTimeOver(_uint iLODIndex);
public:
	HRESULT							Ready_Mesh(const _char* pFilePath);
	HRESULT							Ready_Material();
	void							PlusRenderdTime(_float fTimeDelta);
	HRESULT							Get_SharedBuffers(_uint iLODIndex, ID3D11Buffer* pVertex, ID3D11Buffer* pIndex);
	_bool							Is_Overed(_uint iLODIndex, _uint iMeshIndex);
	const _string&					Find_ModelPrototype() { return m_pModelPrototype->m_ModelPath; }
	_uint							Get_LastLODIndex();
	void							Request_LOD(_uint iLODIndex);
	_uint							Get_ReadyLOD();
private:
	_uint									m_iNumMeshes[4] = { 0,0,0,0 };
	class CMesh_Streaming*					m_Meshes[4] = { nullptr,nullptr,nullptr,nullptr };

	_uint									m_iNumMaterials = {};
	vector<class CMaterial*>	m_Materials;
	atomic<LOADSTATE>						m_LodState[4] = { LOADSTATE::NOTLOADED,LOADSTATE::NOTLOADED ,LOADSTATE::NOTLOADED ,LOADSTATE::NOTLOADED };

	_float									m_fRenderTime[4] = { 0.f,0.f,0.f,0.f };
	_string m_ModelPath;
	_uint m_iMaxLOD = { 0 };
	CModel_Streaming* m_pModelPrototype = { nullptr };
	//메쉬를 최대 4개만 가지게 한 뒤에
	//LOD데이터를 로드할 때 메쉬 맨 처음 데이터가 메쉬 수. 각 데이터 읽을 때마다 그 데이터 크기만큼 FreeList에 할당, 각 메쉬별주소도 저장.
	//각 메쉬별 데이터를 메쉬 안에 전달 그 수만큼 할당? 할당과 파괴를 도중에 하는건 안됨. 벡터 전달??
public:
	static		CModel_Streaming* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath);
	virtual		CComponent* Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END