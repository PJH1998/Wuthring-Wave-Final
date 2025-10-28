#pragma once
#include "Base.h"
#include "Effect_Prefab.h"
#include "Particle_Controller.h"
#include "Mesh_Controller.h"
#include "TrailMesh_Controller.h"
#include "Load_Controller.h"

NS_BEGIN(Editor)
class CEffect_Controller :public CBase
{
private:
	typedef struct tagAnimationActorDesc
	{
		class CModel* pModelCom = { nullptr };
		class CAnimationActor* pAnimActor = { nullptr };
		_string strAnimName = {};
		float fDuration = {};

		const _float4x4* pBoneMatrix = { nullptr };
	}ANIMACTOR_DSEC;

private:
	explicit CEffect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CEffect_Controller() = default;

#pragma region 
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

	void Reset_ChildrenInfo();
	void Reset_PrefabInfo();
	void Remove_Prefab();

public:
	void Prefab_To_Json(const _string& strFilePath);

	void Particle_VB_To_Json(json& ParticleVBJson, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* pVBDesc);
	void Particle_OB_To_Json(json& ParticleJson, CParticle::PARTICLE_DESC* pParticleDesc);

	void Mesh_VB_To_Json(json& MeshVBJson, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* pVBDesc);
	void Mesh_OB_To_Json(json& MeshJson, CEffect_Mesh::EFFECTMESH_DESC* pMeshDesc);

	void TrailMesh_To_Json(json& TrailMesh, CTrail_Mesh::TRAILMESH_DESC* pTrailDesc);

public:
	void Load_Prefab();

	void Load_Particle(const _wstring& ParticleTag);

	void Load_FXMesh(const _wstring& FXMeshTag);

	void Load_TrailMesh(const _wstring& TrailMeshTag);

public:
	void Save_SelectedChildren_To_Json();
	void Load_Children_To_Json();
	void Load_Children_To_PrefabDesc(_wstring& ChildrenTag, EFFECT_TYPE eChildrenType);

public:
	void Import_AnimationData(const EFFECTACTOR_DESC& effectActorDesc);
	void PrefabBinding_Tab();

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };
	class CParticle_Controller*									m_pParticle_Controller = { nullptr };
	class CMesh_Controller*										m_pMesh_Controller = { nullptr };
	class CTrailMesh_Controller*								m_pTrailMesh_Controller = { nullptr };
	class CLoad_Controller*										m_pLoad_Controller = { nullptr };

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

	_bool														m_IsLoad = false;

	//프리팹에 바인딩할 애니메 정보 관련
	ANIMACTOR_DSEC												m_AnimActorDesc = {};
	_char														m_BoneName[MAX_PATH];
	_bool														m_bBoneFlag = false;
	_float														m_fTrackPosition = -1.f;
	_bool														m_bTest = false;


public:
	static CEffect_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END