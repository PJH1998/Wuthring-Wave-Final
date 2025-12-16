#pragma once
#include "Client_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CRigidbody;
//class CShader;
//class CModel;
NS_END

NS_BEGIN(Client)
class CLevi_Drop final : public CGameObject
{
public:
	typedef struct tagDropDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring			wstrEffectTag;
		_float				fAttackDamage;
	}DROPDESC;

	typedef struct tagDropReset
	{
		_float3				vTargetPos;
	}DROPRESET;
private:
	explicit CLevi_Drop(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Drop(const CLevi_Drop& Prototype);
	virtual ~CLevi_Drop() = default;

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
	_float3				m_vTargetPos{};
	_bool				m_isDisolve{};
	// Effect?
	_bool				m_isLife{};
	_wstring			m_wstrEffectTag;
	CALLBACK_CLIENT		m_CallBack{};

#pragma region SOUND
	_uint					m_iSoundChannel{};
#pragma endregion

private:
	HRESULT		Bind_Resources();
	void		Ready_Component(DROPDESC* pDesc);
	void		OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static		CLevi_Drop* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};
NS_END
