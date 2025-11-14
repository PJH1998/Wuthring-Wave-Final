#pragma once
#include "Interface_Edit.h"

NS_BEGIN(Editor)


class CSFX_Interface final : public CInterface_Edit
{
private:
	explicit CSFX_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CSFX_Interface() = default;

public:
	virtual	HRESULT			Initialize();

public:
	static CSFX_Interface*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;
};

NS_END