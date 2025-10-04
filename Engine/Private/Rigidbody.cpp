#include "EnginePch.h"
#include "Rigidbody.h"

#include "GameInstance.h"
#include "GameObject.h"

#include "Jolt/Physics/Collision/Shape/BoxShape.h"

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

		break;
	}
	case SHAPE::CONVEXHULL:
	{

		break;
	}
	case SHAPE::MESH:
	{

		break;
	}
	default:
		CRASH("Shape Error");
	}

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
	//bodySetting.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;

	// GameObject(Owner) -> UserData로 전달
	bodySetting.mUserData = reinterpret_cast<uint64>(m_pOwner);

	m_pBody = m_pGameInstance->Register_Body(bodySetting, &m_pBodyInterface);

	return S_OK;
}

void CRigidbody::Update()
{
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
}
