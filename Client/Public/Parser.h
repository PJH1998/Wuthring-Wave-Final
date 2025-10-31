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
	void							Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel);
	void							Clone_MapObjects(LEVEL eLevel, _uint iIndex);
	// Load CSV File (Excel)
	const vector<vector<_string>>&	Load_CSV(const _char* pFilePath);

private:
	void							Read_Map_Prototype(const _string pFilePath, LEVEL eLevel);
	void							Read_Map_Dat(LEVEL eLevel, const _string pFilePath);
public:
	HRESULT						Initialize();

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	vector<vector<_string>> m_Data;
	unordered_map<LEVEL, vector<const _char*>> m_LoadingMap;

public:
	static		CParser*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END