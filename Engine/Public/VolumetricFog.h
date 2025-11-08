#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CComputeShader;

class CVolumetricFog final : public CBase
{
private:
	typedef struct tagVF_Data{
		_float padding[4];
	}VF_DATA;

private:
	CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVolumetricFog() = default;

public:
	HRESULT						Initialize(_uint iWinSizeX, _uint iWinSizeY);


private:
	_float3						m_vFroxelSize = {};

	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	
	CComputeShader*				m_pCS = { nullptr };

	ID3D11ShaderResourceView*	m_pHZB = { nullptr };
	ID3D11ShaderResourceView*	m_pVF_SRV = { nullptr };
	ID3D11UnorderedAccessView*	m_pVF_UAV = { nullptr };
	ID3D11Buffer*				m_pVF_Buffer = { nullptr };

private:
	HRESULT						Ready_FroxelVolume();
	HRESULT						Ready_ComputeShader();

public:
	static CVolumetricFog*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY);
	virtual void				Free();
};

NS_END