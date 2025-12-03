#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
//class CShader;
//class CModel;
NS_END

NS_BEGIN(Client)
class CLevi_Wave final : public CGameObject
{
public:
	typedef struct tagWaveDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring			wstrEffectTag;
		_float				fAttackDamage;
	}WAVEDESC;

	typedef struct tagWaveReset
	{
	}WAVERESET;
private:
	explicit CLevi_Wave(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Wave(const CLevi_Wave& Prototype);
	virtual ~CLevi_Wave() = default;

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
	//CModel*				m_pModelCom = { nullptr };
	//CShader*			m_pShaderCom = { nullptr };
	_float				m_fLifeTime{};
	_float				m_fMaxLifeTime{};
	_float				m_fDesolveTime{};
	_bool				m_isDisolve{};
	// Effect?
	_wstring			m_wstrEffectTag;
	CALLBACK_CLIENT		m_CallBack{};

private:
	HRESULT		Bind_Resources();
	void		Ready_Component(WAVEDESC* pDesc);
	void		OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CLevi_Wave* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
