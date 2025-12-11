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
	typedef struct tagHitDesc {
		_uint iLayer;
		_float fAttack;
		CTransform* pTransform = { nullptr };
		_bool IsBack = { false };
	}HIT_DESC;

	typedef struct tagParryDesc {
		_uint iLayer;
		_float fAttack;
		CTransform* pTransform = { nullptr };
	}PARRY_DESC;

	typedef struct tagCaptureDesc {
		_uint iLayer;
		_float fAttack;
		CTransform* pTransform = { nullptr };
		const _float4x4* pSocketMatrix = { nullptr };
	}CAPTURE_DESC;

	typedef struct tagQTEDesc {

		CTransform* pTargetTransform = { nullptr };
		CHARACTER_EVENT eEvent = { CHARACTER_EVENT::END };
	}QTE_DESC;



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
		pair<LEVEL, _wstring> facialComputeShaderData = {};
		//pair<LEVEL, _wstring> controllerData = {};
		vector<pair<_wstring, _wstring>> PartPrototypes;
		_float3 vScale = { 1.f, 1.f, 1.f };
		_float3 vRotation = { 0.f, 0.f, 0.f };
		_float3 vPosition = { 0.f, 0.f, 0.f };
		_float fHookRange = { 25.f };
		_float fDragRange = { 15.f };
		_float fReacedRopeHook = { 1.f };
		_float fThrowRange = { 40.f };


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

	// 상태 판단.
	virtual void Hit_Judge(void* pArg = nullptr) {};
	virtual void Parry_Judge(void* pArg = nullptr) {};
	virtual void Grab_Judge(void* pArg = nullptr) {};
	virtual void Resolve_PerfectDodge() {};

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
	void Sync_Collider(_fvector vVelocity, _float fTimeDetla);
	_fvector Get_Velocity();
	void Add_Force(_fvector vForce, _float fTimeDelta);

	
	// WorldMatrix
	_matrix Get_WorldMatrix();
	void Set_Position(_fvector vPos);

	void ColliderActive(_bool IsActive);

#ifdef _DEBUG
	void Print_LookRay();
	void Debug_ImGui();
#endif // _DEBUG

#pragma endregion


#pragma region EVENT 
public:
	virtual void Bind_QTE(_bool IsQTE) {};
	_bool IsQTEend() { return m_IsQTEend; }
	void Set_QTEEnd(_bool IsQTEend) { m_IsQTEend = IsQTEend; }

	_bool IsVisible() { return m_IsVisible; }
	void Set_Visible(_bool IsVisible) { m_IsVisible = IsVisible; }

	_bool Is_OutLineVisible() { return m_IsOutLineVisible; }
	void Set_OutLineVisible(_bool IsVisible) { m_IsOutLineVisible = IsVisible; }

	virtual void Process_DelayedActions() {};
	virtual void Calc_ChangeTimer(_float fTimeDelta) {}; // Timer 계산
	virtual void Bind_ChangeEffect() {}; // ChaneEffect 실행.

	virtual void Render_Damage(const HIT_DESC* pDesc);
	virtual void Begin_Toggle_SFX(SFX_TOGGLE eType, _float fDuration = 0.f);
	virtual void End_SFX();

	virtual void Spawn_SFX(const _wstring& strSFXTag);

	virtual void Spawn_Effect(const _wstring& wStrEffectTag);
	virtual void OnEvent(CHARACTER_EVENT eEvent, void* pArg = nullptr) {};

	void Spwan_RopeEffect(const _wstring& wStrEffectTag, const _string& strBoneName);
	void Execute_Telport(_vector vPos);

	void Reserve_LandSlide(const SLIDE_DATA& eData);

	void Bind_GrabEscapePossible();
	void Bind_GrabEscapeExecute();
	void Bind_GrabVisible(_bool IsVisible);
	void ResetPose();

	void Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate, _float fDuration);

	void Change_TimeRatio_ToLayer(COLLISIONLAYER eCollisionLayer, _float fTimeRatio, _float fDuration);
	void Change_TimeRatio_ToLayer(COLLISIONLAYER eCollisionLayer, _float fTimeRatio);

	LEVEL Get_CurrentLevel() { return m_eCurLevel; }

	void Spawn_MotionTrail(_float fDuration, _float fInterval, _float fMotionLifeTime, _float4 vColor, _uint iShaderPath = 0);
	

	void Use_Spring(_float fDestination, _float fDuration);

	void Stop_Anim();
	void Start_Anim();

	void Play_Sound(const _wstring& strSoundTag, CHANNEL eChannel, _float fVolume, _float fFrequency = 1.f);
	void Stop_Sound(CHANNEL eChannel);

	_float Rand(_float fMin, _float fMax);

	
#pragma endregion




