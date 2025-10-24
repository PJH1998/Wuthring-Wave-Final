#pragma once
#include "Loader.h"

NS_BEGIN(Client)

class CLoader_Test_UI final : public CLoader
{
private:
	explicit CLoader_Test_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader_Test_UI() = default;

public:
	virtual		HRESULT		Initialize() override;

private:
	HRESULT				Load_Texture();
	HRESULT				Load_Model();
	HRESULT				Load_Shader();
	HRESULT				Load_Object();

public:
	static		CLoader_Test_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END