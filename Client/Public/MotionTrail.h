#pragma once
#include "Component.h"

NS_BEGIN(Client)

class CMotionTrail final : public CComponent
{
private:
	CMotionTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMotionTrail(const CMotionTrail& Prototype);
	virtual ~CMotionTrail() = default;


public:
	static CMotionTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END