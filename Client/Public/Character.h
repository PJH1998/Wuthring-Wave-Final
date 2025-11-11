#pragma once

#include "Actor.h"
NS_BEGIN(Client)
class CCharacter abstract : public CActor
{
public:
	enum CHARACTER_EVENT_ID
	{
		HIT = 0,
		EVENT_END
	};


public:
	typedef struct tagHitDesc{
		_uint iLayer;
		_float fAttack;
		CTransform* pTransform = { nullptr };
	}HIT_DESC;

	typedef struct tagParryDesc{
		_uint iLayer;
		_float fAttack;
		CTransform* pTransform = { nullptr };
	}PARRY_DESC;



public:
	using HarmonyEndCallback = function<void()>;

	void Set_HarmonyEndCallback(HarmonyEndCallback callback)
	{
		m_OnEnsembleEnd = callback;
	}


	void Notify_HarmonyEnd()
	{
		if (m_OnEnsembleEnd)
			m_OnEnsembleEnd();
	}

	void Bind_NotifyEnd()
	{

	}

	void Clear_HarmonyEndCallback() { 
		m_OnEnsembleEnd = nullptr; 

	}

public:
	typedef struct tagCharacterDesc : public CActor::ACTOR_DESC
	{
		class CPlayer* pOwner = { nullptr };
		pair<LEVEL, _wstring> stateMachineData = {};
		pair<LEVEL, _wstring> flyComputeShaderData = {};
		//pair<LEVEL, _wstring> controllerData = {};
		vector<pair<_wstring, _wstring>> PartPrototypes;
		_float3 vScale = { 1.f, 1.f, 1.f};
		_float3 vRotation = { 0.f, 0.f, 0.f };
		_float3 vPosition = { 0.f, 0.f, 0.f };
		CHARACTER_STAT eStat = {};

	}CHARACTER_DESC;
	

#pragma region 
protected:
	explicit CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCharacter(const CCharacter& Prototype);
	virtual ~CCharacter() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;


#pragma endregion

#pragma region 
public:
	// Object 
	void Set_InputController(class CInputController* pInputControllerCom);
	void Set_SpringCamera(class CSpringCamera* pSpringCamera);
	void Set_Collider(class CCollider* pColliderCom, _float3 vColliderOffset, _float fColliderHeight, _float fColliderRadius);
	void Set_Ability(class CAbility* pAbilityCom);
#pragma endregion




#pragma region PHYSICS
public:
	// 거리 판단
	const _float Calculate_RootMotionScale();

	// Hit 판단.
	virtual void Hit_Judge(void* pArg = nullptr) {};
	virtual void Parry_Judge(void* pArg = nullptr) {};
	// Wall
	_bool Check_ClimbableWall(_float3* pWallNormal = nullptr);
	_bool Check_ClimbableWall_Above(_float fEndRayOffset, _float3* pWallNormal = nullptr);

	// Land Check
	_float Get_DistanceFromGround(_float fStartYOffset = 0.f);
	_bool Is_LandCollider(_float3* pNormal = nullptr);
	//_bool Is_Land(_float fRayOffsetY = 0.2f, _float fLandDistance = 0.3f);

	// Gravity
	void Set_Gravity(_bool IsGravity);

	// Collider
	void Set_ColliderReferenceBone(const _string& strBoneName, _float3 vOffset = { 0.f, 0.f, 0.f });
	void Sync_Collider(_fvector vVelocity, _float fTimeDetla);

	_fvector Get_Velocity();
	void Add_Force(_fvector vForce, _float fTimeDelta);

	// WorldMatrix
	_matrix Get_WorldMatrix();

#ifdef _DEBUG
	void Print_LookRay();
#endif // _DEBUG

#pragma endregion


#pragma region EVENT 
public:
	virtual void Bind_QTE(_bool IsQTE) {};
	_bool IsQTEend() { return m_IsQTEend;  }
	void Set_QTEEnd(_bool IsQTEend) { m_IsQTEend = IsQTEend; }

	virtual void Process_DelayedActions() {};
#pragma endregion

#pragma region STATE
public:
	// Caemra
	void Camera_Shake(_float fIntensity);
	void Play_Action(const _wstring& strActionTag); // Action Camera (Cut Scene)

	// Ability에서 확인 받기 => 상태 판별?
	_bool Check_AnyConidtion_FromAbility(_uint iCondition);

	// Ability에 제공. => 상태 판별할때 사용.
	void Bind_Condition_ToAbillity(_uint iCondition);
	void Remove_Condition_ToAbillity(_uint iCondition);
	void Bind_CostCondition_ToAbility(_uint iCondition, _uint iConditionFlag);

