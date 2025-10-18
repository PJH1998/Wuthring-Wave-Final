#pragma once
#include "Interface_Edit.h"

NS_BEGIN(Editor)

class CShader_Interface final : public CInterface_Edit
{
private:
	explicit CShader_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShader_Interface() = default;

public:
	virtual	HRESULT		Initialize();
	void				Update_Shadow();
	

private:
	_float				m_fBias[4] = {};

private:
	void				Set_ShadowBias();

public:
	static CShader_Interface* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END