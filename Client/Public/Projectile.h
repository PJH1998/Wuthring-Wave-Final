#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CProjectile final : public CGameObject
{
public:
	typedef struct tagProjectileDesc
	{
		_wstring	wstrEffectTag;
		_uint		iLayer;
		_uint		iTargetLayer;
		_float		fRadius;
		_float3		vExtents;
		_float3		vTargetPos;
	}PROJECTILEDESC;
private:
	explicit CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CProjectile(const CProjectile& Prototype);
	virtual ~CProjectile() = default;

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	CRigidbody* m_pRigidBodyCom = { nullptr };
	// Effect?

private:
	void Ready_Component(PROJECTILEDESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CProjectile* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
