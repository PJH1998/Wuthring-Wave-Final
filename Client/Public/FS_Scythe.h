#pragma once
#include "Actor.h"

NS_BEGIN(Engine)
class CComputeShader;
class CShader;
class CModel;
class CRigidbody;
class CAnimMachine;
NS_END

NS_BEGIN(Client)
class CAttackVolume;
class CGameSystem;

class CFS_Scythe final : public CActor
{
public:
	typedef struct tagScytheDesc : public CActor::ACTOR_DESC
	{
		_float fAttackDamage;
	}SCYTHE_DESC;

	typedef struct tagsScytheReset
	{
		_float fChangeTrackPos;
		_string strPatternKey;

	}SCYTHE_RESET;

private:
	explicit CFS_Scythe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CFS_Scythe(const CFS_Scythe& Prototype);
	virtual ~CFS_Scythe() = default;

public:
	virtual	HRESULT				Initialize_Prototype() override;
	virtual	HRESULT				Initialize_Clone(void* pArg) override;
	virtual	void				Priority_Update(_float fTimeDelta) override;
	virtual	void				Update(_float fTimeDelta) override;
	virtual	void				Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg);

	virtual void	Collider_Active(const _wstring& wStrColliderTag, _bool Isactive) override;
	virtual void	Effect_Active(const _wstring& wStrEffectTag) override;
	virtual void	Object_Func(const _wstring& wStrObjectTag) override;

private:
	CAttackVolume*			m_pAttackVolumes[5] = {nullptr,};
	CAnimMachine*			m_pAnimMachineCom = { nullptr };
	CGameSystem*			m_pGameSystem = { nullptr };

	_bool		m_isVolumeActive[5]{};
	_string		m_strAnimKey;
	_uint		m_iState{};
	_float		m_fLifeTime{};

	_float m_fAttackDamage{};

private:
	void			Bind_Resources();
	void			Ready_Component(SCYTHE_DESC* pDesc);
	void			Ready_PartObjects(SCYTHE_DESC* pDesc);
	void			OnHit_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold, COLLISIONLAYER eVolumeLayer);
public:
	static CFS_Scythe* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual	void Free() override;
};
NS_END
