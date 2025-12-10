#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CHeaven_SkyBox final : public CGameObject
{
private:
	enum class SKYBOX { DOME, CLOUD, FX, END };

private:
	explicit CHeaven_SkyBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CHeaven_SkyBox(const CHeaven_SkyBox& Prototype);
	virtual ~CHeaven_SkyBox() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) override;
	
private:
	CShader*				m_pShader = { nullptr };
	CModel*					m_pModel[ENUM_CLASS(SKYBOX::END)] = { nullptr };

	_uint					m_iShaderIndex[ENUM_CLASS(SKYBOX::END)] = {};

	_float					m_fTime = {};
	_float					m_fCloudSpeed = {};
	
private:
	HRESULT					Ready_Component();

public:
	static		CHeaven_SkyBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*	Clone(void* pArg) override;
	virtual		void			Free() override;
};

NS_END