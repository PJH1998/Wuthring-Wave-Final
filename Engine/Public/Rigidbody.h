#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CRigidbody final : public CComponent
{
public:
	enum BODYTYPE { BODY, CHARACTER, VIRTUAL };
public:
#pragma region DESC
	typedef struct tagRigidbodyDesc {
		class CGameObject* pOwner = { nullptr };
		SHAPE			eShape;
		_float3			vPos;
		_float4			vQuat = _float4(0.f, 0.f, 0.f, 1.f);
		EMotionType	eType;
		_uint				iLayer;
		BODYTYPE		eBodyType = { BODYTYPE::BODY };
	}RIGIDBODY_DESC;

	typedef struct tagSphereBodyDesc : public RIGIDBODY_DESC {
		_float				fRadius; // 援?諛섏?由?
	}SPHEREBODY_DESC;

	typedef struct tagBoxBodyDesc : public RIGIDBODY_DESC {
		_float3			vExtent; // Box 媛?異?蹂?諛섏?由?
	}BOXBODY_DESC;

	typedef struct tagCapsuleBodyDesc : public RIGIDBODY_DESC {
		_float				fHeight; // 罹≪뒓 紐명넻 ?믪씠
		_float				fRadius; // 罹≪뒓 援?遺遺?諛섏?由?
	}CAPSULEBODY_DESC;

	typedef struct tagConvexHullBodyDesc : public RIGIDBODY_DESC {
		class CModel* pModel = { nullptr };
	}CONVEXHULLBODY_DESC;

	typedef struct tagMeshBodyDesc : public RIGIDBODY_DESC {
		class CModel* pModel = { nullptr };
	}MESHBODY_DESC;
#pragma endregion

private:
	explicit CRigidbody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRigidbody(const CRigidbody& Prototype);
	virtual ~CRigidbody() = default;

public:
	virtual		HRESULT			Initialize_Prototype()			override;
	virtual		HRESULT			Initialize_Clone(void* pArg)	override;
	virtual		HRESULT			Render() override;

	void							Update_Rigidbody(const _fmatrix& Matrix, _float fTimeDelta);
	void							Sync_Rigidbody(class CTransform* pTransform);

public:
	void							Activate(_bool isActivate) { true == isActivate ? m_pBodyInterface->ActivateBody(m_BodyID) : m_pBodyInterface->DeactivateBody(m_BodyID); }
	void							OnGravity(_bool isGravity) { m_pBodyInterface->SetGravityFactor(m_BodyID, isGravity); }

	void							Force(const _float3& vForce) { m_pBodyInterface->AddForce(m_BodyID, LoadVec3(vForce)); }
	void							Impulse(const _float3& vForce) { m_pBodyInterface->AddImpulse(m_BodyID, LoadVec3(vForce)); }

	_bool							IsLand(_float3* pNormalOut = nullptr);

private:
	class CGameObject*		m_pOwner = { nullptr };
	Body*							m_pBody = {nullptr};
	BodyID						m_BodyID;
	BodyInterface*				m_pBodyInterface = { nullptr };

	CharacterVirtual*			m_pCharacterVirtual = { nullptr };
	Character*					m_pCharacter = { nullptr };
	//Ref<Character>			m_pCharacter = { nullptr };

private:
	const JPH::Array<Vec3>					ConvertToArrayVec3(class CModel* pModel);
	const JPH::Array<Float3>				ConvertToArrayFloat3(class CModel* pModel, _uint iIndex);
	const JPH::Array<IndexedTriangle>	ConvertToArrayTri(class CModel* pModel, _uint iIndex);

private:
	void							Make_MeshShape(void* pArg);

	void							Ready_Body(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape);
	void							Ready_Character(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape);
	void							Ready_Virtual(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape);

public:
	static		CRigidbody*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*	Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END