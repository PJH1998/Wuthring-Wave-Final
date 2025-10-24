#pragma once
#include "Base.h"

NS_BEGIN(Editor)

class CLoad_Controller final : public CBase
{

private:
	explicit CLoad_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoad_Controller() = default;

#pragma region 기본
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//다른곳에서 이용할 때면 텍스처, 매쉬 등 다양한 얘들 원형이 필요할거 같음.
	//void Load_AllTextureFromFolder(const _string& strFolderPath);
	//void Load_AllMeshDatFromFolder(const _string& strFolderPath);
	//void Load_AllColorTextureFormFolder(const _string& strFolderPath);

public:
	void Prefab_Load_Tab();

	void Load_Prefab_FromJson(const _string& strFilePath);

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };

public:
	static CLoad_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END