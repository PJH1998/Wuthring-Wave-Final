#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CTexture;
class CVIBuffer_Cube;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Client)

class CScan final : public CGameObject
{
private:
	CScan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CScan(const CScan& Prototype);
	virtual ~CScan() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	_float2					m_vRadius = {};
	_float					m_fCurRadius = {};
	_float					m_fBlendRadius = {};
	_float					m_fSpeed = {};
	_float4					m_vColor = {};
	_float					m_fWidth = {};

	CTexture*				m_pTexture = { nullptr };
	CVIBuffer_Cube*			m_pVIBuffer = { nullptr };
	CShader*				m_pShader = { nullptr };
	CRigidbody*				m_pRigidBody = { nullptr };

	SCAN_INFO				m_tInfo = {};

private:
	HRESULT					Ready_Components();
	HRESULT					Bind_ShaderResources();

public:
	static CScan*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END