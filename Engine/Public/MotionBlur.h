#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CShader;
class CVIBuffer_Rect;

class CMotionBlur final : public CBlur
{
private:
	typedef struct tagMotionBlurData {
		_float fLimitVelocity;
		_float fLimitDepth;
		_float fLengthScale;
		_float PaddingMotion;
	}MOTION_BLUR_DATA;

private:
	CMotionBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMotionBlur() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

#ifdef _DEBUG
	void				Set_Motion(_float fLimitVelocity, _float fLimitDepth, _float fLengthScale) { m_fLimitVelocity = fLimitVelocity, m_fLimitDepth = fLimitDepth, m_fLengthScale = fLengthScale; }
#endif

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};
	_uint				m_fWinSizeX = {};
	_uint				m_fWinSizeY = {};
	
	_float				m_fLimitVelocity = {};
	_float				m_fLimitDepth = {};
	_float				m_fLengthScale = {};

	ID3D11SamplerState* m_pDefaultSampler = { nullptr };

public:
	static CMotionBlur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END