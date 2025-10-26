#pragma once

#include "Actor.h"
NS_BEGIN(Client)
// 플레이어 캐릭터의 부모 객체.
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
	typedef struct tagPlayerStat
	{
		_float fHp = {};
		_float fEnsembleEnergy = {};
		_float fAttack = {};
	}CHARACTER_STAT;


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
	

#pragma region 기본 함수
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

#pragma region STATE 조건에 사용
public:
	virtual void PartAcitvate(_uint iPartType, _bool IsActive) {};
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true) {};

	void Process_Input(class CInputController* pInputControllerCom);
	
	_bool Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 0.1f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);
	_bool Check_AnyInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);
	_bool Check_AllInput(_uint iKeyFlag, KEYSTATE eKeyState = KEYSTATE::PRESS);
	_bool Is_LockOn();
	_bool Is_Land(_float3* pNormal = nullptr);
	void Change_State(_uint iCategory, _uint iSubState);


	ACTORDIR Calculate_Direction();
	void Move_By_Camera_Direction_8Way(ACTORDIR eDir, _float fTimeDelta, _float fSpeed);
	void Move_Fall(_float fTimeDelta, _float fSpeed);
	void Move_Direction(_fvector vDir, _float fTimeDelta, _float fSpeed);
	_float Get_DistanceToGround(_float fStartYOffset = 0.f);
	_bool Check_ClimbableWall(_float3* pWallNormal = nullptr); // 벽전환이 가능한가?
	_bool Check_ClimbableWall_Above(_float fEndRayOffset, _float3* pWallNormal = nullptr);
	void Set_Gravity(_bool IsGravity);
	



	
	
#pragma endregion


public:
	class CPlayer* Get_Owenr() { return m_pOwner; }
	const CHARACTER_STAT& Get_CharacterStat() { return m_Stats; }
    void Add_EnsembleEnergy(_float fEnergy)  { m_Stats.fEnsembleEnergy = min(m_Stats.fEnsembleEnergy + fEnergy, m_fMaxEnsembleEnergy); }
    _bool Is_EnsembleFull() const { return  m_Stats.fEnsembleEnergy >= m_fMaxEnsembleEnergy; }
    void Reset_EnsembleEnergy() { m_Stats.fEnsembleEnergy = 0.f; }
	

protected:
	class CPlayer* m_pOwner = { nullptr };
	class CInputController* m_pInputControllerCom = { nullptr };
	class CStateMachine* m_pStateMachineCom = { nullptr };
	class CSpringCamera* m_pSpringCamera = { nullptr };


	_float m_fMaxEnsembleEnergy = { 100.f };
	_float m_fColliderRadius = {};
	_float m_fColliderHeight = {};
	_float3 m_vColliderOffSet = {};

	CHARACTER_STAT m_Stats = {};
	EnsembleEndCallback m_OnEnsembleEnd = { nullptr };
protected:
	_bool m_IsLockOn = { false };
	

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;

};
NS_END

