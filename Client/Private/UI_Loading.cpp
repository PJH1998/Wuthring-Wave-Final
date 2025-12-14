#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_Loading.h"

#include "GameSystem.h"
#include "UI_Text.h"

CUI_Loading::CUI_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Loading::CUI_Loading(const CUI_Loading& Prototype)
	:CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_Loading::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Loading::Initialize_Clone(void* pArg)
{
	LEVEL eDestLevel = static_cast<UI_LOADING_DESC*>(pArg)->eDestLevel;

	CGameObject::Initialize_Clone(pArg);
#ifdef KSTA_ON_TRANSFORM_CACHING
	m_vecCachedUITransform.resize(1);
#endif // KSTA_ON_TRANSFORM_CACHING

	Ready_Components(pArg);
	__super::Ready_Events();

	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Loading.json";
	Load_ChildObjects(strFilePath);


	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/LoadingTestFadeOut.json",
	};
	Load_Animations(vecAnimFilePaths);


	// 목적지 레벨에 맞게 로딩 이미지 인덱스 지정
	switch (eDestLevel)
	{
	case Client::LEVEL::LOGO:
		m_strBGName = L"BG_Login04";
		break;
	case Client::LEVEL::GAMEPLAY:
	{
		const _uint iNumBG = 3;
		_uint iRandIndex = static_cast<_uint>(m_pGameInstance->Rand(0.f, iNumBG - 0.001f));
		switch (iRandIndex) {
		case 0:		m_strBGName = L"Bg_Lianxita40";		break;
		case 1:		m_strBGName = L"Bg_Lianxita41";		break;
		case 2:		m_strBGName = L"Bg_Lianxita36";		break;
		}
	}
		break;
	case Client::LEVEL::HEAVEN:
		m_strBGName = L"Bg_Loading09";
		break;
	case Client::LEVEL::TEST:
		m_strBGName = L"BG_Login04";
		break;
	}
	
	for (auto& strBGName : Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames)
		Find_ChildObject(strBGName)->SetActivate(false);
	Find_ChildObject(m_strBGName)->SetActivate(true);


	// 이미지에 따른 텍스트 변경
	Ready_Texts();

	return S_OK;
}

void CUI_Loading::Priority_Update(_float fTimeDelta)
{	
	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Loading::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Loading::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Loading::Render()
{
	//__super::Render();
	//for (auto& child : m_vecChildObjects)s
	//	child->Render();
	
	//static _uint i = 0;
	//i++;
	//cout << "[CUI_Loading::Render] Render Called! : " << i << endl;
}

HRESULT CUI_Loading::Ready_Components(void* pArg)
{
	return S_OK;
}

HRESULT CUI_Loading::Ready_Texts()
{
	_uint iDestLevel = ENUM_CLASS(LEVEL::LOADING);

	_wstring strTitleText = {};
	_wstring strDescriptionText = {};

	_wstring strBGName = m_strBGName;

	if		(strBGName == L"Bg_Lianxita40")
	{
		strTitleText = L"잠꼬대 마을";
		strDescriptionText = L"흑조 구름에 삼켜진 마을. 속삭이는 잠꼬대가 이 곳을 맴돌면서, 명식 레비아탄의 음모를 널리 퍼트린다. \n일곱 언덕 사람들은 「잠꼬대 마을」이라는 이름으로 대신 지칭하며, 과거 이 마을의 영광스러운 이름을 잊으려 한다.";
	}

	else if (strBGName == L"Bg_Lianxita41")
	{
		strTitleText = L"세 영웅의 봉우리";
		strDescriptionText = L"상귀스 사냥 평원의 가장 높은 곳에 위치한 산봉우리. \n독특한 산봉우리 구조와 세 영웅의 왕이 방문한 것으로 인해 해당 이름이 붙여졌으며, 검투사들에게 마음 속 성지로 여겨지고 있다. \n전설에 따르면, 모든 영웅의 왕들은 이곳에서 마지막 수행을 마친 후 운명을 이겨내고 영광을 누린다고 한다.";
	}

	else if (strBGName == L"Bg_Lianxita36")
	{
		strTitleText = L"아틸리우스 협곡";
		strDescriptionText = L"단단한 반석으로 이루어진 천연 협곡. 돌기둥이 늘어서 있으며 가파르고 협준하다. \n상귀스 사냥 평원 내부로 들어가는 주요 통로로서, 일곱 언덕 사람들이 대대로 영웅의 왕 아틸리우스 석상의 증명 하에 사냥의 여정에 올랐다.";
	}

	else if (strBGName == L"BG_Login04")
	{
		strTitleText = L"153기 - 명조 웨더링 웨이브";
		strDescriptionText = L"로고화면 로딩중입니다.";
	}

	else if (strBGName == L"Bg_Loading09")
	{
		strTitleText = L"순회의 천국";
		strDescriptionText = L"레비아탄의 화신은 끝없는 흑조와 함께, 하나로 융합된 영원의 어둠을 이 세상에 가져올 것이다. \n종말에 맞서, 저 멀리 보이는 희망의 빛을 위해 싸워라";
	}
;
	CUI_Text* pTitleText = m_pGameSystem->Create_FontToScreen(_float2{ 150.f, 880.f }, strTitleText, TEXT_COLOR_TYPE::TT_TITLE, 0.5f, L"UI_Text_TitleTest");
	m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_UI_Text", pTitleText);

	CUI_Text* pDescriptionText = m_pGameSystem->Create_FontToScreen(_float2{ 150.f, 920.f }, strDescriptionText, TEXT_COLOR_TYPE::TT_NORMAL, 0.3f, L"UI_Text_DescriptionTest");
	m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_UI_Text", pDescriptionText);

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Text_LoadingTitle", pTitleText);
	m_pGameInstance->Add_RootUI(L"UI_Text_LoadingDescription", pDescriptionText);

	return S_OK;
}

CUI_Loading* CUI_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Loading* pInstance = new CUI_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Loading::Clone(void* pArg)
{
	CUI_Loading* pInstance = new CUI_Loading(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Created : CUI_Loading");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Loading::Free()
{
	Safe_Release(m_pGameSystem);

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);

	if (m_isClone)
	{
		m_pGameInstance->Remove_RootUI(L"UI_Text_LoadingTitle");
		m_pGameInstance->Remove_RootUI(L"UI_Text_LoadingDescription");
	}

	__super::Free();
}
