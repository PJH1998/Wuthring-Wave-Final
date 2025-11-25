#include "ClientPch.h"
#include "MonsterTable.h"
#include "GameSystem.h"

CMonsterTable::CMonsterTable()
{
}

HRESULT CMonsterTable::LoadDataTable(const _char* pFilePath)
{
	vector<vector<string>> Datas = CGameSystem::GetInstance()->Load_CSV(pFilePath);
	if (Datas.size() < 2)
	{
		MSG_BOX("Data Not Found");
		return E_FAIL;
	}
	_string PoolTag = "Pool_Enemy_";
	for (size_t i = 1; i < Datas.size(); ++i)
	{
		MONSTER_INFO MobInfo{};
		
		MobInfo.strName			= Datas[i][0];
		MobInfo.iMonsterID		= stoi(Datas[i][1]); //참조용 몬스터ID
		MobInfo.wstrPoolTag		= StringToWString(PoolTag + MobInfo.strName);
		MobInfo.fMaxHp			= stof(Datas[i][2]);
		MobInfo.fMaxStamina		= stof(Datas[i][3]);
		MobInfo.fAttack			= stof(Datas[i][4]);
		MobInfo.fAttackAddMin	= stof(Datas[i][5]);
		MobInfo.fAttackAddMax	= stof(Datas[i][6]);
		MobInfo.fImpluseRate	= stof(Datas[i][7]);

		m_MonsterKey.emplace(MobInfo.strName, MobInfo.iMonsterID);
		m_MonsterTable.emplace(MobInfo.iMonsterID, MobInfo);
	}
    return S_OK;
}

HRESULT CMonsterTable::LoadNPCDataTable(const _char* pFilePath, _uint iType)
{
	vector<vector<string>> Datas = CGameSystem::GetInstance()->Load_CSV(pFilePath);
	if(Datas.size() < 2)
	{
		MSG_BOX("Data Not Found");
		return E_FAIL;
	}

	for(size_t i = 1; i < Datas.size(); ++i)
	{
		NPCINFO Info{};
		Info.vPosition.x = stof(Datas[i][1]);
		Info.vPosition.y = stof(Datas[i][2]);
		Info.vPosition.z = stof(Datas[i][3]);

		Info.vRotation.x = XMConvertToRadians(stof(Datas[i][4]));
		Info.vRotation.y = XMConvertToRadians(stof(Datas[i][5]));
		Info.vRotation.z = XMConvertToRadians(stof(Datas[i][6]));

		Info.MeshtypeIndices[0] = stoi(Datas[i][7]);
		Info.MeshtypeIndices[1] = stoi(Datas[i][8]);
		Info.MeshtypeIndices[2] = stoi(Datas[i][9]);

		Info.isCollide = stoi(Datas[i][10]) == 0 ? false : true;

		m_NPCTable[NPCTYPE(iType)].push_back(Info);
	}

	return S_OK;
}

MONSTER_INFO* CMonsterTable::Get_MonsterInfo(const _char* pMonsterKey) const  
{  
   auto Keyiter = m_MonsterKey.find(pMonsterKey);  
   if(Keyiter == m_MonsterKey.end())  
       return nullptr;  

   auto Infoiter = m_MonsterTable.find(Keyiter->second);  
   if (Infoiter == m_MonsterTable.end())  
       return nullptr;  

   return const_cast<MONSTER_INFO*>(&Infoiter->second);  
}

CMonsterTable* CMonsterTable::Create()
{
	return new CMonsterTable();
}

void CMonsterTable::Free()
{
	__super::Free();

	m_MonsterKey.clear();
	m_MonsterTable.clear();

}
