#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CShadowDummy : public CGameObject
{
private:
	explicit CShadowDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CShadowDummy(const CShadowDummy& Prototype);
	virtual ~CShadowDummy() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;
	
	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	
private:
	void						Ready_Component();

public:
	static	CShadowDummy*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject*	Clone(void* pArg) override;
	virtual	void			Free() override;
};

NS_END