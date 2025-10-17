#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)
class CEdit_LightObject final : public CGameObject
{
private:
	explicit CEdit_LightObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEdit_LightObject(const CEdit_LightObject& Prototype);
	virtual ~CEdit_LightObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;
	virtual		void			Render_Shadow()override;

private:
	LIGHT_DESC* m_LightDesc = {};
public:
	static CEdit_LightObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END