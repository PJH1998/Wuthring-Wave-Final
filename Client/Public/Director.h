#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CDirector final : public CBase
{
private:
	explicit CDirector();
	virtual ~CDirector() = default;

public:
	void		Play_Action(const _wstring& strActionTag, _bool isMaintain); // Tag / true : 유지, false : 끝나면 자동 Recovery

private:
	class CGameInstance*	m_pGameInstance = { nullptr };

public:
	static		CDirector*	Create();
	virtual		void			Free();
};

NS_END