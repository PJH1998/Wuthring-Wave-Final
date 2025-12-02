#pragma once
#include "Client_Define.h"
#include "Base.h"

NS_BEGIN(Client)
class CGameSystem;

class CMonsterTable final : public CBase
{
private:
	enum NPCTYPE { FEMALE_M, MALE_M, FEMALE_S, END };

private:
	explicit CMonsterTable();
	virtual ~CMonsterTable() = default;

public:
	//HRESULT Initialize();
	HRESULT						LoadDataTable(const _char* pFilePath);
	HRESULT						LoadNPCDataTable(const _char* pFilePath, _uint iType);
	MONSTER_INFO*				Get_MonsterInfo(const _char* pMonsterKey) const;
	_uint						Get_NumNPCInstance(_uint iType) const { return m_NPCTable[iType].size(); }
	const vector<NPCINFO>&		Get_NpcData(_uint iType) const;
	void						Clear_NPCData();
private:
	map<const _string, _uint> m_MonsterKey;
	map<_uint, MONSTER_INFO> m_MonsterTable;
	vector<NPCINFO> m_NPCTable[NPCTYPE::END];
public:
	static CMonsterTable* Create();
	virtual void Free() override;
};
NS_END
