#pragma once
#include "Level.h"

NS_BEGIN(Client)

class CLevel_GamePlay :
    public CLevel
{
private:
	explicit CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_GamePlay() = default;

public:
	virtual		HRESULT		Initialize() override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Render() override;

private:
	void 			Ready_Layer_Player();
	void			Ready_Dummy();
	void			Ready_MonsterTest();



private:
	LEVEL m_eCurLevel = { LEVEL::GAMEPLAY };
	class CGameSystem* m_pGameSystem = { nullptr };

public:
	static		CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END