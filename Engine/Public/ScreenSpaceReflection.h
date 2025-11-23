#pragma once
#include "SFX.h"

NS_BEGIN(Engine)

class CScreenSpaceReflection final : public CSFX
{
private:
	CScreenSpaceReflection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CScreenSpaceReflection() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

#ifdef _DEBUG
public:
	void					Set_SSR(_float fMinStep, _float fMaxStep, _float fStartOffset)
	{
		m_fMinStepSize = fMinStep, m_fMaxStepSize = fMaxStep, m_fStartOffset = fStartOffset;
	}
#endif

private:
	_float				m_fMinStepSize = {};
	_float				m_fMaxStepSize = {};
	_float				m_fStartOffset = {};
	
	//	//SSR
	//	uint  g_iStep;
	//float g_fMinStepSize;
	//float g_fMaxStepSize;
	//float g_fMaxDistance;
	//float g_fStartOffset;

public:
	static CScreenSpaceReflection*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void					Free();
};

NS_END