#pragma once
#include "Base.h"
#include"MapObject_Instance.h"

NS_BEGIN(Client)

class CParser final : public CBase
{
public:
	typedef struct tagSpawnDesc {
		_float4 vMonsterSpawnorPos;

		_float4 vMonsterPos1;
		_char szMonsterName1[MAX_PATH] = {};

		_float4 vMonsterPos2;
		_char szMonsterName2[MAX_PATH] = {};

		_float4 vMonsterPos3;
		_char szMonsterName3[MAX_PATH] = {};
	}SPAWN_DESC;

private:
	explicit CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CParser() = default;

public:
#pragma region MAP
	// File Model
	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel);
	void							Clone_MapObjects(LEVEL eLevel);
#pragma endregion

#pragma region SPAWNER
	void							Clone_Spawners(LEVEL eLevel);
#pragma endregion

#pragma region CSV
	// Load CSV File (Excel)
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
#pragma endregion

#pragma region SEQUENCE
	void							Load_Sequence(const _char* pFolderPath);
#pragma endregion


private:
	void							Read_Map_Prototype(const _string pFilePath, LEVEL eLevel);
	void							Read_Map_Dat(LEVEL eLevel, const _string pFilePath);

#pragma region EffectLoad
public:
	//프리팹 말고, 자식들 원형 미리 생성해놓고자 함.
	void						Create_Effect(const string& strFolderPath, LEVEL eLevel);
	
	//자식들 원형 만들어놨으면 프리팹 읽어서 프리팹 생성. 풀링 매니저에 넣어줘야함.
	//폴더째로 프리팹 읽을거면 개수를 여기서 지정해줘야됨.
	//폴더째로 읽고, 폴더로 나눠두면 좋을거 같은데 ex) 보스, 아우구스타, 공용, 등등.
	void						Create_Prefab(const string& strFolderPath, LEVEL eLevel, _int PoolingNum); 

	//텍스처랑 Dat 먼저 읽어놔야 위에 이펙트 문제없이 클론가능.
	void						Load_EffectTexture_FromFolder(const string& strFolderPath, LEVEL eLevel);
	void						Load_EffectMeshDat_FromFolder(const string& strFolderPath, LEVEL eLevel);
	void						Load_FXDecal_Data_FromFolder(const string& strFolderPath);
private:
	//원형 있어야 클론가능.
	void						Load_Prefab_FromJson(const _string& strFilePath, const _string& strPrefabTag, LEVEL eLevel, _int PoolingNum);

	//원형 만들어놓기
	void						Load_Particle_VB_FromJson(const _string& strFilePath, const _string& VBTag, LEVEL eLevel);
	void						Load_Particle_OB_FromJson(const _string& strFilePath, const _string& ParticleTag, LEVEL eLevel);
	void						Load_TrailMesh_FromJson(const _string& strFilePath, const _string& TrailMeshTag, LEVEL eLevel);
	void						Load_FXRect_FromJson(const _string& strFilePath, const _string& RectTag, LEVEL eLevel);
	void						Load_FXDecal_FromJson(const _string& strFilePath, const _string& DecalTag, LEVEL eLevel);
	void						Load_FXDecal_Data_FromJson(const _string& strFilePath);
#pragma endregion
public:
	HRESULT						Initialize();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	vector<vector<_string>> m_Data;
	unordered_map<LEVEL, vector<const _char*>> m_LoadingMap;
	vector<CMapObject_Instance::MAP_LOAD> m_MapInstanceData;
	//unordered_map<const _char*, vector<SPAWN_DESC>> m_MonsterDesc;
	unordered_map<LEVEL, vector<SPAWN_DESC>> m_MonsterDesc;
public:
	static		CParser*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END