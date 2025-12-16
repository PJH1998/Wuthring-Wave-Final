#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CAugustaHeadProp final : public CProp
{
public:
	typedef struct tagAugustaHeadPropDesc : public CProp::PROP_DESC {
		
	} AUGUSTA_HEADPROP_DESC;

private:
	explicit CAugustaHeadProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugustaHeadProp(const CPartObject& Prototype);
	virtual ~CAugustaHeadProp() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;
	virtual void Render_Shadow() override;

public:
	virtual void Activate(_bool IsActivate) override;

private:
	// Shader 변수
	_float  m_fTime = { };
	_float2 m_vScrollSpeed = {}; // x, y



private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Bind_Resources();

public:
	static CAugustaHeadProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

