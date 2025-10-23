#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CGameSystem final : public CBase
{
	DECLARE_SINGLETON(CGameSystem)
private:
	explicit CGameSystem();
	virtual ~CGameSystem() = default;

public:
#pragma region GameSystem
	void		Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
#pragma endregion

#pragma region Parser
	const vector<vector<_string>>& Load_CSV(const _char* pFilePath);
	void							Create_Map_Model(const _char* pFilePath, LEVEL eLevel);
#pragma endregion



private:
	class		CParser*		m_pParser = { nullptr };

public:
	virtual		void	Free() override;

};

NS_END