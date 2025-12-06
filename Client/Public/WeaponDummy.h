#pragma once
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CWeaponDummy final : public CPartObject
{
public:
	typedef struct tagWeaponDummyDesc : public CPartObject::PART_DESC
	{
		const _float4x4* pSocketMatrix;
		_float3 vOffsetPos;
		_float3 vOffsetRadian;
		_wstring wstrModelTag;
	}WD_DESC;

private:
	enum SHADERPATH{ BASE, BAOSHI, SUISHI, LINE, END };

private:
	explicit CWeaponDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CWeaponDummy(const CWeaponDummy& Prototype);
	virtual ~CWeaponDummy() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;

	virtual	void	Reset(const _fmatrix& WorldMatrix, void* pArg) {};

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

	const _float4x4* m_pSocketMatrix = { nullptr };

#ifdef _DEBUG
	_float3 m_vOffsetPos = {};
	_float3 m_vOffsetRot = {};
#else
	_float4x4 m_OffsetMatrix = {};
#endif // _DEBUG

private:
	HRESULT Bind_Resources();
	void Ready_Component(WD_DESC* pDesc);
public:
	static CWeaponDummy* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
