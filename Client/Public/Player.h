#pragma once
#include "Player_Define.h"
#include "GameObject.h"

NS_BEGIN(Client)
// Player Container
class CPlayer final : public CGameObject
{
public:
	enum CHARACTERTYPE
	{
		NONE = -1,
		ROVER = 0,
		AUGUSTA = 1,
		GALBRENA = 2,
		TYPE_END
	};

	enum SWITCH_STATE
	{
		SWITCH_NONE,     // 전환 대기 없음
		SWITCH_PENDING,  // 전환 준비 중
		SWITCH_READY     // 전환 준비 완료
	};

public:
	typedef struct tagPlayerPartyDesc
	{
		LEVEL eCurLevel = {LEVEL::END };
		_float3 vPosition = {};
		_float3 vScale = {};
		_float3 vRotation = {};

		_uint iPlayerCount = {};
		_wstring wStrInputControllerTag = {};
		vector<PLAYER_SPEC> PlayerSpecs = {};
	}PLAYER_DESC;

#pragma region 기본 함수들.
public:
	explicit CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPlayer(const CPlayer& Prototype);
	virtual ~CPlayer() = default;


public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_Shadow() override;

#pragma endregion

#pragma region UI Interface
	class CAbility* Get_AbilityCom(CHARACTERTYPE eCharacterType);
	CHARACTERTYPE Get_CurrentChar() const { return static_cast<CHARACTERTYPE>(m_iCurrentCharacterIdx); }

	_bool IsQTEPossible(CHARACTERTYPE eCharacterType);
	void ExecuteQTE(CHARACTERTYPE eCharacterType);

	_vector Get_LookVector();
	_vector Get_Position();
	const _float4x4* Get_PlayerMatrixPtr();
#pragma endregion
	

public:
	// State에서 호출: Ensemble Skill이 끝났음을 알림
	void Notify_HarmonyEnd();
	void On_HarmonyEnd(CHARACTERTYPE eCharacter);


public:
	void OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void OnCollider_GrappleDuring(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);


public:
	_bool Is_TargetValid(class CTransform* pTarget); // 타겟이 유효한가?

	
#pragma region GAMESYSTEM과의 연계함수.
public:
	void Notify_GrabVisible(_bool IsVisible);
	void Notify_EscapeGrabReady();
	void Notify_EscapeGrabExecute();
	void Notify_Event(CHARACTER_EVENT eEvent, void* pArg = nullptr);

	void Bind_EventLock(_bool IsLock);

	void Lock_Input(_bool IsLock);


#pragma endregion



private:
	vector<class CCharacter*> m_Characters; 
	class CInputController* m_pInputControllerCom = { nullptr };
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CRigidbody* m_pGrappleRigidbodyCom = { nullptr };
	class CSpringCamera* m_pSpringCamera = { nullptr };

	LEVEL m_eCurLevel = { LEVEL::END };
	_int m_iCurrentCharacterIdx = { CHARACTERTYPE::NONE };
	_int m_iPrevCharacterIdx = { CHARACTERTYPE::NONE };
	_int m_iHarmonyCharacterIdx = { CHARACTERTYPE::NONE };


private:
	class CGameSystem* m_pGameSystem = { nullptr };
	class CPlayerStatus* m_pPlayerStatus = { nullptr }; // 플레이어 Interface

	// LockOn
	//vector<class CTransform*> m_TargetTransforms;
	vector<TARGET_INFO> m_TargetCandidates;
	vector<GRAPPLE_INFO> m_GrappleCandidates;
	vector<THROW_INFO> m_ThrowCandidates;


	TARGET_INFO m_TargetInfo = {};
	TARGET_INFO m_LockOnTargetInfo = {};
	GRAPPLE_INFO m_TargetGrappleInfo = { };
	THROW_INFO m_TargetThrowInfo = {};

	class CCollider* m_pColliderCom = { nullptr };

	_bool m_IsLockOn = { false };
	_bool m_IsChanage = { false };
	_bool m_IsQTE = { false };
	_bool m_IsEventLock = { false };

	CHARACTERTYPE m_eNextCharacter = {};
	CALLBACK_CLIENT m_CallBack = {};

	_float3 m_vColliderOffSet = {};
	_float m_fColliderHeight = {};
	_float m_fColliderRadius = {};

	// Mutex
	mutex m_Mutex;

	_float m_fTargetDistance = {}; // 몬스터와의 거리
	_uint m_iCondition = {};
	UI_TAB_UTILITY m_eUtilityType = { UI_TAB_UTILITY::NOTHING }; // Player에서 관리.

	// Timer 관리.
	_float m_ChangeTimers[CHARACTERTYPE::TYPE_END] = {};
	_float m_fChangeCoolTime = {};

	_float3 m_vLockOnPos = {};

	_bool m_IsThrowReserve = { false };


private:
	void Player_KeyInput();
	void Change_Character(CHARACTERTYPE eNextCharacter, _float fTimeDetla);
	void Sync_Transform_FromCharacter(class CCharacter* pCharacter);
	void Sync_Condition_FromCharacter(class CCharacter* pCharacter);
	void Sync_InteractionType_ToCharacter(class CCharacter* pCharacter);
	
	// Target 검색 (매프레임)
	void Sorting_Target();
	void Toggle_LockOn();
	// Grapple Target 검색 (매프레임)
	void Sorting_GrappleTarget();
	void Toggle_Grapple();
	// Throw Target 검색 (매프레임)

	void Sorting_ThrowTarget();
	void Toggle_Throw();

	// Collider 처리.
	void Process_CollideEnemy(const CALLBACK_CLIENT* pcallDesc);
	void Process_CollideGrapple(const CALLBACK_CLIENT* pcallDesc);
	void Process_CollideThrow(const CALLBACK_CLIENT* pcallDesc);

	// 몬스터 QTE Event 처리.
	void Process_QTEEvent(CHARACTER_EVENT eEvent, void* pArg);

	void Manage_Condition();
	void Sync_UtilityType();
	_bool IsHitBack(class CTransform* pTransform);
	void Calc_LockOnPos();


	void Stop_Anim();
	void Start_Anim();

	
	

#ifdef _DEBUG
	_float3		m_vDebugTeleportPos = {};
	void			GUI_Teleport();
#endif

private:
	HRESULT Ready_Players(const PLAYER_DESC* pDesc);
	HRESULT Ready_Components(const PLAYER_DESC* pDesc);
	

public:
	static		CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;


};
NS_END

