#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CAugustaSkillWeapon final : public CProp
{
public:
	enum VOLUME
	{
		VOLUME_ATTACK = 0,
		VOLUME_STRONG_ATTACK = 1,
		VOLUME_EFFECT_GRIFFON = 2,
		VOLUME_END
	};

public:
	typedef struct tagAugustaSkillWeaponDesc : public CProp::PROP_DESC {
		
	} AUGUSTA_SKILLPROP_DESC;

protected:
	explicit CAugustaSkillWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugustaSkillWeapon(const CPartObject& Prototype);
	virtual ~CAugustaSkillWeapon() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;


public:
	virtual void Activate(_bool IsActive) override;

private:
	vector<_uint> m_ShaderPaths = {};



private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Bind_Resources();

public:
	static CAugustaSkillWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

