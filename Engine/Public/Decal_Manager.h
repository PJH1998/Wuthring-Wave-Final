#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CDecal_Manager final : public CBase
{
private:
	CDecal_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal_Manager() = default;

public:
	static CDecal_Manager*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;
};

NS_END