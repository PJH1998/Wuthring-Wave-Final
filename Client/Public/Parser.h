#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CParser final : public CBase
{
private:
	explicit CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CParser() = default;

public:
	// File Model
	void							Create_Map_Model(const _char* pFilePath, LEVEL eLevel);

	void							Load_CSV(const _char* pFilePath);

public:
	HRESULT						Initialize();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

public:
	static		CParser*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END