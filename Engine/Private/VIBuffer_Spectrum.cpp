#include "EnginePch.h"
#include "VIBuffer_Spectrum.h"

CVIBuffer_Spectrum::CVIBuffer_Spectrum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Spectrum::CVIBuffer_Spectrum(const CVIBuffer_Spectrum& Prototype)
	:CVIBuffer { Prototype }
{
}

HRESULT CVIBuffer_Spectrum::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer_Spectrum::Initialize_Clone(void* pArg)
{
	return S_OK;
}
