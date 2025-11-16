#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CAoEDoT final : public CGameObject
{
public:
	typedef struct tagAoEDoTDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring			wstrEffectTag;
		_uint				iLayer;
		vector<_uint>		iTargetLayers;
		_float3				vExtent;
		_float3				vOffset;
		_float				fAttackDamage;
		TEXT_COLOR_TYPE		eType;
	}AOEDOT_DESC;

	typedef struct tagAoEDoTReset
	{
		_float				fLifeTime;
		_uint				iTickCount;
	}AOEDOT_RESET;
private:
	explicit CAoEDoT(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAoEDoT(const CAoEDoT& Prototype);
	virtual ~CAoEDoT() = default;

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
	_uint				m_iLayer{};
	vector<_uint>		m_iTargetLayers;
	_float3				m_vOffset{};
	_float				m_fLifeTime{};
	_float				m_fLifeTimeAcc{};
	_float				m_fDelayTime{};
	_float				m_fDelayAcc{};
	_bool				m_isAttack{};
	// Effect?
	_wstring			m_wstrEffectTag;
	CALLBACK_CLIENT m_CallBack{};

private:
	void Ready_Component(AOEDOT_DESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CAoEDoT* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
