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
	//_bool		IsFinished() { return m_fProgress >= 100.f; }
	_float		Get_Progress() { return m_fProgress; }

public:
	virtual			HRESULT		Initialize() { return S_OK; };

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*			m_pGameInstance = { nullptr };
	class CParser*				m_pParser = { nullptr };

	_float							m_fProgress = {};

	mutex							m_Mutex;

protected:
	void					Complete_Load();

public:
	virtual		void			Free() override;
};

NS_END
