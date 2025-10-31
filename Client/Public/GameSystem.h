#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CGameSystem final : public CBase
{
	DECLARE_SINGLETON(CGameSystem)
private:
	explicit CGameSystem();
	virtual ~CGameSystem() = default;

public:
#pragma region GameSystem
	void		Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
#pragma endregion

#pragma region Parser
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
	void							Create_Map_Model(const _char* pFilePath, LEVEL eLevel);
#pragma endregion

#pragma region Factory
	void							Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix);
#pragma endregion

#pragma region CHARACTER INFO
	void Sync_CharacterInfo(const CHARACTER_STAT& eCharacterStat);
#pragma endregion


private:
	class		CParser*		m_pParser = { nullptr };
	class		CFactory*	m_pFactory = { nullptr };
	CHARACTER_STAT m_Stats = {};
	//unordered_map<LEVEL,const _char*> m_
public:
	virtual		void	Free() override;

};

NS_END