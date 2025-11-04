#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CAttackVolume final : public CGameObject
{
public:
	typedef struct tagAttackVolumeDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4* pSocketMatrix;
		const _float4x4* pParentMatrix;
		
		SHAPE	eShape;
		COLLISIONLAYER eLayer;
		COLLISIONLAYER eTargetLayer;
		_float3 vExtent;
		_float3 vOffsetPos;
		_float3 vOffsetRadian;
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

public:
	void TriggerActivate(_bool isActivate);

private:
	const _float4x4*	m_pSocketMatrix = { nullptr };
	const _float4x4*	m_pParentMatrix = { nullptr };
	_float4x4			m_CombinedMatrix{};
	CRigidbody*			m_pRigidBodyCom = { nullptr };

	_float3 m_vOffsetPos{};
	_float3 m_vOffsetRot{};
	COLLISIONLAYER m_eTargetLayer{COLLISIONLAYER::NONE};

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
