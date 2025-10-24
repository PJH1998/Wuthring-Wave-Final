#pragma once
#include "Base.h"
#include "Effect_Prefab.h"

NS_BEGIN(Editor)

class CEffect_Controller :public CBase
{
private:
	explicit CEffect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEffect_Controller() = default;

#pragma region 湲곕낯
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion
public:
	void Prefab_Tab();

	void UpdateSelected_PrefabFromIndex();
	void UpdateSelected_ChildrenFromIndex();

public:
	void Selected_Prefab_Info();

	void Reset_TabInfo();
	void Remove_Prefab();

public:
	void Prefab_To_Json(const _string& strFilePath);

	void Particle_VB_To_Json(json& ParticleVBJson, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc);
	void Particle_OB_To_Json(json& ParticleJson, CParticle::PARTICLE_DESC* pParticleDesc);

	void Mesh_VB_To_Json(json& MeshVBJson, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* pVBDesc);
	void Mesh_OB_To_Json(json& MeshJson, CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc);

	void TrailMesh_To_Json(json& TrailMesh, CTrail_Mesh::TRAILMESH_DESC* pTrailDesc);

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };
	class CParticle_Controller*									m_pParticle_Controller = { nullptr };
	class CMesh_Controller*										m_pMesh_Controller = { nullptr };
	class CTrailMesh_Controller*								m_pTrailMesh_Controller = { nullptr };

	_char														m_PrefabTag[MAX_PATH];
	_bool														m_bTagFlag = false;

	_char														m_ChildrenTag[MAX_PATH];
	_bool														m_bChildrenTagFlag = false;
	_bool														m_bChildrenCreatFlag = false;
	EFFECT_TYPE													m_eChildrenType = EFFECT_TYPE::END;

	_int														m_iSelectedPrefab = 0;
	_bool														m_bSelectedPrefab = false;
	class CEffect_Prefab*										m_pSelectedPrefab = { nullptr };

	CEffect_Prefab::PREFAB_DESC*								m_pSelectedPrefabDesc = { nullptr };
	CEffect_Prefab::FRAME_DESC*									m_pSelectedPrefabFrame = { nullptr };

	_int														m_iSelectedChildren = 0;
	_wstring													m_strChildrenTag = {};
	_bool														m_IsParticle = false;
	_bool														m_IsMeshEffect = false;
	_bool														m_IsTrailMesh = false;

	map<const _wstring, class CEffect_Prefab*>					m_Prefabs = {};
	map<const _wstring, CEffect_Prefab::PREFAB_DESC>			m_PrefabDesc = {};

public:
	static CEffect_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END