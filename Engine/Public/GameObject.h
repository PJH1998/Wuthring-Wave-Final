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
	_uint						Get_ID() { return m_iObjectID; }

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	// 충돌 시, 분기에 따라 호출되는 함수
	// iLayer : 상대의 CollisionLayer
	// pOther : 상대
	// Manifold : 충돌 지점, normal, 겹친 정도를 갖고 있음
	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {}
	virtual		void			OnCollide_End(_uint iLayer, CGameObject* pOther) {}

	// Pooling시, Spawn될 때 초기화 함수
	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };
	class CTransform*			m_pTransformCom = { nullptr };

	map<const _wstring, class CComponent*>	m_Components;

	// 활성화 관련 Bool 변수
	_bool							m_isActivate = { true };
	// Object ID
	_uint							m_iObjectID = {};

protected:
	HRESULT						Add_Component(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg);

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END