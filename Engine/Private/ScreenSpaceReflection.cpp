#include "EnginePch.h"
#include "ScreenSpaceReflection.h"

CScreenSpaceReflection::CScreenSpaceReflection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSFX { pDevice, pContext }
{
}

HRESULT CScreenSpaceReflection::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	return S_OK;
}

void CScreenSpaceReflection::Update(_float fTimeDelta)
{
}

HRESULT CScreenSpaceReflection::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{


	return S_OK;
}

CScreenSpaceReflection* CScreenSpaceReflection::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CScreenSpaceReflection* pInstance = new CScreenSpaceReflection(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CScreenSpaceReflection");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CScreenSpaceReflection::Free()
{
	__super::Free();
}
