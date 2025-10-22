#pragma once
#include"Edit_MapObject.h"


NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
class CTexture;
NS_END


NS_BEGIN(Editor)
class CEdit_MapObject_Sonora : public CEdit_MapObject
{
protected:
	CEdit_MapObject_Sonora(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject_Sonora(const CEdit_MapObject_Sonora& Prototype);
	virtual ~CEdit_MapObject_Sonora() = default;

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

	_char* Get_ModelName() { return m_ModelName; }

	void Add_Child(CEdit_MapObject_Sonora* pObject);
	void Quit_Child(CEdit_MapObject_Sonora* pObject);
	void Make_ChildLocalMatrix(_fmatrix ParentMatrix);
	void Set_ShaderPass(_uint i) { m_iShaderPassIndex = i; }

public:
	static CEdit_MapObject_Sonora* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END