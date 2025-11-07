#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CHZB final : public CBase
{
private:
	explicit CHZB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CHZB() = default;

public:
	ID3D11ShaderResourceView* Get_Resource() { return m_pMMSRV; }

public:
	HRESULT					Initialize(_uint iWinSizeX, _uint iWinSizeY);
	void						Update();

#ifdef _DEBUG
	void						Render();
#endif
private:
	ID3D11Device*										m_pDevice = { nullptr };
	ID3D11DeviceContext*							m_pContext = { nullptr };
	class CGameInstance*							m_pGameInstance = { nullptr };
	class CComputeShader*							m_pComputeShader = { nullptr };

	ID3D11UnorderedAccessView*					m_pUAV[MAX_MIPLEVEL] = { nullptr };
	ID3D11ShaderResourceView*					m_pSRV[MAX_MIPLEVEL] = { nullptr };

	ID3D11ShaderResourceView*					m_pMMSRV = { nullptr };

	_uint													m_iWinSizeX = {};
	_uint													m_iWinSizeY = {};

public:
	static		CHZB*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual		void		Free() override;
};

NS_END