#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CFactory final : public CBase
{
private:
	explicit CFactory(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CFactory() = default;

public:
	HRESULT						Initialize();

public:
	void							Create_MonsterDummy(LEVEL eLayerLevel, _float3 vPos, const _fmatrix& PreTransformationMatrix);

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

public:
	static		CFactory*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END