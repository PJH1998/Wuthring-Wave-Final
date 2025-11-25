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
	void 						Ready_Layer_Player();
	void						Ready_Dummy();
	void						Ready_MonsterTest();
	void						Ready_HavocWarrior();
	void						Ready_ElectroPredator();
	void						Ready_CoroSaurus();
	void						Ready_Effect();
	void						Ready_Skybox();
	void						Ready_UI();
	void						Ready_Mouse();
	void						Ready_SFX();
	void						Ready_NPC();

#ifdef _DEBUG
private:
	void DEBUG_FUNCTION();
	_float m_fRadius = {1.f};
	_float m_fMaxDistance = { 5.f };
	_float m_fBias[4] = {0.01f, 0.01f , 0.01f , 0.01f };
	_float m_fMinBias[4] = { 0.005f , 0.005f , 0.005f , 0.005f };
	_float m_fSlopeScale = { 2.f};
	_float m_fMapBias = {0.01f};
	_int m_iLUT_Index = { 0 };
	_float m_fLUT_Intensity = {};

	_float  m_fLimitVelocity = {15.f};
	_float	m_fLimitDepth = {150.f};
	_float	m_fLengthScale = {5.f};
	_bool	m_IsDyanmicLUT = { false };
	_bool	m_IsSSS = { true };
	_float	m_fExposure = { 0.6f };

	_float m_fMinStep = { 5.f };
	_float m_fMaxStep = { 20.f };
	_float m_fStart = { 10.f };
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