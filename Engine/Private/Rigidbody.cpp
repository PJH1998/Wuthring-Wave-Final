#include "EnginePch.h"
#include "Rigidbody.h"

#include "GameInstance.h"

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
	RefConst<Shape> BoxShape = new JPH::BoxShape(Vec3(5.f, 5.f, 5.f));

	BodyCreationSettings bodySetting(
		BoxShape,					// Shape
		Vec3(0.f, 0.f, 0.f),				// Position
		Quat::sIdentity(),			// Quat
		EMotionType::Dynamic,	// Motion Type
		0								// Collision Layer
	);

	m_BodyID = m_pGameInstance->Register_Body(bodySetting);

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
}
