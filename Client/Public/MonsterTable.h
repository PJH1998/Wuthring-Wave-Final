#pragma once
#include "Client_Define.h"
#include "Base.h"

NS_BEGIN(Client)
class CGameSystem;

class CMonsterTable final : public CBase
{
private:
	enum NPCTYPE { FEMALE_M, MALE_M, FEMALE_S, END };
	typedef struct tagNPCInfo
	{
		_float3 vPosition;
		_float3 vRotation;
		_uint MeshtypeIndices[3];
		_bool isCollide;
	}NPCINFO;

private:
	explicit CMonsterTable();
	virtual ~CMonsterTable() = default;

public:
	//HRESULT Initialize();
	HRESULT LoadDataTable(const _char* pFilePath);
	HRESULT LoadNPCDataTable(const _char* pFilePath, _uint iType);
	MONSTER_INFO* Get_MonsterInfo(const _char* pMonsterKey) const;

private:
	map<const _string, _uint> m_MonsterKey;
	map<_uint, MONSTER_INFO> m_MonsterTable;
	vector<NPCINFO> m_NPCTable[NPCTYPE::END];
public:
	static CMonsterTable* Create();
	virtual void Free() override;
};
NS_END
