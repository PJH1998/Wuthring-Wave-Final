#pragma once
#include "Level.h"

NS_BEGIN(Client)
class CLoader;

class CLevel_Loading final : public CLevel
{
private:
	explicit CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT	Initialize(LEVEL eNextLevel);
	virtual void			Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;

private:
	CLoader*				m_pLoader = { nullptr };
	LEVEL					m_eNextLevel = { LEVEL::END };
	_bool					m_isFinished = { false };

private:
	HRESULT				Ready_Prototype();
	HRESULT				Ready_Event();
	HRESULT				Ready_LoadingThread();
	HRESULT				Ready_GameObject();

public:
	static		CLevel_Loading*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevel);
	virtual		void					Free() override;
};

NS_END