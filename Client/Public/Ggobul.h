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
class CGgobul final : public CActor
{
public:
	enum GGOBULTYPE { HEAD, HAMMER, KNIFE, END};
	typedef struct tagGgobulDesc : public CActor::ACTOR_DESC
	{
	}GGOBUL_DESC;

	typedef struct tagGgobulReset
	{
		//const _float4x4* pWorldMatrix;
		//_float3 vInitPosition{};
		//_float3 vInitDirection{};
		GGOBULTYPE eType;
		_float fChangeTrackPos;
		_string strPatternKey;

	}GGOBUL_RESET;

private:
	explicit CGgobul(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CGgobul(const CGgobul& Prototype);
	virtual ~CGgobul() = default;

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
	//CComputeShader*		m_pComputeShaderCom = { nullptr };
	//CShader*			m_pShaderCom = { nullptr };
	//CModel*				m_pModelCom = { nullptr };
	CRigidbody*			m_pAttackVolume[GGOBULTYPE::END] = {nullptr,};
	CAnimMachine*		m_pAnimMachineCom = { nullptr };

	GGOBULTYPE				m_eType{ GGOBULTYPE::END};
	const _float4x4*		m_pAttackTransform = { nullptr };
	_float4x4				m_BoneCombindMatrix{};
	vector<_bool>			m_MeshEnables;

	_string		m_strAnimKey;
	_uint		m_iState{};
	//_bool m_isRootMotion;
	//_bool m_isRootMotionRotate;
	//_bool m_isRootMotionTranslate;
	//_float m_fRootMotionRate;
	//_float m_fAnimationSpeed;

	_float m_fAttackDamage{};

private:
	void			Bind_Resources();
	void			Ready_Component(GGOBUL_DESC* pDesc);
	void			OnCollide_Enter(_uint iLayer, void* pOther, const ContactManifold& Manifold);
public:
	static CGgobul* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual	void Free() override;
};
NS_END
