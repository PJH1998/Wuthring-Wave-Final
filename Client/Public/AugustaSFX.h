#pragma once
#include "ScreenEffect.h"

NS_BEGIN(Client)

class CAugustaSFX final : public CScreenEffect
{
private:
	CAugustaSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAugustaSFX(const CAugustaSFX& Prototype);
	virtual ~CAugustaSFX() = default;



public:
	static CAugustaSFX*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);                           
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END