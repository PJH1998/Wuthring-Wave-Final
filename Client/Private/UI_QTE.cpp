
#include "ClientPch.h"
#include "UI_QTE.h"

#include "Animator_UI.h"
#include "GameSystem.h"

CUI_QTE::CUI_QTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_QTE::CUI_QTE(const CUI_QTE& Prototype)
	: CUI_Image(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}


HRESULT CUI_QTE::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_QTE::Initialize_Clone(void* pArg)
{
	__super::Initialize_Clone(pArg);

	CGameObject::Initialize_Clone(pArg);
	
	Ready_Components(pArg);
	__super::Ready_Events();
	
	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_QTE1.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();
	
	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FadeIn.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1_FadeOut.json",			

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_BG_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_BG_Start.json",			

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_FG_Start.json",			

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_KeyGuide_Initialize.json",			
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/QTE1A_KeyGuide_FadeIn.json",				
	};
	Load_Animations(vecAnimFilePaths);
	
	Reset(_fmatrix(), nullptr);
	m_isActivate = false;
	
	//Create_ChildText();
	//m_pGameInstance->Add_RootUI(L"UI_QTE", this);

	m_isClone = true;

	return S_OK;
}

void CUI_QTE::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_QTE::Update(_float fTimeDelta)
{


	__super::Update(fTimeDelta);            // Update Animator_UI Component
}


void CUI_QTE::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}


void CUI_QTE::Render()
{
	if (!m_isActivate)
		return;

	__super::Render();
}

void CUI_QTE::Reset(const _fmatrix& WorldMatrix, void* pArg)
{


	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0;

	m_isActivate = true;
}

void CUI_QTE::PreAssign_ChildUIs()
{
	//m_pRUI_? = ;
}

CUI_QTE* CUI_QTE::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_QTE* pInstance = new CUI_QTE(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_QTE");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_QTE::Clone(void* pArg)
{
	CUI_QTE* pInstance = new CUI_QTE(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_QTE");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_QTE::Free()
{
	Safe_Release(m_pGameSystem);
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Interact");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
