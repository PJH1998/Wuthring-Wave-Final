#pragma once
#include "Prop.h"

NS_BEGIN(Client)
class CAugustaEnergyBlade final : public CProp
{
public:
	enum VOLUME
	{
		VOLUME_ULTI = 0,         // X, Z 길게, Y짧게
		VOLUME_SWORD_ATTACK = 1, // 기본 Attack과 동일하게(Bayonet) 좀더 크게>
		VOLUME_SWORD_ULTI = 2,   // 내가 보고 있는 범위 전체?(크게)
		VOLUME_END
	};

public:
	typedef struct tagAugustaSkillWeaponDesc : public CProp::PROP_DESC {

	} AUGUSTA_SKILLPROP_DESC;

private:
	explicit CAugustaEnergyBlade(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAugustaEnergyBlade(const CPartObject& Prototype);
	virtual ~CAugustaEnergyBlade() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

	void Set_ParentMatrixPtr(const _float4x4* pParentMatrixPtr) { m_pParentWorldMatrix = pParentMatrixPtr; }


public:
	virtual void Activate(_bool IsActivate) override;

	// Owner의 게이지 채우기?
	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

private:
	vector<_uint> m_ShaderPaths = {};
	const _float4x4* m_pParentWorldMatrix = { nullptr };


	// Shader 변수
	//_float4 m_vEnergyColor = {};
	_float  m_fTime = { };
	//_float  m_fEnergyIntensity = {};
	_float2 m_vScrollSpeed = {}; // x, y
	

private:
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);
	void Bind_Resources();

public:
	static CAugustaEnergyBlade* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

