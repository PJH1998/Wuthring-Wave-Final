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
	_bool IsModinaryBattle() { return m_eLastBattle == BOSSBGM::MODINARY; }


	//잡몹전투 -> 스포너가 MODEINARY 전달.
	//MODINARY면 잡몹전투 브금 실행. IsModinaryBattle이 True를 반환.
	//잡몹이 다 죽음. -> 플레이어가 몬스터 감지 못함.
	//플레이어가 전투 끝났음을 알림.
	//브금 END로 변경?

private: 
	void Ready_BGM();

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	unordered_map<_uint, _wstring> m_BGMs;
	_wstring m_BattleBGM;

	_wstring m_szChangeBGMName;
	map<BOSSBGM, _wstring> m_BossBGM;
	BOSSBGM m_eLastBattle = { BOSSBGM::END };


	_bool m_IsCurBGMChange = { false };
	_float m_fBGMRate = {};
	_bool m_IsBattle = { false };

public:
	static CBGM_Manager* Create();
	virtual void Free()override;
};

NS_END