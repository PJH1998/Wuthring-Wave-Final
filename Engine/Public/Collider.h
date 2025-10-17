#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
public:
	typedef struct tagColliderDesc {
		class CGameObject* pOwner = { nullptr };
		_float3			vPos;
		_float4			vQuat = _float4(0.f, 0.f, 0.f, 1.f);
		EMotionType	eType;
		_uint				iLayer;
		_float				fHeight;		// Ä¸½¶ ¸öÅë ³ôÀÌ
		_float				fRadius;		// Ä¸½¶ ±¸ ºÎºÐ ¹ÝÁö¸§
	}COLLIDER_DESC;

private:
	explicit CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCollider(const CCollider& Prototype);
	virtual ~CCollider() = default;

public:
	void								Sync_Position(class CTransform* pTransform);

	_bool								IsLand(_float3* pNormalOut = nullptr);

	void								Set_Gravity(_bool isGravity) { m_isGravity = isGravity; }

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
	void								Update(const _fvector& vVelocity);
	virtual		HRESULT				Render() override;

private:
	class CGameObject*			m_pOwner = { nullptr };
	CharacterVirtual*				m_pCharacterVirtual = { nullptr };

	_uint								m_iCollisionLayer = {};

	_bool								m_isGravity = { true };

private:
	Vec3								Slide(const Vec3& Velocity);

public:
	static		CCollider*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END