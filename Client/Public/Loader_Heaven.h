#pragma once
#include "Loader.h"

#include "Custom_UI.h"

NS_BEGIN(Client)

class CLoader_Heaven final : public CLoader
{
private:
	explicit CLoader_Heaven(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader_Heaven() = default;

public:
	virtual		HRESULT		Initialize() override;

private:
	HRESULT				Load_Texture();
	HRESULT				Load_Model();
	HRESULT				Load_Shader();
	HRESULT				Load_Object();
	HRESULT				Load_Monster();
	HRESULT				Load_Leviatan();

	HRESULT				Load_Player();
	HRESULT				Load_Augusta();
	HRESULT				Load_Rover();
	HRESULT				Load_Galbrena();


	HRESULT				Load_SequencePlayer();
	HRESULT				Load_Yuno();
	HRESULT				Load_SequenceAugusta();
	HRESULT				Load_SequenceLupa();




	HRESULT				Load_UI();
	HRESULT				Load_Font();
	HRESULT				Load_Effect();
	HRESULT				Load_ScreenEffect();

private:
	CCustom_UI::CUSTOM_UITREE_DESC Load_UITree(_string strFilePath);

private:
	LEVEL m_eCurLevel = { LEVEL::HEAVEN };

public:
	static		CLoader_Heaven*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END