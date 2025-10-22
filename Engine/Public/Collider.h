#pragma once
#include "CollideComponent.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCollider final : public CCollideComponent
{
public:
	typedef struct tagColliderDesc {
		_float3			vPos;
		_float4			vQuat = _float4(0.f, 0.f, 0.f, 1.f);
		EMotionType	eType;
		_uint				iLayer;
		_float				fHeight;		// Capsule Height
		_float				fRadius;		// Capsule Radius
	}COLLIDER_DESC;

private:
	explicit CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCollider(const CCollider& Prototype);
	virtual ~CCollider() = default;

public:
	// Datat (void*)
	void								Set_Desc(void* pData) { m_tCollisionData.pDesc = pData; }
	// Collider 움직임 -> Transform에 적용
	void								Sync_Position(class CTransform* pTransform);
	// 땅을 타고 있는지 Check
	_bool								IsLand(_float3* pNormalOut = nullptr);
	// Gravity On/Off
	void								Set_Gravity(_bool isGravity) { m_isGravity = isGravity; }

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
	void								Update(const _fvector& vVelocity);
	virtual		HRESULT				Render() override;

private:
	Ref<CharacterVirtual>		m_pCharacterVirtual = { nullptr };
	COLLISION_DATA				m_tCollisionData = {};

	_uint								m_iCollisionLayer = {};

	_bool								m_isGravity = { true };

private:
	// 경사로에서 이동속도 변화 -> Slide로 보정
	Vec3								Slide(const Vec3& Velocity);

public:
	static		CCollider*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END