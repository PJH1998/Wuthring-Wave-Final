#pragma once
#include"GameObject.h"


NS_BEGIN(Engine)
class CModel;
class CModel_Instance;
class CShader;
NS_END


NS_BEGIN(Editor)
class CMapObject : public CGameObject
{
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
	//class CModel* m_pModelCom = { nullptr };
	CModel_Instance* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };


private:
	_uint m_iShaderPassIndex = {};
	_uint m_iNumInstance = {};
	_uint m_iPickedInstance = {};
	_float4x4* m_pInstanceMatrix = { nullptr };
	_float4* m_pRotation = { nullptr };

public:
	static CMapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END