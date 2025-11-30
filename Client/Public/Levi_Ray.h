#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CLevi_Ray final : public CGameObject
{
	typedef struct tagLeviRayDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring			wstrEffectTag;
		_wstring			wstrModelTag;
		_uint				iLayer;
		vector<_uint>		iTargetLayers;
		_float				fAttackDamage;
		_float3				vExtent;
		TEXT_COLOR_TYPE		eType;
	}LEVIRAY_DESC;

	typedef struct tagLeviRayReset
	{
		_float3				vTargetPos;
	}LEVIRAY_RESET;

private:
	explicit CLevi_Ray(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Ray(const CLevi_Ray& Prototype);
	virtual ~CLevi_Ray() = default;

public:
	virtual		HRESULT					Initialize_Prototype() override;
	virtual		HRESULT					Initialize_Clone(void* pArg) override;
	virtual		void					Priority_Update(_float fTimeDelta) override;
	virtual		void					Update(_float fTimeDelta)override;
	virtual		void					Late_Update(_float fTimeDelta) override;
	virtual		void					Render() override;

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	CRigidbody* m_pRigidBodyCom = { nullptr };
	_uint				m_iLayer{};
	vector<_uint>		m_iTargetLayers;
	_float				m_fLifeTime{};
	_float				m_fDelay{};
	// Effect?
	_wstring			m_wstrEffectTag;
	CALLBACK_CLIENT m_CallBack{};

private:
	HRESULT		Bind_Resources();
	void		Ready_Component(LEVIRAY_DESC* pDesc);
	void		OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CLevi_Ray* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
