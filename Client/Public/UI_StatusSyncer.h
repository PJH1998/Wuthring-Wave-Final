#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CUI_StatusSyncer final : public CBase
{
private:
	explicit CUI_StatusSyncer();
	virtual ~CUI_StatusSyncer() = default;

public:
	HRESULT Initialize();

private:
	class CGameInstance*	m_pGameInstance			= { nullptr };

	class CUI_HUD*			m_pRootUI_HUD				= { nullptr };

public:
	static CUI_StatusSyncer* Create();
	virtual void Free() override;
};

NS_END
