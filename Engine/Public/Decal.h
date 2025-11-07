#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CVIBuffer_Decal;
class CShader;
class CGameInstance;

class CDecal final : public CBase
{
private:
	CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal() = default;

public:
	HRESULT						Initialize();
	void						Update(_float fTimeDelta);
	void						Render(CShader* pShader);
	
	HRESULT						Add_DecalTexture(const _tchar* pFilePath, TEXTURETYPE eTextureType);
	HRESULT						Add_DecalData(const DECAL_DESC& Decal);
	ID3D11ShaderResourceView*	Get_DecalSRV(TEXTURETYPE eTextureType);
	

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	CGameInstance*				m_pGameInstance = { nullptr };
	list<VTXINSTANCE_DECAL>		m_DecalDatas;
	_uint						m_iNumDecals = {};

	CTexture*					m_pDecalTexture[ENUM_CLASS(TEXTURETYPE::END)] = { nullptr };
	CVIBuffer_Decal*			m_pVIBuffer_Decal = { nullptr };

private:
	HRESULT						Bind_Resources(CShader* pShader);

public:
	static CDecal*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free() override;
};

NS_END