#pragma once
#include "C:\Users\dnheu\source\repos\Wuthering_Wave_Final\EngineSDK\Inc\GameObject.h"
#include"Editor_Enum.h"
NS_BEGIN(Engine)
class CRigidbody;
NS_END

NS_BEGIN(Editor)
class CEdit_TriggerBox final: public CGameObject
{
public:
	using TriggerCallback = function<void(void*)>;
	typedef struct tagTriggerBox
	{
		_float4x4* WorldMatrix = { nullptr };
		_float3 vExtends;
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;

	}TRIGGER;

private:
	CEdit_TriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_TriggerBox(const CEdit_TriggerBox& Prototype);
	virtual ~CEdit_TriggerBox() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;

private:
	void Ready_Components(void* pArg);

private:
	BoundingBox* m_pBoundingBox = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };

	void Collision();

	void CallBack(_uint iFuncIndex,void* pArg);
private:
	vector<CGameObject*> m_CurrentFrame;
	vector<CGameObject*> m_LastFrame;

	vector< TriggerCallback> m_Functions;
public:
	static CEdit_TriggerBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END