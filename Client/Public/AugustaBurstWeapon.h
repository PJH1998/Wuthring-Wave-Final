#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CAugustaBurstWeapon final : public CProp
{
public:
	enum VOLUME
	{
		VOLUME_SWORD_ATTACK = 0, // 기본 Attack과 동일하게(Bayonet) 좀더 크게>
		VOLUME_SWORD_ULTI = 1,   // 내가 보고 있는 범위 전체?(크게)
		VOLUME_END
	};

	enum MESHTYPE
	{
		MESH_DEFAULT = 0,
		MESH_EFFECT = 1,
		MESH_END
	};

public:
	typedef struct tagAugustaSkillWeaponDesc : public CProp::PROP_DESC {
		
	} AUGUSTA_SKILLPROP_DESC;

private:
	explicit CAugustaBurstWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugustaBurstWeapon(const CPartObject& Prototype);
	virtual ~CAugustaBurstWeapon() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;


#ifdef _DEBUG
	void Debug_Emissive(_float4 vEmissiveColor, _float fIntensity);
#endif // _DEBUG

	

public:
	virtual void Activate(_bool IsActivate) override;
	virtual void Change_Volume(_uint iVolumeIdx) override;
	virtual void Change_VolumeLayer(_uint iVolumeIdx, COLLISIONLAYER eLayer) override;

	// Owner의 게이지 채우기?
	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

private:
	vector<_uint> m_ShaderPaths = {};

	// Shader 변수
	_float m_fTime = {}; // UV 흐름을 주기 위한 변수.

private:
	_bool IsEffect(_uint iMeshIndex);

	void Render_Default(_uint iMeshIndex);
	void Render_Effect(_uint iMeshIndex);

private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Ready_AttackVolumes();
	void Bind_Resources();

public:
	static CAugustaBurstWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

