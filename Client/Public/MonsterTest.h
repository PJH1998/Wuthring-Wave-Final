#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
//class CRigidbody;
//class CCollider;
NS_END

NS_BEGIN(Client)

class CMonsterTest final : public CGameObject
{
public:
	typedef struct tagMonsterTestDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szPrototypeModelTag;
	}MONSTERTEST_DESC;

private:
	explicit CMonsterTest(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMonsterTest(const CMonsterTest& Prototype);
	virtual ~CMonsterTest() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

	//virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}
	//virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	//CRigidbody*			m_pRigidbodyCom = { nullptr };
	//CCollider*				m_pColliderCom = { nullptr };
	CBehavior_Tree*			m_pBehaviorTreeCom = { nullptr };

	CTransform*				m_pTargetTransformCom = { nullptr };

	_uint					m_iState{};
	_int					m_iHP{};
	_bool					m_isAnimationFinished{};

private:
	void						Ready_Component(MONSTERTEST_DESC* pDesc);

public:
	static		CMonsterTest*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END