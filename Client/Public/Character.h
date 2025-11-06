#pragma once

#include "Actor.h"
NS_BEGIN(Client)
class CCharacter abstract : public CActor
{
public:
	typedef struct tagChangeStateDesc
	{
		StateKey eStateKey;
		void* pEvent;
	}CHANGE_STATE_DESC;

public:
	typedef struct tagHitDesc
	{
		_uint iLayer;
		_float fAttack;
		CTransform* pTransform = { nullptr };
	}HIT_DESC;

public:
	enum class CHARACTER_STATE
	{

	};

public:
	using EnsembleEndCallback = function<void()>;

	void Set_EnsembleEndCallback(EnsembleEndCallback callback)
	{
		m_OnEnsembleEnd = callback;
	}

	void Notify_EnsembleEnd()
	{
		if (m_OnEnsembleEnd)
			m_OnEnsembleEnd();
	}
	void Clear_EnsembleEndCallback() { m_OnEnsembleEnd = nullptr; }

public:
	typedef struct tagCharacterDesc : public CActor::ACTOR_DESC
	{
		class CPlayer* pOwner = { nullptr };
		pair<LEVEL, _wstring> stateMachineData = {};
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
	// Hit 판단.
	virtual void Hit_Judge(void* pArg = nullptr) {};
	// Wall
	_bool Check_ClimbableWall(_float3* pWallNormal = nullptr);
	_bool Check_ClimbableWall_Above(_float fEndRayOffset, _float3* pWallNormal = nullptr);

	// Land Check
	_float Get_DistanceFromGround(_float fStartYOffset = 0.f);
	_bool Is_LandCollider(_float3* pNormal = nullptr);
	_bool Is_Land(_float fRayOffsetY = 0.2f, _float fLandDistance = 0.3f);

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
	void RayDir(_vector vRayDir, _float3 vEndPos);
#endif // _DEBUG

#pragma endregion


#pragma region STATE
public:
	// Camera Action
	void Play_Action(const _wstring& strActionTag);

	// Ability에 제공. => 상태 판별할때 사용.
	void Bind_Condition_ToAbillity(_uint iCondition);
	void Remove_Condition_ToAbillity(_uint iCondition);

	// Transition Character From Player
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType) {}; // 전환 시 실행할 함수.

	/* Parts */
	virtual void PartActivate(_uint iPartType, _bool IsActive) {};
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false) {};
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) {}; 

	// Look Vector
	_vector Get_LookVector();
	_vector Get_CameraLookVector();
	_vector Get_LookVector_NoPitch();
	_vector Get_RightVector();
	_vector Get_RightVector_NoPitch();

	// LockOn
	void Set_LockOn(class CTransform* pTargetTransform, _bool IsLockOn);
	_bool Is_LockOn();
	
	// KeyInput
	_bool Check_AnyInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);
	_bool Check_AllInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);

	// Animation
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) {};
	virtual _bool Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition
		, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true,
		const GPU_BLEND_INFO& blendInfo = G_DefaultBlendInfo);
	void Start_FlyBlending(_float fDuration);

	// Change State
	void Change_State(_uint iCategory, _uint iSubState, void* pArg = nullptr);
	

	// Move
	ACTORDIR Calculate_Direction();
	_vector Get_CameraRightVector();

	_vector Calculate_Move_Direction(ACTORDIR eDir);
	void Move_LockOn_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed);
	void Move_By_Camera_Direction_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed);
	void Move_Fall(_float fTimeDelta, _float fSpeed);
	void Move_Direction(_fvector vDir, _float fTimeDelta, _float fSpeed);

	// Rotate
	void Rotate_Direction(_fvector vDir);
	void Rotate_DirectionNoPitchLerp(_fvector vDir, _float fTimeDelta, _float fSpeed);
	void Rotate_DirectionLerp(_fvector vDir, _float fTimeDelta, _float fSpeed);
	void Rotate_Target();
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
	void Sync_UI(); // UI
#pragma endregion


	

protected:
	class CGameSystem* m_pGameSystem = { nullptr };
	class CInputController* m_pInputControllerCom = { nullptr };
	class CStateMachine* m_pStateMachineCom = { nullptr };
	class CSpringCamera* m_pSpringCamera = { nullptr };
	class CTransform* m_pTargetTransform = { nullptr }; // Auto Target 용도
	class CTransform* m_pHitTargetTransform = { nullptr }; // Hit Target 용도 (맞은 방향을 알기 위한)

	_float4x4 m_MatrixIdentity = {};
	_float m_fColliderRadius = {};
	_float m_fColliderHeight = {};
	_float3 m_vColliderOffSet = {};

	_string m_strColliderReferenceBone = {}; // strColliderRefBone
	_float3 m_vAnimColliderOffset = {};
	
	_bool m_IsHit = { false };

	//CHARACTER_STAT m_Stats = {};
	EnsembleEndCallback m_OnEnsembleEnd = { nullptr };


protected:
	queue<CHANGE_STATE_DESC> m_StateChangeQueue; // 특정한 이벤트가 발생해서 StateMachine 외부에서 상태가 변경되야 하는 경우 ex) Hit 등등

	_bool m_IsLockOn = { false };
	_bool m_IsLand = { false };

	
	

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;

};
NS_END

