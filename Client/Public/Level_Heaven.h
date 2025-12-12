#pragma once
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Heaven final : public CLevel
{
private:
	explicit CLevel_Heaven(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Heaven() = default;

public:
	virtual		HRESULT		Initialize() override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Render() override;

private:
	void 						Ready_Layer_Player();
	void 						Ready_Layer_SequnecePlayer();
	void						Ready_Dummy();
	void						Ready_MonsterTest();
	void						Ready_HavocWarrior();
	void						Ready_ElectroPredator();
	void						Ready_CoroSaurus();
	void						Ready_Leviatan();
	void						Ready_Effect();
	void						Ready_Skybox();
	void						Ready_UI();
	void						Ready_SFX();
	void						Ready_Scene();

#ifdef _DEBUG
private:
	void DEBUG_FUNCTION();

	_int		m_iLUT_Index = { 0 };
	_float		m_fLUT_Intensity = {};
	LIGHT_DESC	m_tLightDesc = {};
	
#endif

private:
	LEVEL m_eCurLevel = { LEVEL::HEAVEN };
	class CGameSystem* m_pGameSystem = { nullptr };
	_bool m_SonoroTest = { false };

public:
	static		CLevel_Heaven* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END