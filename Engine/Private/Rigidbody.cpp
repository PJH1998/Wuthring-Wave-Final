#include "EnginePch.h"
#include "Rigidbody.h"

#include "GameInstance.h"
#include "GameObject.h"

#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"

CRigidbody::CRigidbody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CRigidbody::CRigidbody(const CRigidbody& Prototype)
	: CComponent { Prototype }
{
}

HRESULT CRigidbody::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRigidbody::Initialize_Clone(void* pArg)
{
	ASSERT_CRASH(pArg);

	RIGIDBODY_DESC* pDesc = static_cast<RIGIDBODY_DESC*>(pArg);
	m_pOwner = pDesc->pOwner;

	RefConst<Shape> BodyShape;

	using namespace JPH;
	switch (pDesc->eShape)
	{
	case SHAPE::SPHERE:
	{
		SPHEREBODY_DESC* pSphereDesc = static_cast<SPHEREBODY_DESC*>(pDesc);
		BodyShape = new SphereShape(pSphereDesc->fRadius);
		break;
	}
	case SHAPE::BOX:
	{
		BOXBODY_DESC* pBoxDesc = static_cast<BOXBODY_DESC*>(pDesc);
		BodyShape = new BoxShape(Vec3(pBoxDesc->vExtent.x, pBoxDesc->vExtent.y, pBoxDesc->vExtent.z));
		break;
	}
	case SHAPE::CAPSULE:
	{
		CAPSULEBODY_DESC* pCapsuleDesc = static_cast<CAPSULEBODY_DESC*>(pDesc);
		BodyShape = new CapsuleShape(pCapsuleDesc->fHeight * 0.5f, pCapsuleDesc->fRadius);
		break;
	}
	case SHAPE::CONVEXHULL:
	{
		CONVEXHULLBODY_DESC* pConvexHullDesc = static_cast<CONVEXHULLBODY_DESC*>(pDesc);
		ASSERT_CRASH(pConvexHullDesc->pModel);
		Ref<ConvexHullShapeSettings> ConvexHullSetting = new ConvexHullShapeSettings(ConvertToArrayVec3(pConvexHullDesc->pModel));
		BodyShape = ConvexHullSetting->Create().Get();
		break;
	}
	case SHAPE::MESH:
	{
		// Mesh는 따로 처리
		Make_MeshShape(pArg);
		return S_OK;
	}
	default:
		CRASH("Shape Error");
	}

	if (false == pDesc->isCharacter)
		Ready_Body(pDesc, BodyShape);
	else
		Ready_Character(pDesc, BodyShape);
	
	return S_OK;
}

HRESULT CRigidbody::Render()
{
	if(nullptr != m_pBody)
		m_pGameInstance->DrawShape(m_pBody->GetShape());
	if (nullptr != m_pCharacter)
		m_pGameInstance->DrawShape(m_pCharacter->GetShape());

	return S_OK;
}

void CRigidbody::Update_Rigidbody(const _fmatrix& Matrix, _float fTimeDelta)
{
	_vector vScale{}, vRotation{}, vTranslation{};

	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, Matrix);

	m_pBodyInterface->MoveKinematic(m_BodyID, LoadVec3(vTranslation), LoadQuat(vRotation), fTimeDelta);
}

void CRigidbody::Sync_Rigidbody(CTransform* pTransform)
{
	Vec3 vPos;
	Quat vRotation;
	m_pBodyInterface->GetPositionAndRotation(m_BodyID, vPos, vRotation);

	_vector vQuaternion = XMVectorSet(vRotation.GetX(), vRotation.GetY(), vRotation.GetZ(), vRotation.GetW());
	pTransform->Quaternion(vQuaternion);
	pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
}

_bool CRigidbody::IsLand(_float3* pNormalOut)
{
	if (nullptr == m_pCharacter)
		return false;

	if (nullptr != pNormalOut)
	{
		Vec3 vNormal = m_pCharacter->GetGroundNormal();
		*pNormalOut = _float3(vNormal.GetX(), vNormal.GetY(), vNormal.GetZ());
	}

	return m_pCharacter->IsSupported();
}

const JPH::Array<Vec3> CRigidbody::ConvertToArrayVec3(CModel* pModel)
{
	JPH::Array<Vec3> Vertices;

	vector<_float3> ModelVertices = pModel->Get_VerticesPos(0);

	for (size_t i = 0; i < ModelVertices.size(); ++i)
		Vertices.push_back(LoadVec3(ModelVertices[i]));

	return Vertices;
}

