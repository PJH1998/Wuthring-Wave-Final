#pragma once
#include "SequencePlayerDefine.h"
#include "YunoState_Enum.h"

NS_BEGIN(Client)
class CYuno final : public CCharacter
{
public:
	enum VOLUME
	{
		VOLUME_ATTACK = 0,
		VOLUME_SKILL = 1,
		VOLUME_END
	};

#pragma region STATE
private:
	struct StateTransitionContext
	{
		// Prev Info
		EYunoIdleType m_eIdleType = EYunoIdleType::END;
		EYunoAirAttackType m_eAirAttackType = EYunoAirAttackType::END;
		_string m_strPrevInfo = {};
		void Clear()
		{
			m_eIdleType = EYunoIdleType::END;
			m_strPrevInfo.clear();
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
		PART_MOON = 0,
		TYPE_END
	};

#pragma region 0. 
private:
	explicit CYuno(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CYuno(const CYuno& Prototype);
	virtual ~CYuno() = default;

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
	virtual void Play_PartAnimation(_uint iPartType, const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);
	virtual void PartActivate(_uint iPartType, _bool IsActive) override;
	virtual void Part_VolumeChange(_uint iPartType, _uint iVolumeIdx) override;
	virtual void Part_VolumeActivate(_uint iPartType, _bool IsActive) override;
	virtual void Clear_PartAnimation(_uint iPartType, const _string& strAnimName) override;
	virtual void Set_SocketMatrixToParts(_uint iPartType, const _string& strBoneName) override;
	void Sync_Position();

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
	virtual void Bind_ChangeEffect() override; // ChaneEffect 실행.

	virtual void Bind_DissolveTimer() override;
	virtual void Bind_DefaultShaderPath() override;
	virtual void Bind_DissolveShaderPath() override;
	virtual void Activate(_bool IsActivate) override;
#pragma endregion

#pragma endregion
private:
	_string m_strPreAnimation = {};
	_string m_strCurrentAnimation = {};
	_bool m_IsPlayAnimation = { true };
	_uint m_iCurrentPartType = { PARTTYPE::TYPE_END }; 

	
	// Attack Volume
	_uint m_iVolumeIdx = {};
	vector<class CAttackVolume*> m_AttackVolumes;

	class CYunoMoon* m_pYunoMoon = { nullptr };

private:
	void Update_Physics(_float fTimeDelta);
	void Update_Camera(_float fTimeDelta);
	void Update_TargetDistance(_float fTimeDelta);

private:
	void Bind_Resources();

	void Ready_Components(const CHARACTER_DESC* pDesc);
	void Ready_Variables(const CHARACTER_DESC* pDesc);
	void Ready_Positions(const CHARACTER_DESC* pDesc);
	void Ready_PartObjects(const CHARACTER_DESC* pDesc);
	void Ready_AttackVolumes();

public:
	static		CYuno* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

