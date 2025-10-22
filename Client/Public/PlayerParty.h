#pragma once
#include "Player_Define.h"
#include "GameObject.h"

NS_BEGIN(Client)
// Player Container들을 관리 감독하는 컨트롤러
class CPlayerParty final : public CGameObject
{
public:
	enum PLAYERTYPE
	{
		AUGUSTA = 0,
		GALBRENA = 1,
		PLAYER = 2,
		TYPE_END
	};

public:
	typedef struct tagPlayerPartyDesc
	{
		LEVEL eCurLevel = {LEVEL::END };
		_uint iPlayerCount = {};
		vector<PLAYER_SPEC> PlayerSpecs = {};
	}PLAYER_PARTY_DESC;

#pragma region 기본 함수들
public:
	explicit CPlayerParty(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPlayerParty(const CPlayerParty& Prototype);
	virtual ~CPlayerParty() = default;


public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_Shadow() override;

#pragma endregion

public:
	// 협주게이지?
	void Ensemble_Skill(PLAYERTYPE iPlayerType);

private:
	vector<CPlayer*> m_Players; // 연주자들

	LEVEL m_eCurLevel = { LEVEL::END };
	_uint m_iCurrentPlayerIdx = {};
	_uint m_iPrevPlayerIdx = {};

private:
	HRESULT Ready_Players(const PLAYER_PARTY_DESC* pDesc);

public:
	static		CPlayerParty* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;


};
NS_END

