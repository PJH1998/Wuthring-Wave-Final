#pragma once
#include "Base.h"

#include "CollisionLayer.h"

NS_BEGIN(JPH)
class TempAllocator;
class JobSystem;
class PhysicsSystem;
NS_END

NS_BEGIN(Engine)

class CPhysicsManager final : public CBase
{
private:
	explicit CPhysicsManager();
	virtual ~CPhysicsManager() = default;

public:
	// Physics System ÃÊ±âÈ­
	void				SetUp_PhysicsSystem();
	// Object -> BroadPhase ¸ÊÇÎ
	void				SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer) {
		ASSERT_CRASH(nullptr != m_pBPLayer);
		m_pBPLayer->SetUp_ObjectToBP(iObjectLayer, iBPLayer);
	};
	// Object VS Object Layer Setting
	void				SetUp_ObjectFilter(_uint iSrc, _uint iDst) {
		ASSERT_CRASH(nullptr != m_pObjectLayerFilter);
		m_pObjectLayerFilter->SetUp_ObjectFilter(iSrc, iDst);
	};
	// Object VS BroadPhase Layer Setting
	void				SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer) {
		ASSERT_CRASH(nullptr != m_pObjectVsBPFilter);
		m_pObjectVsBPFilter->SetUp_ObjectVsBPFilter(iObjectLayer, iBPLayer);
	};

public:
	HRESULT			Initialize(_uint iNumObjectLayer);
	void				Update(_float fTimeDelta);


private:
	TempAllocator*		m_pAllocator = { nullptr };
	JobSystem*			m_pJobSystem = { nullptr };
	PhysicsSystem*		m_pPhysicsSystem = { nullptr };

	PhysicsSettings		m_PhysicsSetting;

	BPLayer*										m_pBPLayer = { nullptr };
	ObjectLayerPairFilterImpl*				m_pObjectLayerFilter = { nullptr };
	ObjectVsBroadPhaseLayerFilterImpl*	m_pObjectVsBPFilter = { nullptr };

	_uint		m_iNumBodies = { 10240 };
	_uint		m_iNumBodyMutexes = {}; // Autodetect
	_uint		m_iMaxBodyPairs = { 65536 };
	_uint		m_iMaxContactConstraints = { 20480 };

	_uint		m_iMaxJob = { thread::hardware_concurrency() };

public:
	static CPhysicsManager* Create(_uint iNumObjectLayer);
	virtual void Free() override;
};

NS_END