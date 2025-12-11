#pragma once
#include "Base.h"
NS_BEGIN(Client)
class CBGM_Manager final : public CBase
{
public:
	enum BGMLEVEL { ASPHODEL, SOERVERIGN, HEAVEN, END };
private:
	explicit CBGM_Manager();
	virtual ~CBGM_Manager() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);


	void Ready_BGM();


	void Change_BGM(_bool IsBattle);
private:
	class CGameInstance* m_pGameInstance = { nullptr };
	unordered_map<_uint, vector<_wstring>> m_BGMs;
	_wstring m_CurBGM;
	_wstring m_BattleBGM;
	_bool m_IsCurBGMChange = { false };
	_float m_fLerpTime = {};
	_float m_fBGMRate = {};
	_bool m_IsBattle = { false };

public:
	static CBGM_Manager* Create();
	virtual void Free()override;
};

NS_END