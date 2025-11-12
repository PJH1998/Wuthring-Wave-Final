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

private:
	class CGameSystem*		m_pGameSystem = { nullptr };
	LEVEL							m_eCurLevel = { LEVEL::LOGO };

public:
	static		CLevel_Logo*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END