#pragma once
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Logo final : public CLevel
{
private:
	explicit CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Logo() = default;

public:
	virtual		HRESULT			Initialize() override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Render() override;

private:
	void							Ready_Camera();
	void							Ready_Layer_LogoMaleRover();
	void							Ready_Layer_LogoFemaleRover();

	void							Ready_UI();
	void							Ready_Mouse();
	void							Ready_SkyBox();

private:
#ifdef _DEBUG
	void							Update_SoundOrder(_float fTimeDelta);
	void							Update_ClickSound();
	void							Update_GoinFinish(_float fTimeDelta);
#endif // _DEBUG

	

private:
	class CGameSystem*				m_pGameSystem = { nullptr };
	LEVEL							m_eCurLevel = { LEVEL::LOGO };

#ifdef _DEBUG

private:
	_float m_fMinStep = { 5.f };
	_float m_fMaxStep = { 20.f };
	_float m_fStart = { 10.f };

	_bool			m_isReqedFinish = false;

	// ========== for Sound.. ==========
	_float			m_fElapsedTime = 0.f;
	const _float	m_fLoginStartTime = 7.f;
	_uint			m_iSoundOrder = 0;

	_float			m_fElapsedFinishTime = 0.f;
	const _float	m_fFinishTime = 2.5f;
	_bool			m_isGoinFinish = false;
	// ==============================

private:
	void DEBUG_FUNCTION();

#endif

public:
	static		CLevel_Logo*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END