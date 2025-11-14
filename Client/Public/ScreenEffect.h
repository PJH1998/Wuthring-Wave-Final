#pragma once
#include "GameObject.h"

NS_BEGIN(Client)

class CScreenEffect abstract : public CGameObject
{
protected:
	CScreenEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CScreenEffect(const CScreenEffect& Prototype);
	virtual ~CScreenEffect() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

protected:
	_float4x4					m_ViewMatrix = {};
	_float4x4					m_ProjMatrix = {};

	CVIBuffer_Rect*				m_pVIBuffer_Rect = { nullptr };
	CShader*					m_pShader = { nullptr };

private:
	HRESULT					Ready_Components();

public:
	virtual CGameObject*	Clone(void* pArg) PURE;
	virtual void			Free() override;
};

NS_END