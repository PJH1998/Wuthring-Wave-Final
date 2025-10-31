#pragma once
#include "Weapon.h"

NS_BEGIN(Client)
class CRoverSword final : public CWeapon
{
public:
	typedef struct tagRoverWeaponDesc : public CWeapon::WEAPON_DESC {
		
	} ROVER_WEAPON_DESC;

protected:
	explicit CRoverSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CRoverSword(const CPartObject& Prototype);
	virtual ~CRoverSword() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;


private:
	vector<_uint> m_ShaderPaths = {};



private:
	void Ready_Components(const WEAPON_DESC* pDesc);
	void Ready_Variables(const WEAPON_DESC* pDesc);
	void Ready_Positions(const WEAPON_DESC* pDesc);
	void Bind_Resources();

public:
	static CRoverSword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

