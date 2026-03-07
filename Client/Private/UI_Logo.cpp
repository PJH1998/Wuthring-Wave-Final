#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_Logo.h"
#include "UI_Text.h"

#include "GameSystem.h"

CUI_Logo::CUI_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Logo::CUI_Logo(const CUI_Logo& Prototype)
	:CCustom_UI(Prototype)
	, m_pGameSystem( CGameSystem::GetInstance() )
{
}

HRESULT CUI_Logo::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Logo::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);
#ifdef KSTA_ON_TRANSFORM_CACHING
	m_vecCachedUITransform.resize(1);
#endif // KSTA_ON_TRANSFORM_CACHING

	Ready_Components(pArg);
	__super::Ready_Events();

	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Logo.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_InitialShow.json",		// [SectorA_Main]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_Initialize.json",		// [SectorA_Main]

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_LT_Initialize.json",	// [SectorLT_SmallLogo]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_LT_FadeIn.json",		// [SectorLT_SmallLogo]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_LT_FadeOut.json",		// [SectorLT_SmallLogo]

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_B_Initialize.json",	// [SectorB_Button]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_B_FadeIn.json",		// [SectorB_Button]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_B_FadeOut.json",		// [SectorB_Button]

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_AB_Initialize.json",	// [SectorA_MainBack]
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_AB_FadeIn.json",		// [SectorA_MainBack]
	};
	Load_Animations(vecAnimFilePaths);


	// 최초 켜고 꺼짐 설정
	//for (auto& strBGName : Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames)
	//	Find_ChildObject(strBGName)->SetActivate(false);
	//Find_ChildObject(strRandBGName)->SetActivate(true);

	Create_ChildText();


	CAnimator_UI* pAnimator_MainLogo = static_cast<CAnimator_UI*>(m_pUI_MainLogo->Get_Component(L"Com_Animator_UI"));
	pAnimator_MainLogo->Change_Animation(L"Logo_Initialize");
	CAnimator_UI* pAnimator_SmallLogo = static_cast<CAnimator_UI*>(m_pUI_SmallLogo->Get_Component(L"Com_Animator_UI"));
	pAnimator_SmallLogo->Change_Animation(L"Logo_LT_Initialize");
	CAnimator_UI* pAnimator_ButtonLogo = static_cast<CAnimator_UI*>(m_pUI_ButtonLogo->Get_Component(L"Com_Animator_UI"));
	pAnimator_ButtonLogo->Change_Animation(L"Logo_B_Initialize");
	CAnimator_UI* pAnimator_BackLogo = static_cast<CAnimator_UI*>(m_pUI_BackLogo->Get_Component(L"Com_Animator_UI"));
	pAnimator_BackLogo->Change_Animation(L"Logo_AB_Initialize");

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Logo", this);

	return S_OK;
}

void CUI_Logo::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Logo::Update(_float fTimeDelta)
{
	// 애니메이션 변화가 필요 시 Update보다 먼저.
	Update_AnimControl(fTimeDelta);

	__super::Update(fTimeDelta);            // Update Animator_UI Component

}

void CUI_Logo::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Logo::Render()
{

	//__super::Render();
	//for (auto& child : m_vecChildObjects)
	//	child->Render();

	//static _uint i = 0;
	//i++;
	//cout << "[CUI_Logo::Render] Render Called! : " << i << endl;
}

void CUI_Logo::PreAssign_ChildUIs()
{
	m_pUI_MainLogo		= Find_ChildObject(L"SectorA_Main");
	m_pUI_SmallLogo		= Find_ChildObject(L"SectorLT_SmallLogo");
	m_pUI_ButtonLogo	= Find_ChildObject(L"SectorB_Button");
	m_pUI_BackLogo		= Find_ChildObject(L"SectorA_MainBack");
}

void CUI_Logo::Update_AnimControl(_float fTimeDelta)
{
	// 1초 경과 시 최초 애니메이션을 재생.
	CAnimator_UI* pAnimator_MainLogo = static_cast<CAnimator_UI*>(m_pUI_MainLogo->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnimator_SmallLogo = static_cast<CAnimator_UI*>(m_pUI_SmallLogo->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnimator_ButtonLogo = static_cast<CAnimator_UI*>(m_pUI_ButtonLogo->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnimator_BackLogo = static_cast<CAnimator_UI*>(m_pUI_BackLogo->Get_Component(L"Com_Animator_UI"));

	if		(m_fTimeElapsed >= 1.f &&
			m_iAnimOrder == 0)
	{
		pAnimator_MainLogo->Change_Animation(L"Logo_InitialShow");
		m_iAnimOrder = 1;
	}
	else if	(m_fTimeElapsed >= 7.f &&
			m_iAnimOrder == 1)
	{
		pAnimator_SmallLogo	->Change_Animation(L"Logo_LT_FadeIn");
		pAnimator_ButtonLogo->Change_Animation(L"Logo_B_FadeIn");
		pAnimator_BackLogo->Change_Animation(L"Logo_AB_FadeIn");
		m_iAnimOrder = 2;
	}


	m_fTimeElapsed += fTimeDelta;
}

HRESULT CUI_Logo::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_Logo::Create_ChildText()
{
	_wstring strText = L"솔라리스 연결";


	CUI_Text* pFont = m_pGameSystem->Create_FontToScreen_Alpha(
		_float2{ g_iWinSizeX / 2.f, g_iWinSizeY / 2.f + 450.f - 28.f },
		strText,	// 상호작용 글씨
		TEXT_COLOR_TYPE::TT_NORMAL,
		
		0.35f,
		L"UI_Text_Interact"
	);

	CCustom_UI* pAttacher = m_pUI_ButtonLogo;
	auto& fontDesc = pFont->Get_UIDesc();
	auto& attacherDesc = pAttacher->Get_UIDesc(); // 사본 가져오기

	attacherDesc.vecChildNames.push_back(fontDesc.strUIName);
	//pAttacher->Set_UIDesc(attacherDesc); // 변경된 Desc 설정 (필요한 경우)
	pAttacher->Add_Child(pFont);

	for (auto& inst : fontDesc.vecInstanceDescs)
		inst.matExtraData.Text.fAlpha = 1.f;

	fontDesc.strParentName = pAttacher->Get_UIDesc().strUIName;
	fontDesc.pParentObject = pAttacher;

	//pFont->Set_UIDesc(fontDesc);
	pFont->Update_Description(0.f);

	pFont->Update_Alignment(TEXT_ALIGN_TYPE::CENTER);
}

CUI_Logo* CUI_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Logo* pInstance = new CUI_Logo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Logo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Logo::Clone(void* pArg)
{
	CUI_Logo* pInstance = new CUI_Logo(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Created : CUI_Logo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Logo::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Logo");

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);

	__super::Free();
}
