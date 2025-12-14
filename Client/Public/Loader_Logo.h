#pragma once
#include "Loader.h"
#include "Custom_UI.h"

NS_BEGIN(Client)

class CLoader_Logo final : public CLoader
{
private:
	explicit CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader_Logo() = default;

public:
	virtual		HRESULT		Initialize() override;

private:
	HRESULT				Load_Texture();
	HRESULT				Load_Model();
	HRESULT				Load_Shader();
	HRESULT				Load_Object();
	HRESULT				Load_LogoMaleRover();
	HRESULT				Load_LogoFeMaleRover();
	HRESULT				Load_Effect();


	HRESULT				Load_UI();
private:
	LEVEL m_eCurLevel = { LEVEL::LOGO };

	HRESULT				Load_MonsterTable();

private:
	CCustom_UI::CUSTOM_UITREE_DESC	Load_UITree(_string strFilePath);

public:
	static		CLoader_Logo*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END