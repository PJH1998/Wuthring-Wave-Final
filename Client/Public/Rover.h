#pragma once
#include "Player_Define.h"
#include "RoverState_Enum.h"

NS_BEGIN(Client)
class CRover final : public CCharacter
{
public:
	enum PENDING_CONDITION : _uint
	{
		HIT = 0,
		DODGE,
		PARRY,
		QTE,
		CONDITION_END
	};

	enum VOLUME
	{
		VOLUME_KNOCKBACK = 0,
		VOLUME_SKILL = 1,
		VOLUME_END
	};

#pragma region STATE
private:
	struct StateTransitionContext
	{
		// Ground
		ERoverDashType m_eDashType = ERoverDashType::END;
		ERoverIdleType  m_eIdleType = ERoverIdleType::END;
		ERoverRunType m_eRunType = ERoverRunType::END;
		ERoverSprintType m_eSprintType = ERoverSprintType::END;
		ERoverLandType m_eLandType = ERoverLandType::END;
		ERoverDodgeType m_eDodgeType = ERoverDodgeType::END;

		ERoverAttackType m_eAttackType = ERoverAttackType::END;
		ERoverSkillType m_eSkillType = ERoverSkillType::END;
		ERoverUniqueType m_eUniqueType = ERoverUniqueType::END;
		ERoverBurstType m_eBurstType = ERoverBurstType::END;
		ERoverSpecialType m_eSpecialType = ERoverSpecialType::END;
		ERoverQTEType m_eQTEType = ERoverQTEType::END;

		// Air
		ERoverJumpType m_eJumpType = ERoverJumpType::END;
		ERoverFallType m_eFallType = ERoverFallType::END;
		ERoverAirAttackType m_eAirAttackType = ERoverAirAttackType::END;
		ERoverAirFlyType m_eAirFlyType = ERoverAirFlyType::END;

		// Climb
		ERoverClimbIdleType m_eClimbIdleType = ERoverClimbIdleType::END;
		ERoverClimbMoveType m_eClimbMoveType = ERoverClimbMoveType::END;
		ERoverClimbExitType m_eClimbExitType = ERoverClimbExitType::END;
		_bool m_IsClimbSecondStep = { false };

		// Hit
		ERoverHitType m_eHitType = ERoverHitType::END;

		// Prev Info
		_string m_strPrevInfo = {};
		void Clear()
		{
			// Land
			m_eIdleType = ERoverIdleType::END;
			m_eRunType = ERoverRunType::END;
			m_eDashType = ERoverDashType::END;
			m_eLandType = ERoverLandType::END;
			m_eDodgeType = ERoverDodgeType::END;

			// Attack
			m_eAttackType = ERoverAttackType::END;
			m_eSkillType = ERoverSkillType::END;
			m_eUniqueType = ERoverUniqueType::END;
			m_eBurstType = ERoverBurstType::END;
			m_eSpecialType = ERoverSpecialType::END;

			// Air
			m_eJumpType = ERoverJumpType::END;
			m_eFallType = ERoverFallType::END;
			m_eAirAttackType = ERoverAirAttackType::END;
			m_eAirFlyType = ERoverAirFlyType::END;

			// Climb
			m_eClimbIdleType = ERoverClimbIdleType::END;
			m_eClimbMoveType = ERoverClimbMoveType::END;
			m_eClimbExitType = ERoverClimbExitType::END;
			m_IsClimbSecondStep = false;

			m_eHitType = ERoverHitType::END;
			m_strPrevInfo.clear(); // String 비우기.
		};
	};

	StateTransitionContext m_StateContext;


public:
	StateTransitionContext& GetStateContextForWrite()
	{
		return m_StateContext;
	};

	StateTransitionContext TakeStateContext()
	{
		StateTransitionContext tempCopy = m_StateContext;
		m_StateContext = {}; 
		return tempCopy; 
	}

#pragma endregion
public:
	enum PARTTYPE : _uint
	{
		PART_SWORD = 0,
		PART_DARKWING = 1,
		PART_DARKSCYTHE = 2,
		PART_WING = 3,
		TYPE_END
	};

#pragma region 0. 
protected:
	explicit CRover(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRover(const CRover& Prototype);
	virtual ~CRover() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual	void	Render_OutLine() override;
	virtual void	Render_Shadow() override;
#pragma endregion


#pragma region 1. STATE
public:
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType) override;
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);
	virtual void PartActivate(_uint iPartType, _bool IsActive) override;
	virtual void Part_VolumeChange(_uint iPartType, _uint iVolumeIdx) override;
	virtual void Part_VolumeActivate(_uint iPartType, _bool IsActive) override;
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) override;
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) override;
	virtual void Hit_Judge(void* pArg = nullptr) override;
	void Sync_Position();

	virtual void Bind_QTE(_bool IsQTE) override;

	virtual void Reset_QTECamera() override;
	virtual void Bind_QTECamera() override;
#pragma region 2. NOTIFY
	public:
		virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
		virtual void Effect_Active(const _wstring& wStrEffectTag) override;
		virtual void Object_Func(const _wstring& wStrObjectTag) override;
#pragma endregion

#pragma region 3. CALL BACK
	public:
		void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
#pragma endregion

#pragma region 4. EVENT
public:
	virtual void Process_DelayedActions(_float fTimeDelta);
	virtual void Bind_ChangeEffect() override; // ChaneEffect 실행.
#pragma endregion

#pragma endregion
private:
	class CRoverSword* m_pRoverSword = { nullptr };
	class CRoverDarkWing* m_pRoverDarkWing = { nullptr };
	class CRoverDarkScythe* m_pRoverDarkScythe = { nullptr };
	class CWing* m_pWing = { nullptr };
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };
	_uint m_iCurrentPartType = { PARTTYPE::TYPE_END }; // State���� Ȱ��ȭ?

	
	// Attack Volume
	_uint m_iVolumeIdx = {};
	vector<class CAttackVolume*> m_AttackVolumes;

	_bool m_PendingConditions[CONDITION_END] = {};

private:
	void Bind_Resources();

	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);
	void Ready_AttackVolumes();

public:
	static		CRover* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

