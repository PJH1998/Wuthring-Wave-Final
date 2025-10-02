#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CMainApp final : public CBase
{
private:
	explicit CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT			Initialize();
	void				Post_Update();						// 레벨 전환
	void				Update(_float fTimeDelta);
	HRESULT			Render();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_bool							m_isChangeLevel = { false };
	LEVEL							m_eNextLevel = { LEVEL::END };
	_bool							m_isLoad = { false };

	// Frame 확인용
	//_float							m_fMinute = {};
	//_uint							m_iFrame = {};

private:
	void			Start_Level();

public:
	static		CMainApp* Create();
	virtual		void			Free() override;
};

NS_END