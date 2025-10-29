#pragma once
#include "Player_Define.h"
#include "AugustaState_Enum.h"


NS_BEGIN(Client)
class CAugusta final : public CCharacter
{
#pragma region STATE 상태 변경에 사용.
private:
	struct StateTransitionContext
	{
		// Ground
		EDashType m_eDashType = EDashType::END;
		EIdleType  m_eIdleType = EIdleType::END;
		ERunType m_eRunType = ERunType::END;
		ELandType m_eLandType = ELandType::END;

		EAttackType m_eAttackType = EAttackType::END;
		ESkillType m_eSkillType = ESkillType::END;
		EUniqueType m_eUniqueType = EUniqueType::END;
		EBurstType m_eBurstType = EBurstType::END;
		ESpecialType m_eSpecialType = ESpecialType::END;

		// Air
		EJumpType m_eJumpType = EJumpType::END;
		EFallType m_eFallType = EFallType::END;
		EAirAttackType m_eAirAttackType = EAirAttackType::END;
		EAirSkillType m_eAirSkillType = EAirSkillType::END;

		// Climb
		EClimbIdleType m_eClimbIdleType = EClimbIdleType::END;
		EClimbMoveType m_eClimbMoveType = EClimbMoveType::END;
		_bool m_IsClimbSecondStep = { false };

		EClimbExitType m_eClimbExitType = EClimbExitType::END;
		
		// 컨텍스트 사용 뒤 초기화
		void Clear()
		{
			m_eIdleType = EIdleType::END;
			m_eRunType = ERunType::END;
			m_eDashType = EDashType::END;
			m_eLandType = ELandType::END;

			m_eAttackType = EAttackType::END;
			m_eSkillType = ESkillType::END;
			m_eUniqueType = EUniqueType::END;
			m_eBurstType = EBurstType::END;
			m_eSpecialType = ESpecialType::END;
			
			m_eJumpType = EJumpType::END;
			m_eFallType = EFallType::END;
			m_eAirAttackType = EAirAttackType::END;
			m_eAirSkillType = EAirSkillType::END;

			m_eClimbIdleType = EClimbIdleType::END;
			m_eClimbMoveType = EClimbMoveType::END;
			m_eClimbExitType = EClimbExitType::END;
			m_IsClimbSecondStep = false;
		};
	};

	StateTransitionContext m_StateContext;


public:
	// 현재 State에서 호출
	StateTransitionContext& GetStateContextForWrite()
	{
		return m_StateContext;
	};

	// 호출 받는 State
	StateTransitionContext TakeStateContext()
	{
		StateTransitionContext tempCopy = m_StateContext; // 현재 컨텍스트를 복사
		m_StateContext = {}; // 원본 컨텍스트를 즉시 비움 기본값 초기화)
		return tempCopy; // 복사본을 반환
	}

#pragma endregion
public:
	enum PARTTYPE : _uint
	{
		PART_BAYONET = 0, // Bayonet
		PART_SKILLWEAPON = 1, // SKill Weapon
		PART_GRIFFON = 2, // Griffon SKILL E UniqueGauge
		TYPE_END
	};

#pragma region 0. 기본 함수
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
#pragma endregion


#pragma region 1. STATE 관리.
public:
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true) override;
	virtual void PartActivate(_uint iPartType, _bool IsActive) override;
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) override;
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) override;
	void Sync_Position();

#ifdef _DEBUG
public:
	virtual void PartRotation(_uint iPartType, _fvector vQuaternion);
#endif // _DEBUG

#pragma region 2. NOTIFY
	public:
		virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) override;
		virtual void Effect_Active(const _wstring& wStrEffectTag) override;

#pragma endregion


#pragma endregion


private:
	class CAugustaBayonet* m_pBayonet = { nullptr };
	class CAugustaSkillWeapon* m_pSkillWeapon = { nullptr };
	class CAugustaGriffon* m_pGriffon = { nullptr };
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };
	_uint m_iCurrentPartType = { PARTTYPE::TYPE_END }; // State마다 활성화?

	
#ifdef _DEBUG
	// RayCast 저장
	vector<pair<_float, _float>> m_RayCasts = {};
#endif // _DEBUG



private:
	// Runtime 도중 필요한 값에 대한 준비.
	void Bind_Resources();

	// 초기 값에 대한 준비.
	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);

public:
	static		CAugusta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

