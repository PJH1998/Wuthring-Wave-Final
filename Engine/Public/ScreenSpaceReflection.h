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

public:
	static CScreenSpaceReflection*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void					Free();
};

NS_END