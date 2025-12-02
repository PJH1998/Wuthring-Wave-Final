#include"EditorPch.h"
#include "Edit_MapEffectCollector.h"
#include"Level_Map.h"

CEdit_MapEffectCollector::CEdit_MapEffectCollector()
{
}

HRESULT CEdit_MapEffectCollector::Initialize()
{
	return E_NOTIMPL;
}

void CEdit_MapEffectCollector::Set_ImGuiOption()
{
	//ImGui::
	m_EffectInfo[m_iEffectIndex];

	ImGui::SameLine();
	if (ImGui::Button("Get PickPos"))
		m_EffectInfo[m_iEffectIndex].vPos = CLevel_Map::m_vPickedPos;
}

CEdit_MapEffectCollector* CEdit_MapEffectCollector::Create()
{
	CEdit_MapEffectCollector* pInstance = new CEdit_MapEffectCollector();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Edit_MapEffectCollector");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapEffectCollector::Free()
{
	__super::Free();
}
