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

	ImGui::CollapsingHeader("Base Bias");
	{
		ImGui::InputFloat("BIAS_CSM[0]", &m_fBias[0]);
		ImGui::InputFloat("BIAS_CSM[1]", &m_fBias[1]);
		ImGui::InputFloat("BIAS_CSM[2]", &m_fBias[2]);
		ImGui::InputFloat("BIAS_CSM[3]", &m_fBias[3]);
	}

	ImGui::InputFloat("SLOPE_SCALE", &m_fSlopeScale);
	

	if(ImGui::Button("Apply"))
	{
		m_pGameInstance->Bind_RawValue_Renderer("g_ShadowBias", &m_fBias, sizeof(_float4));
		m_pGameInstance->Bind_RawValue_Renderer("g_DebugSlopeScale", &m_fSlopeScale, sizeof(_float));
	}

	ImGui::End();
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
