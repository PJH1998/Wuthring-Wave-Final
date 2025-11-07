#pragma once
#include "Player_Define.h"
#include "AugustaState_Enum.h"


NS_BEGIN(Client)
class CAugusta final : public CCharacter
{
public:
	enum VOLUME
	{
		VOLUME_RISE = 0,
		VOLUME_END
	};
#pragma region STATE
private:
	struct StateTransitionContext
	{
		// Ground
		EAugustaDashType m_eDashType = EAugustaDashType::END;
		EAugustaIdleType  m_eIdleType = EAugustaIdleType::END;
		EAugustaRunType m_eRunType = EAugustaRunType::END;
		EAugustaLandType m_eLandType = EAugustaLandType::END;

		EAugustaAttackType m_eAttackType = EAugustaAttackType::END;
		EAugustaSkillType m_eSkillType = EAugustaSkillType::END;
		EAugustaUniqueType m_eUniqueType = EAugustaUniqueType::END;
		EAugustaBurstType m_eBurstType = EAugustaBurstType::END;
		EAugustaSpecialType m_eSpecialType = EAugustaSpecialType::END;

		// Air
		EAugustaJumpType m_eJumpType = EAugustaJumpType::END;
		EAugustaFallType m_eFallType = EAugustaFallType::END;
		EAugustaAirAttackType m_eAirAttackType = EAugustaAirAttackType::END;
		EAugustaAirSkillType m_eAirSkillType = EAugustaAirSkillType::END;
		EAugustaAirFlyType m_eAirFlyType = EAugustaAirFlyType::END;

		// Climb
		EAugustaClimbIdleType m_eClimbIdleType = EAugustaClimbIdleType::END;
		EAugustaClimbMoveType m_eClimbMoveType = EAugustaClimbMoveType::END;
		EAugustaClimbExitType m_eClimbExitType = EAugustaClimbExitType::END;
		_bool m_IsClimbSecondStep = { false };

		// Hit
		EAugustaHitType m_eHitType = EAugustaHitType::END;
		

		// Prev Info
		_string m_strPrevInfo = {};
		void Clear()
		{
			// Land
			m_eIdleType = EAugustaIdleType::END;
			m_eRunType = EAugustaRunType::END;
			m_eDashType = EAugustaDashType::END;
			m_eLandType = EAugustaLandType::END;

			// Attack
			m_eAttackType = EAugustaAttackType::END;
			m_eSkillType = EAugustaSkillType::END;
			m_eUniqueType = EAugustaUniqueType::END;
			m_eBurstType = EAugustaBurstType::END;
			m_eSpecialType = EAugustaSpecialType::END;
			
			// Air
			m_eJumpType = EAugustaJumpType::END;
			m_eFallType = EAugustaFallType::END;
			m_eAirAttackType = EAugustaAirAttackType::END;
			m_eAirSkillType = EAugustaAirSkillType::END;
			m_eAirFlyType = EAugustaAirFlyType::END;

			// Climb
			m_eClimbIdleType = EAugustaClimbIdleType::END;
			m_eClimbMoveType = EAugustaClimbMoveType::END;
			m_eClimbExitType = EAugustaClimbExitType::END;
			m_IsClimbSecondStep = false;

			m_eHitType = EAugustaHitType::END;

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
		PART_BAYONET = 0, // Bayonet
		PART_SKILLWEAPON = 1, // SKill Weapon
		PART_GRIFFON = 2, // Griffon SKILL E UniqueGauge
		PART_WING = 3,
		TYPE_END
	};

#pragma region 0. 
protected:
	explicit CAugusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugusta(const CAugusta& Prototype);
	virtual ~CAugusta() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;
	virtual void	Render_Shadow() override;
	virtual void	Render_OutLine() override;
#pragma endregion


#pragma region 1. STATE.
public:
	virtual void TransitionState_FromPlayer(CHARACTER_TRANSITIONTYPE eTransitionType) override;
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false) override;
	virtual void PartActivate(_uint iPartType, _bool IsActive) override;
	virtual void Part_VolumeChange(_uint iPartType, _uint iVolumeIdx) override;
	virtual void Part_VolumeActivate(_uint iPartType, _bool IsActive) override;
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) override;
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) override;
	virtual void Hit_Judge(void* pArg = nullptr) override;
	void Sync_Position();

	

#ifdef _DEBUG
public:
	virtual void PartRotation(_uint iPartType, _fvector vQuaternion);
#endif // _DEBUG

#pragma region 2. NOTIFY
public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
	virtual void Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void Object_Func(const _wstring& wStrObjectTag) override;

#pragma endregion

#pragma region 3. CALLBACK
	public:
		void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
#pragma endregion


#pragma endregion
private:
	class CAugustaBayonet* m_pBayonet = { nullptr };
	class CAugustaSkillWeapon* m_pSkillWeapon = { nullptr };
	class CAugustaGriffon* m_pGriffon = { nullptr };
	class CWing* m_pWing = { nullptr };

	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };
	_uint m_iCurrentPartType = { PARTTYPE::TYPE_END }; // State


private:
	// Runtime 
	void Bind_Resources();
	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);
	void Ready_AttackVolumes();

public:
	static		CAugusta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

