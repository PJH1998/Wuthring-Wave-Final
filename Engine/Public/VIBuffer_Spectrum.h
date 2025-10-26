#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Spectrum : public CVIBuffer
{
private:
	explicit CVIBuffer_Spectrum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Spectrum(const CVIBuffer_Spectrum& Prototype);
	virtual ~CVIBuffer_Spectrum() = default;

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
};

NS_END
