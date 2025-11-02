#pragma once

#include "Actor.h"
NS_BEGIN(Client)
class CCharacter abstract : public CActor
{

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
#pragma endregion


#pragma region PHYSICS
public:
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

#ifdef _DEBUG
	void RayDir(_vector vRayDir, _float3 vEndPos);
#endif // _DEBUG

#pragma endregion


#pragma region STATE
public:
	// Transition Character From Player
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType) {}; // 전환 시 실행할 함수.

	/* Parts */
	virtual void PartActivate(_uint iPartType, _bool IsActive) {};
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false) {};
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) {}; 

	// Look Vector
	_vector Get_LookVector();
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
	virtual _bool Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);
	
	// Change State
	void Change_State(_uint iCategory, _uint iSubState);
	

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
	void Rotate_HitTarget();

	// Turn
	

	
	// Transform
	void Sync_Transform_FromPlayer(_fmatrix WorldMatrix, _fvector vPrevVeloctiy, _float fTimeDelta);
	void Sync_Transform_ToPlayer(class CTransform* pTransformCom); // Player로 보낸다.
#pragma endregion


public:
	class CPlayer* Get_Owenr() { return m_pOwner; }

#pragma region UI Interface
public:
	const CHARACTER_STAT& Get_CharacterStat() { return m_Stats; }
	void Add_SwitchGauge(_float fSwitchGauge) { m_Stats.fSwitchGauge = min(m_Stats.fSwitchGauge + fSwitchGauge, m_Stats.fMaxSwitchGauge); }
	_bool Is_SwitchGaugeFull() const { return  m_Stats.fSwitchGauge >= m_Stats.fMaxSwitchGauge; }
	void Reset_SwitchGauge() { m_Stats.fSwitchGauge = 0.f; }

	void Add_BurstGauge(_float fBurstGauge) { m_Stats.fBurstGauge = min(m_Stats.fBurstGauge + fBurstGauge, m_Stats.fMaxBurstGauge); }
	_bool Is_BurstGaugeFull() const { return  m_Stats.fBurstGauge >= m_Stats.fMaxBurstGauge; }
	void Reset_BurstGauge() { m_Stats.fBurstGauge = 0.f; }

	void Add_UniqueGauge(_float fUniqueGauge) { m_Stats.fUniqueGauge = min(m_Stats.fUniqueGauge + fUniqueGauge, m_Stats.fMaxUniqueGauge); }
	_bool Is_UniqueGaugeFull() const { return  m_Stats.fUniqueGauge >= m_Stats.fMaxUniqueGauge; }
	void Reset_UniqueGauge() { m_Stats.fUniqueGauge = 0.f; }

	void Sync_UI(); // UI
#pragma endregion


	

protected:
	class CGameSystem* m_pGameSystem = { nullptr };
	class CPlayer* m_pOwner = { nullptr };
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

	CHARACTER_STAT m_Stats = {};
	EnsembleEndCallback m_OnEnsembleEnd = { nullptr };
protected:
	_bool m_IsLockOn = { false };
	

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;

};
NS_END

