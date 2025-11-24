#include "ClientPch.h"
#include "Animator_UI.h"

#include "UI_Ovfl_Palette.h"
#include "UI_Text.h"
#include "GameSystem.h"

#define	 IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만
#define	 FLOAT2_LENGTH(x)								(XMVectorGetX(XMVector2Length(XMLoadFloat2(x))))


CUI_Ovfl_Palette::CUI_Ovfl_Palette(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCustom_UI(pDevice, pContext)
{
}

CUI_Ovfl_Palette::CUI_Ovfl_Palette(const CUI_Ovfl_Palette& Prototype)
	: CCustom_UI(Prototype)
	, m_pGameSystem(CGameSystem::GetInstance())
{
}

HRESULT CUI_Ovfl_Palette::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Ovfl_Palette::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	PreAssign_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/UIName.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();
	//Create_ChildText_subnames..();			// 필요하면 추가 후 쓰는거로.



	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		// UI Anims..
	};
	Load_Animations(vecAnimFilePaths);

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Ovfl_Palette", this);


	return S_OK;
}

void CUI_Ovfl_Palette::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_Ovfl_Palette::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Update(fTimeDelta);
}

void CUI_Ovfl_Palette::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_Ovfl_Palette::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_Ovfl_Palette::Reset(const _fmatrix& WorldMatrix, void* pArg)
{



	m_IsGoinDisabled = false;
	m_fDisableTimer = 0.f;
	m_iAnimOrder = 0;
	m_isActivate = true;
}

HRESULT CUI_Ovfl_Palette::Ready_Components(void* pArg)
{
	return S_OK;
}

void CUI_Ovfl_Palette::PreAssign_Presets()
{


}

void CUI_Ovfl_Palette::PreAssign_ChildUIs()
{


}

void CUI_Ovfl_Palette::Update_PalettesInfo()
{
	for (auto& horizonPalettes : m_arrPalettesInfo)
		for (auto& singlePalette : horizonPalettes)
		{
			
		}



}

CUI_Ovfl_Palette* CUI_Ovfl_Palette::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Ovfl_Palette* pInstance = new CUI_Ovfl_Palette(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Ovfl_Palette");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Ovfl_Palette::Clone(void* pArg)
{
	CUI_Ovfl_Palette* pInstance = new CUI_Ovfl_Palette(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Ovfl_Palette");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Ovfl_Palette::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Ovfl_Palette");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
