#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;

class CShaderFilter final : public CBase
{
private:
	CShaderFilter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShaderFilter() = default;

public:
	HRESULT					Initialize();
	HRESULT					Bind_Ramp_Texture(CShader* pShader, const _char* pConstantName);
	HRESULT					Bind_LUT_Texture(CShader* pShader, const _char* pConstantName, _uint iLUT_Index, const _char* pIndexConstantName);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CTexture*					m_pRampTexture = { nullptr };
	CTexture*					m_pLUT_Texture = { nullptr };
	ID3D11ShaderResourceView*	m_pLUT_SRV = { nullptr };
	_uint						m_iNumLUT_Textures = {};

private:
	HRESULT					Ready_Shader_Filters();
	HRESULT					Ready_LUT_SRV();

public:
	static CShaderFilter*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;
};

NS_END