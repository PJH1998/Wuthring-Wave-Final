#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CRigidbody final : public CComponent
{
public:
	typedef struct tagRigidbodyDesc {
		class CGameObject* pOwner = { nullptr };
		SHAPE			eShape;
		_float3			vPos;
		_float4			vQuat = _float4(0.f, 0.f, 0.f, 1.f);
		EMotionType	eType;
		_uint				iLayer;
	}RIGIDBODY_DESC;

	typedef struct tagBoxBodyDesc : public RIGIDBODY_DESC {
		_float3			vExtent;
	}BOXBODY_DESC;

private:
	explicit CRigidbody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRigidbody(const CRigidbody& Prototype);
	virtual ~CRigidbody() = default;

public:
	virtual		HRESULT			Initialize_Prototype()			override;
	virtual		HRESULT			Initialize_Clone(void* pArg)	override;
	void							Update();

public:
	void							AddForce(const _float3& vForce) { 
		m_pBodyInterface->ActivateBody(m_pBody->GetID());
		m_pBodyInterface->AddForce(m_pBody->GetID(), LoadVec3(vForce)); 
		//m_pBodyInterface->AddImpulse(m_pBody->GetID(), LoadVec3(vForce)); 
	}
	void							OnGravity(_bool isGravity) { m_pBodyInterface->SetGravityFactor(m_pBody->GetID(), isGravity); }

private:
	class CGameObject*		m_pOwner = { nullptr };
	Body*							m_pBody = {nullptr};
	BodyInterface*				m_pBodyInterface = { nullptr };

private:
	Vec3 LoadVec3(const _float3& vVector)
	{
		Vec3 vVec3 = Vec3(vVector.x, vVector.y, vVector.z);
		return vVec3;
	}

public:
	static		CRigidbody*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*	Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END