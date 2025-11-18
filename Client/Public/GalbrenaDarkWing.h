#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CGalbrenaDarkWing final : public CProp
{
public:
	enum VOLUME
	{
		VOLUME_ATTACK = 0,
		VOLUME_END
	};

protected:
	explicit CGalbrenaDarkWing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CGalbrenaDarkWing(const CPartObject& Prototype);
	virtual ~CGalbrenaDarkWing() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

	
public:
	virtual void Activate(_bool IsActivate) override;
	// Attack Volume
	virtual void Change_Volume(_uint iVolumeIdx) override;
	virtual void Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer) override;

	// Owner의 게이지 채우기?
	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

private:
	vector<_uint> m_ShaderPaths = {};



private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Ready_AttackVolumes();
	void Bind_Resources();

public:
	static CGalbrenaDarkWing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

