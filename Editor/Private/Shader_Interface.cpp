
#include "EditorPch.h"
#include "Shader_Interface.h"

CShader_Interface::CShader_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext }
{
}

HRESULT CShader_Interface::Initialize()
{
	return S_OK;
}

void CShader_Interface::Update_Shadow()
{
	Set_ShadowBias();
}

void CShader_Interface::Set_ShadowBias()
{
	ImGui::Begin("SHADOW BIAS");

	
	if(ImGui::CollapsingHeader("Base Bias"))
	{
		Setting_Bias("BIAS_CASCADE[0]", &m_fBias[0]);
		Setting_Bias("BIAS_CASCADE[1]", &m_fBias[1]);
		Setting_Bias("BIAS_CASCADE[2]", &m_fBias[2]);
		Setting_Bias("BIAS_CASCADE[3]", &m_fBias[3]);
	}

	if (ImGui::CollapsingHeader("Min Bias"))
	{
		Setting_Bias("MIN_BIAS_CASCADE[0]", &m_fMinBias[0]);
		Setting_Bias("MIN_BIAS_CASCADE[1]", &m_fMinBias[1]);
		Setting_Bias("MIN_BIAS_CASCADE[2]", &m_fMinBias[2]);
		Setting_Bias("MIN_BIAS_CASCADE[3]", &m_fMinBias[3]);
	}

	ImGui::InputFloat("SLOPE_SCALE", &m_fSlopeScale);
	

	m_pGameInstance->Bind_RawValue_Renderer("g_fShadowBais", &m_fBias, sizeof(_float4));
	m_pGameInstance->Bind_RawValue_Renderer("g_fMinShadowBias", &m_fMinBias, sizeof(_float4));
	m_pGameInstance->Bind_RawValue_Renderer("g_DebugSlopeScale", &m_fSlopeScale, sizeof(_float));
	
	ImGui::End();
}

void CShader_Interface::Setting_Bias(const _char* pName, _float* pFloat)
{
	ImGui::PushItemWidth(250.f);
	ImGui::DragFloat(pName, pFloat, 0.001f, 0.001f, 0.5f);

//	ImGui::SliderFloat(pName, pFloat, 0.001f, 0.5f);
	ImGui::PopItemWidth();

	//ImGui::SameLine();

	//ImGui::PushItemWidth(120.f);

	//ImGui::InputFloat("", pFloat, 0.001f, 0.01f);
	//ImGui::PopItemWidth();
}

CShader_Interface* CShader_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShader_Interface* pInstance = new CShader_Interface(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CShader_Interface");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CShader_Interface::Free()
{
	__super::Free();

}
