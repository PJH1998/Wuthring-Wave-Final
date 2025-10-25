#include "EnginePch.h"
#include "PhysicsManager.h"

#include "ContactListenerImpl.h"
#include "CharacterContactListenerImpl.h"

#include "GameInstance.h"

CPhysicsManager::CPhysicsManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }, m_pContext{ pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
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

Ref<CharacterVirtual> CPhysicsManager::Register_CharacterVirtual(const CharacterVirtualSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
	Ref<CharacterVirtual> pInstance = new CharacterVirtual(&CharacterSetting, vPos, vQuat, reinterpret_cast<JPH::uint64>(pUserData), m_pPhysicsSystem);
	ASSERT_CRASH(pInstance);

	// Character VS Character Collision SetUp
	pInstance->SetCharacterVsCharacterCollision(m_pCVCCollision);
	// Chararcter VS Character Collision???깅줉
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

void CPhysicsManager::Remove_Virtual(CharacterVirtual* pVirtual)
{
	m_pCVCCollision->Remove(pVirtual);
}

void CPhysicsManager::Clear_Resource()
{
	//m_pPhysicsSystem->GetBodyInterface().
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

	// Layer ?앹꽦
	m_pBPLayer = new BPLayer(iNumObjectLayer);
	ASSERT_CRASH(m_pBPLayer);
	// Filter ?앹꽦
	m_pObjectLayerFilter = new ObjectLayerPairFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectLayerFilter);
	m_pObjectVsBPFilter = new ObjectVsBroadPhaseLayerFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectVsBPFilter);

	// Virtual Container ?숈쟻 ?좊떦
	m_Virtuals = new vector<CharacterVirtual*>[m_iNumObjectLayer];
	// CharacterVirtual VS CharacterVirtual Collision
	m_pCVCCollision = new CharacterVsCharacterCollisionSimple();

	// RayFilter
	m_pRayFilter = new SpecifiedBroadPhaseLayerFilter(static_cast<BroadPhaseLayer>(ENUM_CLASS(BPLAYER::NON_MOVE)));

#ifdef _DEBUG
	m_pDebugRenderer = new CDebugRender(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pDebugRenderer);

	m_DrawSetting.mDrawShape = true;
	m_DrawSetting.mDrawShapeWireframe = false;
#endif

	//m_ExtendedUpdateSetting.mStickToFloorStepDown = Vec3(0.f, -2.f, 0.f);
	m_ExtendedUpdateSetting.mStickToFloorStepDown = Vec3(0.f, -0.2f, 0.f);

	return S_OK;
}

void CPhysicsManager::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_DIKeyState(DIK_DELETE) == KEYSTATE::DOWN)
		m_isRenderAll = !m_isRenderAll;
#endif
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

void CPhysicsManager::Late_Update()
{
	if (nullptr == m_pContactListener)
		return;
	m_pContactListener->Remove_Update();
}

_bool CPhysicsManager::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, _float4* pOut)
{
	RVec3 StartPos = LoadVec3(vStartPos);
	RVec3 EndPos = LoadVec3(vEndPos);

	_vector vDir = vEndPos - vStartPos;

	RRayCast ray(StartPos, (EndPos - StartPos));
	RayCastResult result;

	_float fOriginFraction = result.mFraction;
	m_pPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, result, *m_pRayFilter);

	if (nullptr != pOut)
	{
		_float fDistanceOffset = 0.8f;
		XMStoreFloat4(pOut, vStartPos + result.mFraction * vDir * fDistanceOffset);
	}

	return fOriginFraction > result.mFraction && result.mFraction > 0.f ? true : false;
}

#ifdef _DEBUG
void CPhysicsManager::Render()
{
	if (false == m_isRenderAll)
		return;
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	m_pPhysicsSystem->DrawBodies(m_DrawSetting, m_pDebugRenderer);
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawShape(const Shape* pShape, RMat44 Matrix)
{
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	pShape->Draw(m_pDebugRenderer, Matrix, Vec3(1.f, 1.f, 1.f), Color(0.f, 255.f, 0.f, 1.f), false, true);
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawRay(const _fvector& vStartPos, const _fvector& vEndPos)
{
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	m_pDebugRenderer->DrawLine(LoadVec3(vStartPos), LoadVec3(vEndPos), Color(255.f, 0.f, 0.f, 1.f));
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
#endif

void CPhysicsManager::SetUp_PhysicsSystem()
{
	// PhysicsSystem ?앹꽦
	m_pPhysicsSystem = new PhysicsSystem();
	m_pPhysicsSystem->Init(
		m_iNumBodies, m_iNumBodyMutexes, 
		m_iMaxBodyPairs, m_iMaxContactConstraints,
		*m_pBPLayer, *m_pObjectVsBPFilter, *m_pObjectLayerFilter);
	m_pPhysicsSystem->SetPhysicsSettings(m_PhysicsSetting);

	// Contact Listener Create / SetUp
	m_pContactListener = new CContactListenerImpl(&m_pPhysicsSystem->GetBodyInterface());
	ASSERT_CRASH(m_pContactListener);
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
	Safe_Delete(m_pRayFilter);
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
	Safe_Release(m_pGameInstance);
}
