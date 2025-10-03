#include "EnginePch.h"
#include "PhysicsManager.h"

CPhysicsManager::CPhysicsManager()
{
}

BodyID CPhysicsManager::Register_Body(const BodyCreationSettings& BodySetting)
{
	Body* body = m_pPhysicsSystem->GetBodyInterface().CreateBody(BodySetting);
	ASSERT_CRASH(body);

	m_pPhysicsSystem->GetBodyInterface().AddBody(body->GetID(), EActivation::Activate);

	return body->GetID();
}

HRESULT CPhysicsManager::Initialize(_uint iNumObjectLayer)
{
	ASSERT_CRASH(iNumObjectLayer > 0);

	RegisterDefaultAllocator();
	m_pAllocator = new TempAllocatorImpl(32 * 1024 * 1024);
	
	// Thread Pool
	m_pJobSystem = new JobSystemThreadPool(2048, 8, m_iMaxJob - 1);

	// Layer 持失
	m_pBPLayer = new BPLayer(iNumObjectLayer);
	// Filter 持失
	m_pObjectLayerFilter = new ObjectLayerPairFilterImpl(iNumObjectLayer);
	m_pObjectVsBPFilter = new ObjectVsBroadPhaseLayerFilterImpl(iNumObjectLayer);

	return S_OK;
}

void CPhysicsManager::Update(_float fTimeDelta)
{
	m_pPhysicsSystem->Update(fTimeDelta, 1, m_pAllocator, m_pJobSystem);
}

void CPhysicsManager::SetUp_PhysicsSystem()
{
	// PhysicsSystem 持失
	m_pPhysicsSystem = new PhysicsSystem();
	m_pPhysicsSystem->Init(
		m_iNumBodies, m_iNumBodyMutexes, 
		m_iMaxBodyPairs, m_iMaxContactConstraints,
		*m_pBPLayer, *m_pObjectVsBPFilter, *m_pObjectLayerFilter);
	m_pPhysicsSystem->SetPhysicsSettings(m_PhysicsSetting);

	Vec3 vGravity = Vec3(0, -9.81f, 0);
	m_pPhysicsSystem->SetGravity(vGravity);
}

CPhysicsManager* CPhysicsManager::Create(_uint iNumObjectLayer)
{
	CPhysicsManager* pInstance = new CPhysicsManager();

	if (FAILED(pInstance->Initialize(iNumObjectLayer)))
	{
		MSG_BOX("Failed to Created : PhysicsManager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPhysicsManager::Free()
{
	__super::Free();

	Safe_Delete(m_pBPLayer);
	Safe_Delete(m_pObjectLayerFilter);
	Safe_Delete(m_pObjectVsBPFilter);

	Safe_Delete(m_pPhysicsSystem);
	Safe_Delete(m_pJobSystem);
	Safe_Delete(m_pAllocator);
}
