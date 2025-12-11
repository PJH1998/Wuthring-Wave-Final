#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CRoverDarkWing final : public CProp
{
public:
	enum VOLUME
	{
		VOLUME_ATTACK = 0,
		VOLUME_END
	};

private:
	explicit CRoverDarkWing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRoverDarkWing(const CPartObject& Prototype);
	virtual ~CRoverDarkWing() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;
	virtual void Render_Shadow() override;
	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

public:
	virtual void Activate(_bool IsActivate) override;

private:
	vector<_uint> m_ShaderPaths = {};



private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Ready_AttackVolumes();
	void Bind_Resources();

public:
	static CRoverDarkWing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

