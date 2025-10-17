#include "EnginePch.h"
#include "Light_Manager.h"

#include "Shader.h"

CLight_Manager::CLight_Manager()
{
}

const LIGHT_DESC* CLight_Manager::Get_LightDesc(const _wstring& strLightTag)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter == m_Lights.end())
		return nullptr;

	return iter->second->Get_LightDesc();
}

#ifdef _DEBUG
LIGHT_DESC* CLight_Manager::Get_LightDesc_For_Map(const _wstring& strLightTag)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter == m_Lights.end())
		return nullptr;

	return iter->second->Get_LightDesc_For_Map();
}
#endif

HRESULT CLight_Manager::Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter != m_Lights.end())
	{
		Safe_Release(iter->second);
		m_Lights.erase(iter);
	}

	CLight* pLight = CLight::Create(LightDesc);

	m_Lights.emplace(strLightTag, pLight);

	return S_OK;
}

HRESULT CLight_Manager::SetUp_Light(CShader* pShader, const _wstring& strLightTag, LIGHT_DESC::TYPE eType)
{
	auto iter = m_Lights.find(strLightTag);
	if (iter == m_Lights.end())
		return E_FAIL;

	const LIGHT_DESC* pLight = iter->second->Get_LightDesc();

	if (eType == LIGHT_DESC::DIRECTION)
	{
		pShader->Bind_Value("g_vLightDiffuse", &pLight->vDiffuse, sizeof(_float4));
		pShader->Bind_Value("g_vLightAmbient", &pLight->vAmbient, sizeof(_float4));
		pShader->Bind_Value("g_vLightSpecular", &pLight->vSpecular, sizeof(_float4));
		pShader->Bind_Value("g_vLightDir", &pLight->vDirection, sizeof(_float4));
	}

	return S_OK;
}

HRESULT CLight_Manager::Clear_Light()
{
	for (auto& Pair : m_Lights)
		Safe_Release(Pair.second);
	m_Lights.clear();

	return S_OK;
}

HRESULT CLight_Manager::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	for (auto& Pair : m_Lights)
		Pair.second->Render(pShader, pVIBuffer);

	return S_OK;
}

CLight_Manager* CLight_Manager::Create()
{
	return new CLight_Manager();
}

void CLight_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_Lights)
		Safe_Release(Pair.second);
	m_Lights.clear();
}
