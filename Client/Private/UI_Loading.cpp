#include "ClientPch.h"
#include "Animator_UI.h"
#include "PlayerStatus.h"

#include "UI_Loading.h"

CUI_Loading::CUI_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Loading::CUI_Loading(const CUI_Loading& Prototype)
	:CCustom_UI(Prototype)
{
}

HRESULT CUI_Loading::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Loading::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);
	m_vecCachedUITransform.resize(1);
	Ready_Components(pArg);
	__super::Ready_Events();

	_wstring strFilePath =
		L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Loading.json";
	Load_ChildObjects(strFilePath);


	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/LoadingTestFadeOut.json",
	};
	Load_Animations(vecAnimFilePaths);


	// 랜덤하게 로딩 이미지 적용
	_uint iNumBG = Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames.size();
	m_iRandomBGIndex = static_cast<_uint>(m_pGameInstance->Rand(0.f, iNumBG - 0.001f));

	_wstring strRandBGName = Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames[m_iRandomBGIndex];
	
	for (auto& strBGName : Find_ChildObject(L"SectorA_BG")->Get_UIDesc().vecChildNames)
		Find_ChildObject(strBGName)->SetActivate(false);
	Find_ChildObject(strRandBGName)->SetActivate(true);

	return S_OK;
}

void CUI_Loading::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);   // Nothing
}

void CUI_Loading::Update(_float fTimeDelta)
{
	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Update(fTimeDelta);            // Update Animator_UI Component
}

void CUI_Loading::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Late_Update(fTimeDelta);       // Add RenderGroup to UI
}

void CUI_Loading::Render()
{

	//__super::Render();
	//for (auto& child : m_vecChildObjects)
	//	child->Render();
	static _uint i = 0;
	i++;
	cout << "[CUI_Loading::Render] Render Called! : " << i << endl;
}

HRESULT CUI_Loading::Ready_Components(void* pArg)
{
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
	for (auto& child : m_vecChildObjects)
		Safe_Release(child);

	__super::Free();
}
