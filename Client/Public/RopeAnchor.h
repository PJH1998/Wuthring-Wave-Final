#pragma once
#include "GameObject.h"

NS_BEGIN(Client)

// 플레이어가 잡아 당길 용도
class CRopeAnchor final : public CGameObject
{
public:
	typedef struct tagRopeObjectDesc : public CGameObject::GAMEOBJECT_DESC {
		_float fEventDistance = {};
		pair<LEVEL, _wstring> modelData;
		pair<LEVEL, _wstring> shaderData;
		_float3 vScale = {};
		_float3 vRotation = {};
		_float3 vPosition = {};
	}ROPEOBJECT_DESC;

private:
	explicit CRopeAnchor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRopeAnchor(const CRopeAnchor& Prototype);
	virtual ~CRopeAnchor() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;

	// Pooling Spawn CallBack
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) {}


public:
	virtual		void	OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CTransform* m_pTargetTransform = { nullptr };

	
	CALLBACK_CLIENT m_CallBack = {};
	mutex m_Mutex; 
	_uint m_iCondition = {};

	_float m_fTargetDistance = {};
	_float m_fEventDistance = { };

private:
	HRESULT Ready_Components(ROPEOBJECT_DESC* pDesc);

public:
	static	CRopeAnchor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

