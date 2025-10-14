#include "EnginePch.h"
#include "PhysicsManager.h"

#include "ContactListenerImpl.h"
#include "CharacterContactListenerImpl.h"

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

Character* CPhysicsManager::Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
	return new Character(&CharacterSetting, vPos, vQuat, reinterpret_cast<JPH::uint64>(pUserData), m_pPhysicsSystem);
}

CharacterVirtual* CPhysicsManager::Register_CharacterVirtual(const CharacterVirtualSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
	CharacterVirtual* pInstance = new CharacterVirtual(&CharacterSetting, vPos, vQuat, reinterpret_cast<JPH::uint64>(pUserData), m_pPhysicsSystem);
	ASSERT_CRASH(pInstance);

	// Character VS Character Collision SetUp
	pInstance->SetCharacterVsCharacterCollision(m_pCVCCollision);
	// Chararcter VS Character Collision俊 殿废
	m_pCVCCollision->Add(pInstance);
	// CharacterContactListener SetUp
	pInstance->SetListener(m_pCharacterContactListener);

	return pInstance;
}

void CPhysicsManager::Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer)
{
	ASSERT_CRASH(pVirtual);

	m_Virtuals[iObjectLayer].push_back(pVirtual);
}

HRESULT CPhysicsManager::Initialize(_uint iNumObjectLayer)
{
	ASSERT_CRASH(iNumObjectLayer > 0);

	m_iNumObjectLayer = iNumObjectLayer;

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

	// Layer 积己
	m_pBPLayer = new BPLayer(iNumObjectLayer);
	ASSERT_CRASH(m_pBPLayer);
	// Filter 积己
	m_pObjectLayerFilter = new ObjectLayerPairFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectLayerFilter);
	m_pObjectVsBPFilter = new ObjectVsBroadPhaseLayerFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectVsBPFilter);
	// Contact Listener 积己
	m_pContactListener = new CContactListenerImpl();
	ASSERT_CRASH(m_pContactListener);

	// Virtual Container 悼利 且寸
	m_Virtuals = new vector<CharacterVirtual*>[m_iNumObjectLayer];
	// CharacterVirtual VS CharacterVirtual Collision
	m_pCVCCollision = new CharacterVsCharacterCollisionSimple();

#ifdef _DEBUG
	m_pDebugRenderer = new CDebugRender(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pDebugRenderer);

	m_DrawSetting.mDrawShape = true;
	m_DrawSetting.mDrawShapeWireframe = false;
#endif

	m_ExtendedUpdateSetting.mStickToFloorStepDown = Vec3(0.f, -2.f, 0.f);

	return S_OK;
}

void CPhysicsManager::Update(_float fTimeDelta)
{
	m_pPhysicsSystem->Update(fTimeDelta, 1, m_pAllocator, m_pJobSystem);

	for (_uint i = 0; i < m_iNumObjectLayer; ++i)
	{
		for (auto& pVirtual : m_Virtuals[i])
		{
			// BroadPhaseLayerFilter
			DefaultBroadPhaseLayerFilter BPLayerFilter = DefaultBroadPhaseLayerFilter(*m_pObjectVsBPFilter, ObjectLayer(i));
			DefaultObjectLayerFilter ObjectLayerFilter = DefaultObjectLayerFilter(*m_pObjectLayerFilter, ObjectLayer(i));
			BodyFilter bodyFilter = BodyFilter();
			ShapeFilter shapeFilter = ShapeFilter();

			pVirtual->ExtendedUpdate(fTimeDelta, Vec3(0.f, -9.81f, 0.f),
				m_ExtendedUpdateSetting,
				BPLayerFilter,
				ObjectLayerFilter,
				bodyFilter,
				shapeFilter,
				*m_pAllocator
			);

			pVirtual = nullptr;
		}
		m_Virtuals[i].clear();
	}
}

#ifdef _DEBUG
void CPhysicsManager::Render()
{
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	m_pPhysicsSystem->DrawBodies(m_DrawSetting, m_pDebugRenderer);
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawShape(const Shape* pShape)
{
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	pShape->Draw(m_pDebugRenderer, RMat44::sIdentity(), Vec3(1.f, 1.f, 1.f), Color(0.f, 255.f, 0.f, 1.f), false, true);
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
#endif

void CPhysicsManager::SetUp_PhysicsSystem()
{
	// PhysicsSystem 积己
	m_pPhysicsSystem = new PhysicsSystem();
	m_pPhysicsSystem->Init(
		m_iNumBodies, m_iNumBodyMutexes, 
		m_iMaxBodyPairs, m_iMaxContactConstraints,
		*m_pBPLayer, *m_pObjectVsBPFilter, *m_pObjectLayerFilter);
	m_pPhysicsSystem->SetPhysicsSettings(m_PhysicsSetting);
	m_pPhysicsSystem->SetContactListener(m_pContactListener);

	// Character Contact Listener
	m_pCharacterContactListener = new CharacterContactListenerImpl(&m_pPhysicsSystem->GetBodyInterface());

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

	Safe_Delete_Array(m_Virtuals);

	Safe_Delete(m_pBPLayer);
	Safe_Delete(m_pObjectLayerFilter);
	Safe_Delete(m_pObjectVsBPFilter);
	Safe_Delete(m_pCVCCollision);
#ifdef _DEBUG
	Safe_Delete(m_pDebugRenderer);
#endif
	Safe_Delete(m_pPhysicsSystem);
	Safe_Delete(m_pCharacterContactListener);
	Safe_Delete(m_pContactListener);
	Safe_Delete(m_pJobSystem);
	Safe_Delete(m_pAllocator);
	Safe_Delete(Factory::sInstance);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
