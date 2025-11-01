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
#pragma region GAMESYSTEM
	void		Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
#pragma endregion

#pragma region PARSER
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel);
	void							Clone_MapObjects(LEVEL eLevel, _uint iIndex);
#pragma endregion

#pragma region FACTORY
	void							Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix);
#pragma endregion

#pragma region DIRECTOR
	void							Add_Action(const _char* pFolderPath);
	void							Play_Action(const _wstring& strActionTag, const _fmatrix& WorldMatrix, _bool isMaintain);
	void							Stop_Action();
#pragma endregion


#pragma region CHARACTER INFO
	void Sync_CharacterInfo(const CHARACTER_STAT& eCharacterStat);
#pragma endregion



private:
	class		CParser*		m_pParser = { nullptr };
	class		CFactory*	m_pFactory = { nullptr };
	class		CDirector*	m_pDirector = { nullptr };
	CHARACTER_STAT m_Stats = {};
public:
	virtual		void	Free() override;

};

NS_END