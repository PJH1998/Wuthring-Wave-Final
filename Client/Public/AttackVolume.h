#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CAttackVolume final : public CPartObject
{
public:
	typedef struct tagAttackVolumeDesc : public CPartObject::PART_DESC
	{
		const _float4x4* pSocketMatrix;
		_float3 vExtent;
		function<void()> CollisionCallback;
	}ATKVOLUME_DESC;

private:
	explicit CAttackVolume(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAttackVolume(const CAttackVolume& Prototype);
	virtual ~CAttackVolume() = default;

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

private:
	const _float4x4* m_pSocketMatrix = { nullptr };
	CRigidbody* m_pRigidBodyCom = { nullptr };

	function<void()> m_CollisionCallback;
private:
	void Ready_Component(ATKVOLUME_DESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CAttackVolume*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*			Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
