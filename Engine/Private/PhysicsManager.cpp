#include "EnginePch.h"
#include "PhysicsManager.h"

#include "ContactListenerImpl.h"

CPhysicsManager::CPhysicsManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

Body* CPhysicsManager::Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut)
{
	Body* body = m_pPhysicsSystem->GetBodyInterface().CreateBody(BodySetting);
	ASSERT_CRASH(body);

	m_pPhysicsSystem->GetBodyInterface().AddBody(body->GetID(), EActivation::Activate);

	*pOut = &m_pPhysicsSystem->GetBodyInterface();

	return body;
}

HRESULT CPhysicsManager::Initialize(_uint iNumObjectLayer)
{
	ASSERT_CRASH(iNumObjectLayer > 0);

	// Register Allocator
	RegisterDefaultAllocator();
	// Create factory
	Factory::sInstance = new Factory;
	// Register physics types with the factory
	RegisterTypes();

	m_pAllocator = new TempAllocatorImpl(32 * 1024 * 1024);
	ASSERT_CRASH(m_pAllocator);
	// Thread Pool
	m_pJobSystem = new JobSystemThreadPool(2048, 8, m_iMaxJob - 1);
	ASSERT_CRASH(m_pJobSystem);

	// Layer 持失
	m_pBPLayer = new BPLayer(iNumObjectLayer);
	ASSERT_CRASH(m_pBPLayer);
	// Filter 持失
	m_pObjectLayerFilter = new ObjectLayerPairFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectLayerFilter);
	m_pObjectVsBPFilter = new ObjectVsBroadPhaseLayerFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectVsBPFilter);
	// Contact Listener 持失
	m_pContactListener = new CContactListenerImpl();
	ASSERT_CRASH(m_pContactListener);

#ifdef _DEBUG
	m_pDebugRenderer = new CDebugRender(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pDebugRenderer);

	m_DrawSetting.mDrawShape = true;
	m_DrawSetting.mDrawShapeWireframe = true;
#endif

	return S_OK;
}

void CPhysicsManager::Update(_float fTimeDelta)
{
	m_pPhysicsSystem->Update(fTimeDelta, 1, m_pAllocator, m_pJobSystem);
}

#ifdef _DEBUG
void CPhysicsManager::Render()
{
	m_pPhysicsSystem->DrawBodies(m_DrawSetting, m_pDebugRenderer);
}
#endif

void CPhysicsManager::SetUp_PhysicsSystem()
{
	// PhysicsSystem 持失
	m_pPhysicsSystem = new PhysicsSystem();
	m_pPhysicsSystem->Init(
		m_iNumBodies, m_iNumBodyMutexes, 
		m_iMaxBodyPairs, m_iMaxContactConstraints,
		*m_pBPLayer, *m_pObjectVsBPFilter, *m_pObjectLayerFilter);
	m_pPhysicsSystem->SetPhysicsSettings(m_PhysicsSetting);
	m_pPhysicsSystem->SetContactListener(m_pContactListener);

	Vec3 vGravity = Vec3(0, -9.81f, 0);
	//Vec3 vGravity = Vec3(0, -5.81f, 0);
	m_pPhysicsSystem->SetGravity(vGravity);
}

CPhysicsManager* CPhysicsManager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumObjectLayer)
{
	CPhysicsManager* pInstance = new CPhysicsManager(pDevice, pContext);

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
#ifdef _DEBUG
	Safe_Delete(m_pDebugRenderer);
#endif
	Safe_Delete(m_pPhysicsSystem);
	Safe_Delete(m_pContactListener);
	Safe_Delete(m_pJobSystem);
	Safe_Delete(m_pAllocator);
	Safe_Delete(Factory::sInstance);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
