#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CHZB final : public CBase
{
public:
	enum HZB_CS_TYPE
	{
		MIPMAP,
		OCCLUSION,
		END
	};

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
	class CComputeShader*							m_pComputeShader[HZB_CS_TYPE::END] = {nullptr};

	_uint													m_iWinSizeX = {};
	_uint													m_iWinSizeY = {};

	// Default Setting
	ID3D11UnorderedAccessView*					m_pUAV[MAX_MIPLEVEL] = { nullptr };
	ID3D11ShaderResourceView*					m_pSRV[MAX_MIPLEVEL] = { nullptr };
	ID3D11ShaderResourceView*					m_pMMSRV = { nullptr };

	// Occlusion Culling
	ID3D11Buffer*										m_pBoxPointsBuffer = { nullptr };
	ID3D11Buffer*										m_pOcclusionFlagBuffer = { nullptr };

private:
	void						Ready_DefaultSetting();
	void						Ready_OcclusionCulling();

public:
	static		CHZB*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual		void		Free() override;
};

NS_END