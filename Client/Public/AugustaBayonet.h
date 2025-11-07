#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CAugustaBayonet final : public CProp
{
public:
	enum VOLUME
	{
		VOLUME_ATTACK = 0,
		VOLUME_STRONG_ATTACK = 1,
		VOLUME_ULTI = 2,
		//VOLUME_EFFECT_GRIFFON = 2,
		VOLUME_END
	};

public:
	typedef struct tagAugustaBayonetDesc : public CProp::PROP_DESC {
		
	} AUGUSTA_BAYONET_DESC;

protected:
	explicit CAugustaBayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugustaBayonet(const CPartObject& Prototype);
	virtual ~CAugustaBayonet() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

	virtual void Activate(_bool IsActivate) override;
	virtual void Change_Volume(_uint iVolumeIdx) override;
	virtual void Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer) override;

	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);


private:
	vector<_uint> m_ShaderPaths = {};


private:
	enum ATK_SOCKET { WEAPON_L, WEAPON_R, WHIP_L, WHIP_R, END };


private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Ready_AttackVolumes();
	void Bind_Resources();

public:
	static CAugustaBayonet* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

