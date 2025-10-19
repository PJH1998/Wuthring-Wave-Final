#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CMeshMaterial final : public CBase
{
private:
	explicit CMeshMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMeshMaterial() = default;

public:
	HRESULT						Initialize(const _char* pFilePath, const json& MaterialData);
	HRESULT						Bind_Resource(class CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT						Bind_Resource(class CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	vector<ID3D11ShaderResourceView*> m_SRVs[ENUM_CLASS(TEXTURETYPE::END)];

private:
	HRESULT						Load_File(const _char* pFilePath, const json& MaterialData, const _char* pTextureTag, TEXTURETYPE eTextureType);

public:
	static CMeshMaterial*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, const json& MaterialData);
	virtual void					Free() override;
};

NS_END