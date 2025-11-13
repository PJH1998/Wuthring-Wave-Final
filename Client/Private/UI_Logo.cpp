#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_Logo.h"

CUI_Logo::CUI_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Logo::CUI_Logo(const CUI_Logo& Prototype)
	:CCustom_UI(Prototype)
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


	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_InitialShow.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Logo_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);


	// 최초 켜고 꺼짐 설정
	//for (auto& strBGName : Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames)
	//	Find_ChildObject(strBGName)->SetActivate(false);
	//Find_ChildObject(strRandBGName)->SetActivate(true);



	CCustom_UI* pMainLogoUI = Find_ChildObject(L"SectorA_Main");
	CAnimator_UI* pAnimator_MainLogo = static_cast<CAnimator_UI*>(pMainLogoUI->Get_Component(L"Com_Animator_UI"));
	pAnimator_MainLogo->Change_Animation(L"Logo_Initialize");

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

	Update_CombinedMatrix();
	Update_CombinedDesc();
}

void CUI_Logo::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

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

void CUI_Logo::Update_AnimControl(_float fTimeDelta)
{
	// 1초 경과 시 최초 애니메이션을 재생.

	CCustom_UI* pMainLogoUI = Find_ChildObject(L"SectorA_Main");
	CAnimator_UI* pAnimator_MainLogo = static_cast<CAnimator_UI*>(pMainLogoUI->Get_Component(L"Com_Animator_UI"));

	if		(m_fTimeElapsed >= 1.f &&
			m_iAnimOrder == 0)
	{
		pAnimator_MainLogo->Change_Animation(L"Logo_InitialShow");
		m_iAnimOrder = 1;
	}


	m_fTimeElapsed += fTimeDelta;
}

HRESULT CUI_Logo::Ready_Components(void* pArg)
{
	return S_OK;
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
	for (auto& child : m_vecChildObjects)
		Safe_Release(child);

	__super::Free();
}
