#pragma once
#include "SequencePlayerDefine.h"
#include "GameObject.h"
NS_BEGIN(Client)
class CSequencePlayer final : public CGameObject
{
public:
	enum SEQUENCECHARACTER
	{
		NONE = -1,
		YUNO = 0,
		AUGUSTA = 1,
		LUPA = 2,
		ZANI = 3,
		SHACON = 4,
		CARLOTA = 5,
		SEQUENCE_END
	};

public:
	typedef struct tagSequencePlayerDesc
	{
		LEVEL eCurLevel = { LEVEL::END };
		_float3 vPosition = {};
		_float3 vScale = {};
		_float3 vRotation = {};
		_uint iPlayerCount = {};
		vector<SEQUENCEPLAYER_SPEC> PlayerSpecs = {};
	}SEQUENCEPLAYER_DESC;

private:
	explicit CSequencePlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSequencePlayer(const CSequencePlayer& Prototype);
	virtual ~CSequencePlayer() = default;


public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_Shadow() override;

#pragma endregion

public:
	void OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);


public:
	_bool Is_TargetValid(class CTransform* pTarget);

#pragma region GAMESYSTEM 연계
public:
	void Summon_Squad_Near_Boss(class CTransform* pTarget);
	void Binding_Trigger();
#pragma endregion






private:
	vector<class CCharacter*> m_Characters;
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	LEVEL m_eCurLevel = { LEVEL::END };
	
	// 현재 실행 중인 SequenceCharacter
	vector<class CCharacter*> m_SequenceCharacters;


private:
	class CGameSystem* m_pGameSystem = { nullptr };
	vector<class CTransform*> m_TargetTransforms; // 타겟 탐색은 해야됌.
	class CTransform* m_pTargetTransform = { nullptr };

	_float3 m_vColliderOffSet = {};
	_float m_fColliderHeight = {};
	_float m_fColliderRadius = {};

	// Mutex
	mutex m_Mutex;

	_float m_fTargetDistance = {}; // 몬스터와의 거리
	_uint m_iCondition = {};

	_uint m_iNextSpawnIndex = 0;
	_uint m_iSequenceMax = { 3 };
private:
	void Sorting_Target();
	void Toggle_LockOn();
	void Process_CollideEnemy(const CALLBACK_CLIENT* pcallDesc);

private:
	HRESULT Ready_Players(const SEQUENCEPLAYER_DESC* pDesc);
	HRESULT Ready_Components(const SEQUENCEPLAYER_DESC* pDesc);
	HRESULT Ready_Sequence(const SEQUENCEPLAYER_DESC* pDesc);
	


public:
	static		CSequencePlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