#pragma region STATE
public:
	// Caemra
	void Camera_Shake(_float fIntensity);
	void Play_Action(const _wstring& strActionTag, _bool isEscape = false); // Action Camera (Cut Scene)

	// Ability에서 확인 받기 => 상태 판별?
	_bool Check_AnyConidtion_FromAbility(_uint iCondition);

	// T 사용시 컨디션 공유. Interaction Type 설정.
	UI_TAB_UTILITY Get_UtilityType();

	// Grapple Target 전달.
	void Bind_GrappleTarget(const GRAPPLE_INFO& grapInfo);

	// Grapple Target을 이용한 사용 함수들
	void Rotate_GrappleTarget();
	void Move_Grapple(_float fTimeDelta, _float fSpeed);
	void Execute_RopeDragTrigger();
	_float Get_GrappleDistance();
	_bool Is_GrappleHook();
	_bool Is_GrappleDrag();
	_bool Is_ReachedGrappleHook();
	ROPEDIR Calculate_RopeDirection();

	// Thorw Target 전달.
	void Bind_ThrowTarget(const THROW_INFO& throwInfo);
	_bool Is_AttachThrowTarget();

	virtual void Attach_ThrowTarget(_bool isAttach) {}; // ThrowTarget 객체를 손뼈에 붙입니다.
	virtual void Throw_AttachTarget() {};
	

	// Ability에 제공. => 상태 판별할때 사용.
	void Bind_Condition_ToAbillity(_uint iCondition);
	void Remove_Condition_ToAbillity(_uint iCondition);
	void Bind_CostCondition_ToAbility(_uint iConditionw, _uint iConditionFlag);

	// Transition Character From Player
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType, void* pArg = nullptr) {}; // 전환 시 실행할 함수.
	
	/* Parts */
	virtual void PartActivate(_uint iPartType, _bool IsActive) {};
	virtual void Part_VolumeChange(_uint iPartType, _uint iVolumeIdx) {};
	virtual void Part_VolumeActivate(_uint iPartType, _bool IsActive) {};
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false) {};
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) {};
	virtual void Set_AnimationToParts(_uint iPartType, const _string& strAnimName) {};
	virtual void Part_ShaderPathChange(_uint iPartType, _uint iShaderPath) {};



	// Look Vector
	_vector Get_Position();
	_vector Get_LookVector();
	_vector Get_CameraLookVector();
	_vector Get_LookVector_NoPitch();
	_vector Get_RightVector();
	_vector Get_RightVector_NoPitch();

	// LockOn
	void Set_AutoLockOn(class CTransform* pTargetTransform, _bool IsLockOn);
	void Set_LockOn(class CTransform* pTargetTransform, _bool IsLockOn);
	_bool Is_LockOn();
	
	// TargetPosition
	void Bind_TargetPosition(_fvector vPos);

	// Hit
	_bool Is_Hit() { return m_IsHit; }
	void Set_Hit(_bool IsHit) { m_IsHit = IsHit; }
	const HIT_DESC* GetPendingHitDesc() const { return &m_PendingHitDesc; } // 읽기 전용 정보 전달.
	void ClearPendingHit() { m_PendingHitDesc = {}; }

	// Capture
	const CAPTURE_DESC* GetPendingCaputreDesc() const { return &m_PendingCaptureDesc; } // 읽기 전용 정보 전달.
	void ActiveCaptureState();
	void ClearCaptureState();

	// KeyInput
	_bool Check_AnyInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);
	_bool Check_AllInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);

	// Animation
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) {};
	virtual void Clear_Animation(const _string& strAnimName, _float fTrackPosition = 0.f);
	virtual _bool Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition
		, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsFacial = true);

	virtual _bool Play_Animation_NonFacical(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition
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
	void Rotate_Target(_bool IsReverse = false);
	
	void Rotate_Target(class CTransform* pTransform);
	void Rotate_TargetPosition();
	void Rotate_Target_Lerp(_float fTimeDelta);
	void Rotate_HitTarget(class CTransform* pTransform);

	// Turn
	
	// Transform
	void Sync_Transform_FromPlayer(_fmatrix WorldMatrix, _fvector vPrevVeloctiy, _float fTimeDelta);
	void Sync_Transform_ToPlayer(class CTransform* pTransformCom); // Player로 보낸다.

	// Interaction Type
	void Sync_UtilityType_FromPlayer(UI_TAB_UTILITY eUtilityType);
#pragma endregion

#ifdef _DEBUG
public:
	void Debug_FullCost(_bool IsAll = false);
	void Clear_CoolTime();
#else
public:
	void Debug_FullCost(_bool IsAll = false);
	void Clear_CoolTime();
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

	
#pragma region CHARACTER CONDITION
public:
	void Add_Condition(_uint iConditionFlag);
	_bool Check_AnyCondition(_uint iConditionFlag);
	_bool Check_AllCondition(_uint iConditionFlag);
	void Remove_Condition(_uint iConditionFlag);
	void Remove_AllCondition();
	void Sync_Condition_ToPlayer(_uint* pCondition);
	void Add_Condition_FromPlayer(_uint iCondition);
	void Remove_Condition_FromPlayer(_uint iCondition);


	void Bind_ChangeTimer() { m_fChangeTimer = m_fChangeDuration; } // Dissolve에 바인딩할 변수값.

	virtual void Reset_QTECamera() {};
	virtual void Bind_QTECamera() {};

	virtual void Bind_DissolveTimer() {};
	virtual void Bind_DefaultShaderPath() {};
	virtual void Bind_DissolveShaderPath() {};

	virtual void Activate(_bool IsActivate) {};

	void Rope_Active(_bool IsActive) { m_IsRopeActive = IsActive;  }
#pragma endregion


protected:
	class CGameSystem* m_pGameSystem = { nullptr };
	class CInputController* m_pInputControllerCom = { nullptr };
	class CStateMachine* m_pStateMachineCom = { nullptr };
	class CStateMachine* m_pFpsStateMachineCom = { nullptr };
	class CSpringCamera* m_pSpringCamera = { nullptr };
	class CTransform* m_pTargetTransform = { nullptr }; // Auto Target 용도
	class CTransform* m_pLockOnTargetTransform = { nullptr }; // Auto Target 용도

	GRAPPLE_INFO m_GrappleInfo = {};
	THROW_INFO m_ThrowInfo = {};

	class CTransform* m_pTargetGrappleTransform = { nullptr }; // Grapple 용도.
	OBJECTTYPE m_eTargetGrappleType = { OBJECTTYPE::END };
	_uint m_iGrappleCondition = {};

	class CComputeShader* m_pFlyComputeShaderCom = { nullptr }; // 활공 용도
	class CComputeShader* m_pFacialComputeShaderCom = { nullptr }; // Facial 용도.

	class CCollider* m_pQTEColliderCom = { nullptr };
	_float4 m_vQTEPos = {};
	HarmonyEndCallback m_OnEnsembleEnd = { nullptr };

	_float m_fTargetDistance = {}; // 타겟과의 거리
	
	_float m_fColliderRadius = {};
	_float m_fColliderHeight = {};
	_float3 m_vColliderOffSet = {};
	
	UI_TAB_UTILITY m_eUtilityType = { UI_TAB_UTILITY::NOTHING };

	// Shader 변수.
	_float m_fMaxDissolveTime = { 0.5f };
	_float m_fDissolveTimer = {};
	_float4 m_vDissolveColor = {};
	_float4 m_vEmissiveColor = {};
	_float  m_fEmissiveIntensity = {};
	_float4 m_vMotionTrailColor = {};

	// Event Shader 변수.
	_float m_fEventDissolveTime = {};
	_float m_fEventDissolveTimer = {};

	_float4x4 m_DissolveWorldMatrix = {};
	_float4x4 m_MatrixIdentity = {}; // SocketMatrix 전달 시 아무것도 없으면 Identity 행렬 전달.
protected:
	_bool m_IsHit = { false };
	_bool m_IsLockOn = { false };
	_bool m_IsLand = { false };
	_bool m_IsQTE = { false };
	_bool m_IsQTEend = { false };
	_bool m_IsVisible = { true };
	_bool m_IsOutLineVisible = { true };
	_bool m_IsRopeActive = { false };
	_bool m_IsEventDissolve = { false }; // Dissolve가 연출용인지? 아닌지.
	_bool m_IsDissolveReverse = { false }; // Dissolve가 반대로 적용되는가?

	_uint m_iCondition = {}; // Client_Enum.h에 정의된 CharacterCondition 관리.

	// Event Data
	queue<DELAYED_ACTION> m_DelayedActions;
	
	HIT_DESC m_PendingHitDesc = {};
	PARRY_DESC m_PendingParryDesc = {};
	CAPTURE_DESC m_PendingCaptureDesc = {};
	SLIDE_DATA  m_PendingSlideData = {};

	_float m_fDodgeableDuration = 0.2f;
	_float m_fDodgeableHitTimer = {};

	_float m_fChangeDuration = { 1.f }; // 변환시간.
	_float m_fChangeTimer = { };

	_float m_fCameraOffset = {};
	_float m_fCameraOriginOffset = {};

	_float m_fHookRange = {};
	_float m_fReachedHook = {};
	_float m_fDragRange = {};
	_float m_fThrowRange = {};

	_float m_fStateTimeRate = { 1.f }; //
	_float m_fOriginTimeRate = { 1.f };
	_float m_fStateDelayTimer = {}; // StateDelayTimer;
	_float4 m_vTargetPosition = {};


	_float m_fCameraOriginDistance = {};
	_float m_fCaemraDistance = {};

	vector<class CAttackVolume*> m_AttackVolumes;
	class CAttackVolume* m_pMainAttackVolume = { nullptr };
	_float4x4 m_GrabComibinedMatrix = {};

protected: // 헬퍼 함수 상속
	void Process_MotionTrail(const _wstring& wStrObjectTag);
	void Process_PlaySound(const _wstring& wStrObjectTag);
	void Process_SpawnSFX(const _wstring& wStrobjectTag);

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;

};
NS_END

