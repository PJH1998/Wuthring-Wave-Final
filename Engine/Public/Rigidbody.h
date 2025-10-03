#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CRigidbody final : public CComponent
{
private:
	explicit CRigidbody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRigidbody(const CRigidbody& Prototype);
	virtual ~CRigidbody() = default;

public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	void							Update();

private:
	BodyID						m_BodyID = {};

public:
	static		CRigidbody*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*	Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END