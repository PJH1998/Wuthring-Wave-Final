#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CGameSystem;

class CProjectile final : public CGameObject
{
public:
	typedef struct tagProjectileDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring			wstrEffectTag;
		_wstring			wstrModelTag;
		_uint				iLayer;
		vector<_uint>		iTargetLayers;
		_float				fRadius;
		_float				fAttackDamage;
		_float				fLifeTime{ 10.f };
		_float				fMaxDelay{ 1.f };
		_bool				isCollisionDestroy{ true };
		TEXT_COLOR_TYPE		eType;
	}PROJECTILEDESC;

	typedef struct tagProjectileReset
	{
		_float3				vTargetPos;
		CTransform*			pOwnerTransform;
		_wstring			wstrSoundTag;
	}PROJECTILERESET;
private:
	explicit CProjectile(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CProjectile(const CProjectile& Prototype);
	virtual ~CProjectile() = default;

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	CRigidbody*			m_pRigidBodyCom = { nullptr };
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	CGameSystem*		m_pGameSystem = { nullptr };
	_uint				m_iLayer{};
	vector<_uint>		m_iTargetLayers;
	_bool				m_isCollision{};
	_bool				m_isCollisionDestroy{};
	_float				m_fLifeTime{};
	_float				m_fMaxLifeTime{};
	_float				m_fDelay{};
	_float				m_fMaxDelay{};
	// Effect?
	_wstring			m_wstrEffectTag;
	CALLBACK_CLIENT		m_CallBack{};

	_int				m_iSoundChannel{ -1 };

private:
	HRESULT		Bind_Resources();
	void		Ready_Component(PROJECTILEDESC* pDesc);
	void		OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CProjectile* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
