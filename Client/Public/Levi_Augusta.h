#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
class CAttackVolume;

class CLevi_Augusta final : public CPartObject
{
public:
	typedef struct tagLeviAugustaDesc : public CPartObject::PART_DESC {
		const _float4x4* pSocketMatrix;
		_float3 vOffsetPos;
		_float3 vOffsetRadian;
		_float fAttackDmg;
		LEVEL eLevel;
	} LEVIAUG_DESC;

	typedef struct tagLeviAugustaReset
	{
		COLLISIONLAYER eLayer;
	}LEVI_AUG_RESET;

private:
	enum SHADERPATH { BASE, BAOSHI, SUISHI, LINE, END };

private:
	explicit CLevi_Augusta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Augusta(const CPartObject& Prototype);
	virtual ~CLevi_Augusta() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

public:
	virtual void	Change_Volume(COLLISIONLAYER eLayer);
	void			Attack_Active(_bool isActive);
	// Owner의 게이지 채우기?
	virtual void OnHitEnter(_uint iLayer, void* pOther, const ContactManifold& Manifold);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAttackVolume* m_pAttackVolume = { nullptr };

	const _float4x4* m_pSocketMatrix = { nullptr };
	vector<_uint> m_ShaderPaths = {};

	_float4			m_vBaseColor{};

#ifdef _DEBUG
	_float3 m_vOffsetPos = {};
	_float3 m_vOffsetRot = {};
#else
	_float4x4 m_OffsetMatrix = {};
#endif // _DEBUG

private:
	void Ready_Components(const LEVIAUG_DESC* pDesc);
	void Ready_Variables(const LEVIAUG_DESC* pDesc);
	void Ready_AttackVolumes(const LEVIAUG_DESC* pDesc);
	void Bind_Resources();

public:
	static CLevi_Augusta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;

};
NS_END