	// Transition Character From Player
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType) {}; // 전환 시 실행할 함수.

	/* Parts */
	virtual void PartActivate(_uint iPartType, _bool IsActive) {};
	virtual void Part_VolumeChange(_uint iPartType, _uint iVolumeIdx) {};
	virtual void Part_VolumeActivate(_uint iPartType, _bool IsActive) {};
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false) {};
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) {}; 

	// Look Vector
	_vector Get_LookVector();
	_vector Get_CameraLookVector();
	_vector Get_LookVector_NoPitch();
	_vector Get_RightVector();
	_vector Get_RightVector_NoPitch();

	// LockOn
	void Set_AutoLockOn(class CTransform* pTargetTransform, _bool IsLockOn);
	void Set_LockOn(class CTransform* pTargetTransform, _bool IsLockOn);
	_bool Is_LockOn();
	
	// Hit
	_bool Is_Hit() { return m_IsHit; }
	void Set_Hit(_bool IsHit) { m_IsHit = IsHit; }
	const HIT_DESC* GetPendingHitDesc() const { return &m_PendingHitDesc; } // 읽기 전용 정보 전달.
	void ClearPendingHit() { m_PendingHitDesc = {}; }



	// KeyInput
	_bool Check_AnyInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);
	_bool Check_AllInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);

	// Animation
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) {};
	virtual _bool Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition
		, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);

	virtual _bool Play_AnimationFly(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition
		, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true,
		const GPU_BLEND_INFO& gpuBlendInfo = G_DefaultBlendInfo);

	// Change State
	void Change_State(_uint iCategory, _uint iSubState, void* pArg = nullptr);
	

	// Move
	ACTORDIR Calculate_Direction();
	_vector Get_CameraRightVector();

	_vector Calculate_Move_Direction(ACTORDIR eDir);
	_vector Calculate_LockOn_Move_Direction(ACTORDIR eDir);


	void Move_LockOn_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed);
	void Move_By_Camera_Direction_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed);
	void Move_Fall(_float fTimeDelta, _float fSpeed);
	void Move_Direction(_fvector vDir, _float fTimeDelta, _float fSpeed);

	// Rotate
	void Rotate_Direction(_fvector vDir);
	void Rotate_DirectionNoPitchLerp(_fvector vDir, _float fTimeDelta, _float fSpeed);
	void Rotate_DirectionLerp(_fvector vDir, _float fTimeDelta, _float fSpeed);
	void Rotate_Target();
	void Rotate_Target_Lerp(_float fTimeDelta);
	void Rotate_HitTarget(class CTransform* pTransform);

	// Turn
	
	// Transform
	void Sync_Transform_FromPlayer(_fmatrix WorldMatrix, _fvector vPrevVeloctiy, _float fTimeDelta);
	void Sync_Transform_ToPlayer(class CTransform* pTransformCom); // Player로 보낸다.
#pragma endregion

#ifdef _DEBUG
public:
	void Debug_FullCost(_bool IsAll = false);
#else
public:
	void Debug_FullCost(_bool IsAll = false);
#endif // _DEBUG


#pragma region UI Interface 
public:
	// Ability 업데이트는 플레이어의 Priority Update에서
	void Ability_Update(_float fTimeDelta);
	class CAbility* Get_AbilityCom();
	_float Get_Cost(COST_TYPE eCostType);
	_float Get_MaxCost();
	void Sync_UI(); // UI
#pragma endregion

#pragma region NOTIFY
public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) {};
	virtual void Effect_Active(const _wstring& wStrEffectTag) {};
	virtual void Object_Func(const _wstring& wStrObjectTag) {}; // 임시
#pragma endregion

	
#pragma region CONDITION
public:
	void Add_Condition(_uint iConditionFlag);
	_bool Check_AnyCondition(_uint iConditionFlag);
	_bool Check_AllCondition(_uint iConditionFlag);

	void Remove_Condition(_uint iConditionFlag);
	void Remove_AllCondition();


#pragma endregion


protected:
	class CGameSystem* m_pGameSystem = { nullptr };
	class CInputController* m_pInputControllerCom = { nullptr };
	class CStateMachine* m_pStateMachineCom = { nullptr };
	class CSpringCamera* m_pSpringCamera = { nullptr };
	class CTransform* m_pTargetTransform = { nullptr }; // Auto Target 용도
	class CTransform* m_pLockOnTargetTransform = { nullptr }; // Auto Target 용도
	class CTransform* m_pHitTargetTransform = { nullptr }; // Hit Target 용도 (맞은 방향을 알기 위한)
	class CComputeShader* m_pFlyComputeShaderCom = { nullptr }; // 활공 용도

	class CCollider* m_pQTEColliderCom = { nullptr };
	
	_float m_fTargetDistance = {}; // 타겟과의 거리

	_float4x4 m_MatrixIdentity = {};
	_float m_fColliderRadius = {};
	_float m_fColliderHeight = {};
	_float3 m_vColliderOffSet = {};
	

	_string m_strColliderReferenceBone = {}; // strColliderRefBone
	_float3 m_vAnimColliderOffset = {};
	

	_float4 m_vQTEPos = {};
	//CHARACTER_STAT m_Stats = {};
	HarmonyEndCallback m_OnEnsembleEnd = { nullptr };
	


protected:
	//queue<EVENT_DESC> m_EventQueue; // 특정한 이벤트가 발생해서 StateMachine 외부에서 상태가 변경되야 하는 경우 ex) Hit 등등

	_bool m_IsHit = { false };
	_bool m_IsLockOn = { false };
	_bool m_IsLand = { false };
	_bool m_IsQTE = { false };
	_bool m_IsQTEend = { false };

	_uint m_iCondition = {}; // Client_Enum.h에 정의된 CharacterCondition 관리.
	queue<DELAYED_ACTION> m_DelayedActions;
	

	HIT_DESC m_PendingHitDesc = {};
	PARRY_DESC m_PendingParryDesc = {};

	const _float m_fDodgeableDuration = 0.2f;
	_float m_fDodgeableHitTimer = {};

	vector<class CAttackVolume*> m_AttackVolumes;
	class CAttackVolume* m_pMainAttackVolume = { nullptr };
	
	

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;

};
NS_END

