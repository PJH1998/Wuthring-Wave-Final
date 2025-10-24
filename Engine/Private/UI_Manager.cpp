#include "EnginePch.h"

#include "UI_Manager.h"
#include "UIObject.h"

CUI_Manager::CUI_Manager()
{

}

HRESULT CUI_Manager::Initialize()
{
	return S_OK;
}

void CUI_Manager::Update(_float fTimeDelta)
{
	for (auto& rootUI : m_vecRootUIs)
	{
		//rootUI->Check_Rect();
	}


	// 근데 그래서 여기서 뭘 돌릴거임?

	// Update는 오브젝트매니저에서돌리는데
}

HRESULT CUI_Manager::Add_RootUI(CUIObject* rootUI)
{
	for (auto& storedRootUI : m_vecRootUIs)
	{
		if (rootUI == storedRootUI)
			return E_FAIL;
	}

	m_vecRootUIs.push_back(rootUI);

	return S_OK;
}

void CUI_Manager::Clear_RootUI()
{
	m_vecRootUIs.clear();
}

CUI_Manager* CUI_Manager::Create()
{
	CUI_Manager* pInstance = new CUI_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : UI_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Manager::Free()
{
	__super::Free();
}
