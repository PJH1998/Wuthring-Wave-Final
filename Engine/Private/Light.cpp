#include "EnginePch.h"
#include "Light.h"

#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "GameInstance.h"

CLight::CLight()
	: m_pGameInstance {CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc)
{
    memcpy(&m_LightDesc, &LightDesc, sizeof(LIGHT_DESC));

    return S_OK;
}

HRESULT CLight::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (false == m_isActive)
		return S_OK;

	_uint iPassIndex = {}; 

	if (ENUM_CLASS(LIGHT_DESC::DIRECTION) == m_LightDesc.eType)
	{
		iPassIndex = ENUM_CLASS(SHADER_DEFFERED::DIRECTIONAL);
		if (FAILED(pShader->Bind_Value("g_vLightDirection", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;
	}
	else if(ENUM_CLASS(LIGHT_DESC::POINT) == m_LightDesc.eType)
	{
		if (m_pGameInstance->IsIn_WorldSpace(XMLoadFloat4(&m_LightDesc.vPosition), m_LightDesc.fRange))
		{
			iPassIndex = ENUM_CLASS(SHADER_DEFFERED::POINT);
			if (FAILED(pShader->Bind_Value("g_vLightPosition", &m_LightDesc.vPosition, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(pShader->Bind_Value("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
				return E_FAIL;
		}
		else
			return S_OK;
	}

	if (FAILED(pShader->Bind_Value("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Value("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Value("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	Add_VF_Light();

	return S_OK;
}

HRESULT CLight::Render_EnvMap(CShader* pShader, CVIBuffer_Rect* pVIBuffer, BoundingBox* pBounding)
{
	if (false == m_isActive)
		return S_OK;

	_uint iPassIndex = {};

	if (ENUM_CLASS(LIGHT_DESC::DIRECTION) == m_LightDesc.eType)
	{
		iPassIndex = ENUM_CLASS(SHADER_ENVMAP_DEFFERED::DIRECTIONAL);
		if (FAILED(pShader->Bind_Value("g_vLightDirection", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;
	}
	else if (ENUM_CLASS(LIGHT_DESC::POINT) == m_LightDesc.eType)
	{
		BoundingSphere LightSphere = BoundingSphere(_float3(m_LightDesc.vPosition.x, m_LightDesc.vPosition.y, m_LightDesc.vPosition.z), m_LightDesc.fRange);

		if (pBounding->Intersects(LightSphere))
		{
			iPassIndex = ENUM_CLASS(SHADER_ENVMAP_DEFFERED::POINT);
			if (FAILED(pShader->Bind_Value("g_vLightPosition", &m_LightDesc.vPosition, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(pShader->Bind_Value("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
				return E_FAIL;
		}
		else
			return S_OK;
	}

	if (FAILED(pShader->Bind_Value("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Value("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_Value("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	return S_OK;
}

void CLight::Add_VF_Light()
{
	VF_LIGHT LightData = {};
	LightData.iType = ENUM_CLASS(m_LightDesc.eType);
	LightData.vDirection = m_LightDesc.vDirection;
	LightData.vDiffuse = m_LightDesc.vDiffuse;
	LightData.vPosition = m_LightDesc.vPosition;
	LightData.fRange = m_LightDesc.fRange;

	m_pGameInstance->Add_LightData_ToVF(LightData);
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

	Safe_Release(m_pGameInstance);
}
