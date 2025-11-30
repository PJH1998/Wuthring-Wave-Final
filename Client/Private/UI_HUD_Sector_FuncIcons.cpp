#include "ClientPch.h"
#include "Animator_UI.h"
#include "UI_HUD_Sector_FuncIcons.h"
//#include "GameSystem.h"

CUI_HUD_Sector_FuncIcons::CUI_HUD_Sector_FuncIcons(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_HUD_Sector_FuncIcons::CUI_HUD_Sector_FuncIcons(const CUI_HUD_Sector_FuncIcons& Prototype)
	: CCustom_UI(Prototype)
	//, m_pGameSystem(CGameSystem::GetInstance())
{
}

HRESULT CUI_HUD_Sector_FuncIcons::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_HUD_Sector_FuncIcons::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	PreAssign_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_HUD_Sector_FuncIcons.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		//L"../../Client/Bin/Resource/UI/FJson/UIAnim/TabUtil_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);

	// ksta : fade out in은 필요함
	//static_cast<CAnimator_UI*>(m_pRUI_All->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"TabUtil_Initialize");

	//m_isActivate = false;

	m_isClone = true;
	//m_pGameInstance->Add_RootUI(L"UI_HUD_Sector_FuncIcons", this);

	return S_OK;
}

void CUI_HUD_Sector_FuncIcons::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_HUD_Sector_FuncIcons::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	// implement

	__super::Update(fTimeDelta);
}

void CUI_HUD_Sector_FuncIcons::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_HUD_Sector_FuncIcons::Render()
{
	if (!m_isActivate)
		return;
}

HRESULT CUI_HUD_Sector_FuncIcons::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_HUD_Sector_FuncIcons::PreAssign_ChildUIs()
{
	if (!m_isActivate)
		return;
	
	m_pRUI_All			 = Find_ChildObject(L"Sub_All");

	m_pUI_SectorRT		 = Find_ChildObject(L"SectorRT_FuncIcons");
	m_pUI_RT_InstIcons	 = Find_ChildObject(L"RT_InstIcons");
}

void CUI_HUD_Sector_FuncIcons::PreAssign_Presets()
{
}

CUI_HUD_Sector_FuncIcons* CUI_HUD_Sector_FuncIcons::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_HUD_Sector_FuncIcons* pInstance = new CUI_HUD_Sector_FuncIcons(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_HUD_Sector_FuncIcons");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_HUD_Sector_FuncIcons::Clone(void* pArg)
{
	CUI_HUD_Sector_FuncIcons* pInstance = new CUI_HUD_Sector_FuncIcons(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_HUD_Sector_FuncIcons");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_HUD_Sector_FuncIcons::Free()
{
	//if (m_isClone)
	//	m_pGameInstance->Remove_RootUI(L"UI_HUD_Sector_FuncIcons");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
