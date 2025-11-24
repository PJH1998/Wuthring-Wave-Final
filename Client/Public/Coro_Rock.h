#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CRigidbody;
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CCoro_Rock final : public CPartObject
{
public:
	typedef struct tagCoroRockDesc : public CPartObject::PART_DESC
	{
		const _float4x4* pSocketMatrix = {};
		_float3 vOffsetTrans ;
		_float3 vOffsetRadian;
		_float fAttackDmg;
	}CORO_ROCK_DESC;

public:
	explicit CCoro_Rock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCoro_Rock(const CCoro_Rock& Prototype);
	virtual ~CCoro_Rock() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;

public:
	void Change_Layer(_uint iLayer);

private:
	CRigidbody* m_pRigidBodyCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	const _float4x4* m_pSocketMatrix = { nullptr };
#ifdef _DEBUG
	_float3 m_vOffsetTrans = {};
	_float3 m_vOffsetRotate = {};
#else
	_float4x4 m_OffsetMatrix = {};
#endif // _DEBUG
private:
	HRESULT		Bind_Resources();
	void		Ready_Component(CORO_ROCK_DESC* pDesc);

	void OnCollide_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

public:
	static CCoro_Rock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
