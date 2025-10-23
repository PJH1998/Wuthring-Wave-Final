#include "EnginePch.h"
#include "Light.h"

#include "Shader.h"
#include "VIBuffer_Rect.h"

CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc)
{
    memcpy(&m_LightDesc, &LightDesc, sizeof(LIGHT_DESC));

    return S_OK;
}

HRESULT CLight::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (FAILED(pShader->Bind_Value("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Value("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Value("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	_uint iPassIndex = {};

	if (ENUM_CLASS(LIGHT_DESC::DIRECTION) == m_LightDesc.eType)
	{
		iPassIndex = ENUM_CLASS(SHADER_DEFFERED::DIRECTIONAL);
		if (FAILED(pShader->Bind_Value("g_vLightDirection", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;
	}
	else if(ENUM_CLASS(LIGHT_DESC::POINT) == m_LightDesc.eType)
	{
		iPassIndex = ENUM_CLASS(SHADER_DEFFERED::POINT);
	}

	pShader->Begin(iPassIndex);
	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	return S_OK;
}

CLight* CLight::Create(const LIGHT_DESC& LightDesc)
{
    CLight* pInstance = new CLight();

    if (FAILED(pInstance->Initialize(LightDesc)))
    {
        MSG_BOX("Failed to Create : Light");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLight::Free()
{
    __super::Free();
}
