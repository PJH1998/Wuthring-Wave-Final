#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CRadialBlur final : public CBlur
{
private:
	CRadialBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContrext);
	virtual ~CRadialBlur() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};
	
	_float2				vCenterUV = {};


public:
	static CRadialBlur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void		Free() override;
};

NS_END