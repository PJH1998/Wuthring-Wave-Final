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
	void			Ready_HavocWarrior();
	void			Ready_ElectroPredator();
	void			Ready_CoroSaurus();
	void			Ready_Effect();
	void			Ready_Skybox();
	void			Ready_UI();

#ifdef _DEBUG
private:
	void Shader_Gui();
	_float m_fRadius = {1.f};
	_float m_fMaxDistance = { 5.f };
	_float m_fBias[4] = {0.01f, 0.01f , 0.01f , 0.01f };
	_float m_fMinBias[4] = { 0.005f , 0.005f , 0.005f , 0.005f };
	_float m_fSlopeScale = { 2.f};
	_float m_fMapBias = {0.01f};

#endif

private:
	LEVEL m_eCurLevel = { LEVEL::GAMEPLAY };
	class CGameSystem* m_pGameSystem = { nullptr };
	_bool m_SonoroTest = { false };

public:
	static		CLevel_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END