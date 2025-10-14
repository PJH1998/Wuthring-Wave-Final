#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CParser final : public CBase
{
	DECLARE_SINGLETON(CParser)
private:
	explicit CParser();
	virtual ~CParser() = default;

public:
	// File°æ·Î, 
	void							Create_Map_Model(const _char* pFilePath, LEVEL eLevel);

private:
	class CGameInstance*	m_pGameInstance = { nullptr };

public:
	virtual		void				Free() override;
};

NS_END