#pragma once
#include "Loader.h"

#include "Custom_UI.h"

NS_BEGIN(Client)

class CLoader_GamePlay final : public CLoader
{
private:
	explicit CLoader_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader_GamePlay() = default;

public:
	virtual		HRESULT		Initialize() override;

private:
	HRESULT				Load_Texture();
	HRESULT				Load_Model();
	HRESULT				Load_Shader();
	HRESULT				Load_Object();
	HRESULT				Load_MonsterTest();
	HRESULT				Load_Monster();
	HRESULT				Load_Production();
	HRESULT				Load_NPC();
	HRESULT				Load_Hide_And_Seek();

	HRESULT				Load_Player();
	HRESULT				Load_Augusta();
	HRESULT				Load_Rover();
	HRESULT				Load_Galbrena();


	HRESULT				Load_UI();
	HRESULT				Load_Font();
	HRESULT				Load_Effect();
	HRESULT				Load_ScreenEffect();

private:
	CCustom_UI::CUSTOM_UITREE_DESC Load_UITree(_string strFilePath);

private:
	LEVEL m_eCurLevel = { LEVEL::GAMEPLAY };

public:
	static		CLoader_GamePlay*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END