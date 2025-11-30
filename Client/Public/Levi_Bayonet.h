#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CAttackVolume;

class CLevi_Bayonet final : public CPartObject
{
public:
	typedef struct tagLeviBayynet : public CPartObject::PART_DESC
	{
		const _float4x4* pSocketMatrix;
		_float3 vOffsetPos;
		_float3 vOffsetRadian;
		_float fAttackDmg;
		TEXT_COLOR_TYPE eType;
		_float4 vBaseColor{ 1.f, 1.f, 1.f, 1.f };
	}LEVIBAYONET_DESC;

private:
	enum SHADERPATH{ BASE, KNIFE, FX, END };

private:
	explicit CLevi_Bayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Bayonet(const CLevi_Bayonet& Prototype);
	virtual ~CLevi_Bayonet() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;


	virtual		void Reset(const _fmatrix& WorldMatrix, void* pArg);
	void Attack_Active(_bool isActive);
	//void Change_Visible(_bool isActive);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAttackVolume* m_pAttackVolume = { nullptr };

	const _float4x4* m_pSocketMatrix = { nullptr };
	vector<_uint> m_ShaderPaths = {};
	_float			m_fRateFX{};
	_float4			m_vBaseColor{};

#ifdef _DEBUG
	_float3 m_vOffsetPos = {};
	_float3 m_vOffsetRot = {};
#else
	_float4x4 m_OffsetMatrix = {};
#endif // _DEBUG

private:
	HRESULT Bind_Resources();
	void Ready_Component(LEVIBAYONET_DESC* pDesc);
	void Ready_Volumes(LEVIBAYONET_DESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
public:
	static CLevi_Bayonet* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
