#pragma once
#include"GameObject.h"


NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
NS_END


NS_BEGIN(Editor)
class CMapObject : public CGameObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
	}MAP_LOAD;

private:
	CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject(const CMapObject& Prototype);
	virtual ~CMapObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

	void Bind_Resources();

private:
	CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };

	CModel* m_pModelComArray[4] = { nullptr, nullptr, nullptr, nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
private:
#ifdef _DEBUG
	_char m_ModelName[MAX_PATH];
#endif

	_uint m_iShaderPassIndex = {};
	_float3 m_vScale = {};
	_float3 m_vRotation= {};
	_float3 m_vTranslation = {};

	_float3 m_vNewScale = {};
	_float3 m_vNewRotation = {};
	_float3 m_vNewTranslation = {};
public:
	static CMapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END