const JPH::Array<Float3> CRigidbody::ConvertToArrayFloat3(CModel* pModel, _uint iIndex)
{
	JPH::Array<Float3> Vertices;

	vector<_float3> ModelVertices = pModel->Get_VerticesPos(iIndex);

	for (size_t i = 0; i < ModelVertices.size(); ++i)
		Vertices.push_back(Float3(ModelVertices[i].x, ModelVertices[i].y, ModelVertices[i].z));

	return Vertices;
}

const JPH::Array<IndexedTriangle> CRigidbody::ConvertToArrayTri(CModel* pModel, _uint iIndex)
{
	JPH::Array<IndexedTriangle> Indices;

	vector<_uint> ModelIndices = pModel->Get_Indices(iIndex);

	for (size_t i = 0; i < ModelIndices.size(); i += 3)
		Indices.push_back(IndexedTriangle(ModelIndices[i], ModelIndices[i + 1], ModelIndices[i + 2]));

	return Indices;
}

void CRigidbody::Make_MeshShape(void* pArg)
{
	MESHBODY_DESC* pDesc = static_cast<MESHBODY_DESC*>(pArg);

	_uint iNumMesh = pDesc->pModel->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		RefConst<Shape> BodyShape;

		Ref<MeshShapeSettings> MeshSetting;
		MeshSetting = new MeshShapeSettings(ConvertToArrayFloat3(pDesc->pModel, i), ConvertToArrayTri(pDesc->pModel, i));
		BodyShape = MeshSetting->Create().Get();

		BodyCreationSettings bodySetting(
			BodyShape,																					// Shape
			Vec3(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z),								// Position
			Quat(pDesc->vQuat.x, pDesc->vQuat.y, pDesc->vQuat.z, pDesc->vQuat.w),	// Quat
			pDesc->eType,																				// Motion Type
			ObjectLayer(pDesc->iLayer)																// Collision Layer
		);

		ASSERT_CRASH(m_pGameInstance->Register_Body(bodySetting, &m_pBodyInterface));
	}
}

void CRigidbody::Ready_Body(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape)
{
	BodyCreationSettings bodySetting(
		BodyShape,									// Shape
		Vec3(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z),				// Position
		Quat(pDesc->vQuat.x, pDesc->vQuat.y, pDesc->vQuat.z, pDesc->vQuat.w),	// Quat
		pDesc->eType,								// Motion Type
		ObjectLayer(pDesc->iLayer)				// Collision Layer
	);
	MassProperties mp;
	mp.ScaleToMass(1.f);

	bodySetting.mMassPropertiesOverride = mp;
	// 관성 (직접 설정한 질량 사용하는 세팅)
	bodySetting.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;

	// GameObject(Owner) -> UserData로 전달
	bodySetting.mUserData = reinterpret_cast<uint64>(m_pOwner);

	m_pBody = m_pGameInstance->Register_Body(bodySetting, &m_pBodyInterface);
	m_BodyID = m_pBody->GetID();
}

void CRigidbody::Ready_Character(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape)
{
	CharacterSettings CharacterSetting;
	CharacterSetting.mLayer = ObjectLayer(pDesc->iLayer);
	CharacterSetting.mFriction = 1.f;
	CharacterSetting.mGravityFactor = 0.f;
	CharacterSetting.mShape = BodyShape;

	m_pCharacter = m_pGameInstance->Register_Character(CharacterSetting, LoadVec3(pDesc->vPos), LoadQuat(pDesc->vQuat), m_pOwner);
	ASSERT_CRASH(m_pCharacter);
	m_pCharacter->AddToPhysicsSystem();

	m_BodyID = m_pCharacter->GetBodyID();
}

CRigidbody* CRigidbody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRigidbody* pInstance = new CRigidbody(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Rigidbody");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CRigidbody::Clone(void* pArg)
{
	CRigidbody* pClone = new CRigidbody(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Rigidbody (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CRigidbody::Free()
{
	__super::Free();

	m_pOwner = nullptr;

	if (nullptr != m_pCharacter)
		m_pCharacter->RemoveFromPhysicsSystem();
	Safe_Delete(m_pCharacter);
}
