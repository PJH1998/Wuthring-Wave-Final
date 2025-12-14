#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CComputeShader;
class CShader;
class CModel;
class CRigidbody;
//class CAnimMachine;
NS_END

NS_BEGIN(Client)
class CGameSystem;

class CLevi_Alter final : public CActor
{
public:
	enum ATTACK_TYPE {SWORD, BOW};
	typedef struct tagAlterDesc : public CActor::ACTOR_DESC
	{
		_float fAttackDmg;
		_float3 vDetectRange;
	}ALTER_DESC;

	typedef struct tagAlterReset
	{
		//const _float4x4* pWorldMatrix;
		_float3 vInitPosition{};
		_float3 vLookAt{};
		_string strPatternKey;
		//const _float4x4* pRootMatrix;
		ATTACK_TYPE eType;

	}ALTER_RESET;

private:
	enum ALTER_SHADER { BANG, HAIR, FACE, UP, DOWN, CLOTH, ALPHA, FX };

private:
	explicit CLevi_Alter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Alter(const CLevi_Alter& Prototype);
	virtual ~CLevi_Alter() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg);

public:
	virtual void	Collider_Active(const _wstring& wStrColliderTag, _bool Isactive) override;
	virtual void	Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void	Object_Func(const _wstring& wStrObjectTag) override;
	void			Sound_Active(const _wstring& wStrObjectTag);

private:
	CGameSystem*			m_pGameSystem = { nullptr };

	//CAnimMachine* m_pAnimMachineCom = { nullptr };
	ATTACK_TYPE				m_eType{};
	_float4					m_vBaseColor{};
	_float3					m_vTargetPosition{};
	_float3					m_vTargetDir{};
	_float					m_fDistanceNonY{};
	_string					m_strAnimKey;
	_string					m_strPatternKey;
	_float					m_fAttackDmg{};
	vector<_uint>			m_ShaderIndices;
	map<const _string, pair<_float, _float>> m_Tracks;

	_bool					m_isTurnLerp{};
	_bool					m_isDist_Interp_Enable{};
	_float					m_fRootMotionRate{};

#pragma region SOUND
	_uint					m_iSoundChannel{};
	_uint					m_iSoundChannel2{};
#pragma endregion

#pragma region SHADER_VALUE
	_float					m_fDissolveRate{};
	_bool					m_isDissolve{};
	_float4					m_vMonsterDissolveColor{};
#pragma endregion

private:
	void			Bind_Resources();
	void			Ready_Component(ALTER_DESC* pDesc);
	void			Ready_PartObject(ALTER_DESC* pDesc);
	void			OnHit_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void			OnDetect_During(_uint iLayer, void* pOther, const ContactManifold& Manifold);
	void			UnActive_Resources();
	void			Reset_NotifyInteraction();

public:
	static CLevi_Alter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual	void Free() override;
};

NS_END