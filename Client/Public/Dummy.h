#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CRigidbody;
class CCollider;
NS_END

NS_BEGIN(Client)

class CDummy final : public CGameObject
{
private:
	explicit CDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CDummy(const CDummy& Prototype);
	virtual ~CDummy() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;
	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CShader*					m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CRigidbody*			m_pRigidbodyCom = { nullptr };
	CCollider*				m_pColliderCom = { nullptr };

private:
	void						Ready_Component();

public:
	static		CDummy*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END