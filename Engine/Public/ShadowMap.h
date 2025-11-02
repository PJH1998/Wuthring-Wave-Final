#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CShadowMap final : public CBase
{
private:
	explicit CShadowMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShadowMap() = default;



public:
	static CShadowMap*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void		Free() override;
};

NS_END