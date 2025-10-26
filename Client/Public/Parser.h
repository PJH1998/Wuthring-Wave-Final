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
	// Load CSV File (Excel)
	const vector<vector<_string>>&	Load_CSV(const _char* pFilePath);

private:
	void							Read_Map_Prototype(const _string pFilePath, LEVEL eLevel);

public:
	HRESULT						Initialize();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	vector<vector<_string>> m_Data;

public:
	static		CParser*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END