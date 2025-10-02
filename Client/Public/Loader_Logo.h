#pragma once
#include "Loader.h"

NS_BEGIN(Client)

class CLoader_Logo final : public CLoader
{
private:
	explicit CLoader_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader_Logo() = default;

public:
	HRESULT				Initialize();
	virtual HRESULT	Loading();

private:
	HRESULT				Load_Texture();
	HRESULT				Load_Model();
	HRESULT				Load_Shader();
	HRESULT				Load_Object();

public:
	static		CLoader_Logo*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void					Free() override;
};

NS_END