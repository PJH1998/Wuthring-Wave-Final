#pragma once
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Logo final : public CLevel
{
private:
	explicit CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Logo() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	// Test
	Engine::CRigidbody*		m_pRigidbody1 = { nullptr };
	Engine::CRigidbody*		m_pRigidbody2 = { nullptr };
	Engine::CRigidbody*		m_pRigidbody3 = { nullptr };

public:
	static		CLevel_Logo*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END