#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CBGM_Manager final : public CBase
{
private:
	explicit CBGM_Manager();
	virtual ~CBGM_Manager() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);

public:  
	void Change_Level(_uint iLevel);
	void Stop_BGM();
		 
	void Engage_Battle(_bool IsBattle, BOSSBGM eBossLevel);
	void Change_BGM(const _wstring& BGMText);
	void Change_BattleBGM(BOSSBGM eBoss);
	_bool IsModinaryBattle() { return m_eLastBattle == BOSSBGM::MODINARY; }

private: 
	void Ready_BGM();

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	unordered_map<_uint, _wstring> m_BGMs;
	_wstring m_BattleBGM;

	_wstring m_szChangeBGMName;
	_wstring m_szChangeBattleBGMName;
	map<BOSSBGM, _wstring> m_BossBGM;
	BOSSBGM m_eLastBattle = { BOSSBGM::END };


	_bool m_IsCurBGMChange = { false };
	_bool m_IsCurBattleBGMChange = { false };
	_float m_fBGMRate = {};
	_bool m_IsBattle = { false };

public:
	static CBGM_Manager* Create();
	virtual void Free()override;
};

NS_END