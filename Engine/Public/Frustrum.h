#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CFrustrum final : public CBase
{
private:
	explicit CFrustrum();
	virtual ~CFrustrum() = default;

public:
	HRESULT						Initialize();
	void							Update();

private:
	class CGameInstance*		m_pGameInstance = { nullptr };

public:
	static		CFrustrum*		Create();
	virtual		void				Free() override;
};

NS_END