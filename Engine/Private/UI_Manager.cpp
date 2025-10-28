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
	for (auto& rootUI : m_RootUIs)
	{
		//rootUI.second->Update();
	}

	// 근데 그래서 여기서 뭘 돌릴거임?

	// Update는 오브젝트매니저에서돌리는데
}

HRESULT CUI_Manager::SetActive_UI(const _wstring& strName_UI, _bool isActive)
{
	CUIObject* pUIObject = Find_UIObject(strName_UI);
	if (!pUIObject)
		return E_FAIL;

	pUIObject->Set_Active(isActive);
	return S_OK;
}

//_bool CUI_Manager::Check_UIEvent_Triggered(const _wstring& strName_UI, _uint iCheckEventType)
//{
//	// 이건 엔진단계에서 안될 듯.
//	// 
//	// 차라리 Find_UIObject 로 가져온 뒤, CCustom_UI* 로 캐스팅해서,
//	// Check_OnInteract 함수 불러오게 하는 편이?
//
//	// 그럼 아래와 같은 꼴..
//	// 
//	// 
//	// 
//	// - 외부에서 UI의 상태를 확인하기 (막 클릭됐는지, 호버 감지중인지 등)
//	// 
//	// UI_UHD : rootUI의 태그 (레벨 시작 시 삽입)
//	// SectorB_Status : 자식중 해당 문자열을 m_tUIDesc.strUIName 으로 가지는 것이 있는지를 찾음
//	// Check_OnInteract(A, B) : A 는 이벤트 타입, B(선택) 은 인스턴스 UI라면 몇번째 인덱스의 인스턴스를 가리키는지.
//	// 
//	// dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_UHD"))->Find_ChildObject(L"SectorB_Status")->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), 0);
//	// dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_UHD"))->Check_OnInteract(L"SectorB_Status", ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), 0);
//	//
//	//
//	// - 외부에서 UI에게 상태 보내기
//	// dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_UHD"))->Find_ChildObject(L"SectorB_Status")->OnEvent(UI_EVENT_TYPE::CLICK_ENTER);
//	// 
//
//	return _bool();
//}

CUIObject* CUI_Manager::Find_UIObject(const _wstring& strName_UI)
{
	auto iter = m_RootUIs.find(strName_UI);
	if (iter != m_RootUIs.end())
		return iter->second;

	return nullptr;
}

HRESULT CUI_Manager::Add_RootUI(const _wstring& strName_UI, CUIObject* pRootUI)
{
	if (pRootUI == nullptr)
		return E_FAIL;

	if (m_RootUIs.find(strName_UI) != m_RootUIs.end())
		return E_FAIL;

	m_RootUIs.emplace(strName_UI, pRootUI);
	return S_OK;
}

void CUI_Manager::Clear_RootUI()
{
	m_RootUIs.clear();
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
