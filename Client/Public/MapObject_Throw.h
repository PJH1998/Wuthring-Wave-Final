#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CRigidbody;
class CModel_Streaming;
NS_END

NS_BEGIN(Client)
class CMapObject_Throw final : public CGameObject
{
public:
	typedef struct tagMapLoad
	{

		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		OBJECTTYPE eObjectType;
		_uint iLevel = {};
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
	}MAP_LOAD;

private:
	CMapObject_Throw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject_Throw(const CMapObject_Throw& Prototype);
	virtual ~CMapObject_Throw() = default;


public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
private:
	void						Change_Level();
	void						Ready_Components(void* pArg);

	void						Collide();
	void						Graped();
private:
	class CModel_Streaming* m_pModelCom = {};
	CShader* m_pShaderCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	CRigidbody* m_pDetectRigidbodyCom = { nullptr };
	CRigidbody* m_pCollideRigidbodyCom = { nullptr };

	_uint m_iShaderPassIndex = {};
	_float3 m_vTargetPos = {};
	_bool m_IsThrowed = {false};
	_float m_fThrowTime = {};
	_float3 m_vStartPos = {};
	_float3 m_vImpulse = {};
 public:
	static CMapObject_Throw* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END