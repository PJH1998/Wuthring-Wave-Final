#pragma once
#include "Base.h"
#include "Transform.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC {
		
	}GAMEOBJECT_DESC;
protected:
	explicit CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	class CComponent*	Get_Component(const _wstring& strComponentTag);
	_bool						IsActivate() { return m_isActivate; }
	void						SetActivate(_bool isActivate) { m_isActivate = isActivate; }
	void						SetBlock(const _fvector& vTranslation) {
		XMStoreFloat3(&m_vBlock, vTranslation);
		m_isBlock = true;
	}

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		HRESULT		Render();
	virtual		HRESULT		Render_Shadow();

	virtual		void			OnCollide_Begin(class CCollider* pCollider, class CCollider* pOtherCollider, _uint iLayer) {}
	virtual		void			OnCollide_OnGoing(class CCollider* pCollider, class CCollider* pOtherCollider, _uint iLayer) {}
	virtual		void			OnCollide_End(class CCollider* pCollider, class CCollider* pOtherCollider, _uint iLayer) {}

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	class CGameInstance*	m_pGameInstance = { nullptr };
	class CTransform*			m_pTransformCom = { nullptr };

	map<const _wstring, class CComponent*>	m_Components;

	_bool							m_isActivate = { true };

	_bool							m_isBlock = { false };
	_float3						m_vBlock = {};

	_float3						m_vPrePosition = {};

protected:
	HRESULT						Add_Component(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg);
	void							Block(class CNavigation* pNavigation);

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END