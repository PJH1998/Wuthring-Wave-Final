#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CLevi_Bow final : public CPartObject
{
public:
	typedef struct tagLeviBayynet : public CPartObject::PART_DESC
	{
		const _float4x4* pSocketMatrix;
		_float3 vOffsetPos;
		_float3 vOffsetRadian;
		_float fAttackDmg;
	}LEVIBAYONET_DESC;

private:
	enum SHADERPATH{ BASE, BAOSHI, SUISHI, LINE, END };

private:
	explicit CLevi_Bow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CLevi_Bow(const CLevi_Bow& Prototype);
	virtual ~CLevi_Bow() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	const _float4x4* m_pSocketMatrix = { nullptr };
	vector<_uint> m_ShaderPaths = {};
	_float			m_fRateFX{};

#ifdef _DEBUG
	_float3 m_vOffsetPos = {};
	_float3 m_vOffsetRot = {};
#else
	_float4x4 m_OffsetMatrix = {};
#endif // _DEBUG

private:
	HRESULT Bind_Resources();
	void Ready_Component(LEVIBAYONET_DESC* pDesc);
	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
public:
	static CLevi_Bow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
