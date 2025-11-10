#pragma once
#include "Client_Define.h"
#include "Base.h"

NS_BEGIN(Client)
class CGameSystem;

class CMonsterTable final : public CBase
{
private:
	explicit CMonsterTable();
	virtual ~CMonsterTable() = default;

public:
	//HRESULT Initialize();
	HRESULT LoadDataTable(const _char* pFilePath);
	MONSTER_INFO* Get_MonsterInfo(const _char* pMonsterKey) const;

private:
	map<const _string, _uint> m_MonsterKey;
	map<_uint, MONSTER_INFO> m_MonsterTable;

public:
	static CMonsterTable* Create();
	virtual void Free() override;
};
NS_END
