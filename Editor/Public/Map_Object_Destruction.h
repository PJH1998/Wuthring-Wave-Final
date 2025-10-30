#pragma once
#include"StaticObject.h"
#include"Editor_Enum.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Editor)
class CMap_Object_Moveable :
    public CStaticObject
{
private:
	CMap_Object_Moveable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMap_Object_Moveable(const CMap_Object_Moveable& Prototype);
	virtual ~CMap_Object_Moveable() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();


	virtual void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

	virtual void Bind_Resources();

	//_char* Get_ModelName() { return m_ModelName; }
private:
	class CMap_Interface* m_pMapInterface = { nullptr };
	_uint m_iShaderPassIndex = {};
	_uint m_iLevel = {};
	OBJECTTYPE m_eObjectType = { OBJECTTYPE::END };
	vector<CModel*> m_pModelComArray;
	CRigidbody* m_pRigidbodyCom = { nullptr };
public:
	static CMap_Object_Moveable* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;


};

NS_END