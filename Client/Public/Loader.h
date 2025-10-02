#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
NS_END

NS_BEGIN(Client)

class CLoader abstract : public CBase
{
protected:
	explicit CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public:
	_bool		IsFinished() { return m_fProgress >= 100.f; }
	_float		Get_Progress() { return m_fProgress; }

public:
	HRESULT				Initialize();
	virtual HRESULT	Loading() = 0;

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance*			m_pGameInstance = { nullptr };

	_float							m_fProgress = {};

	HANDLE						m_hThread = {};
	CRITICAL_SECTION		m_CriticalSection = {};

public:
	virtual		void			Free() override;
};

NS_END