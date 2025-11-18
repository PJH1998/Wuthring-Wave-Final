#include "ClientPch.h"

#include "UI_MobHPBar.h"
#include "Animator_UI.h"



CUI_MobHPBar::CUI_MobHPBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_MobHPBar::CUI_MobHPBar(const CUI_MobHPBar& Prototype)
	: CUI_Image(Prototype)
{
}

HRESULT CUI_MobHPBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_MobHPBar::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/?";
	Load_ChildObjects(strFilePath);

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		//L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Main_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);
	 

	//CCustom_UI* pLockOnUI = Find_ChildObject(L"SectorA_LockOn");
	//static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	//static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Initialize");


	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_MobHPBar", this);

	return S_OK;
}

void CUI_MobHPBar::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_MobHPBar::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	//Update_ApplyTargetPos(pSectorA, vTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.



	__super::Update(fTimeDelta);
}

void CUI_MobHPBar::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


	//Update_CamDistScale(pSectorAMain, 40.f);


	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_MobHPBar::Render()
{
	if (!m_isActivate)
		return;

}

void CUI_MobHPBar::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// 애니메이션 존재한다면 초기화

	m_isActivate = true;
}

void CUI_MobHPBar::Ready_Presets()
{

}

void CUI_MobHPBar::Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos)
{


}

void CUI_MobHPBar::Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance)
{


}

CUI_MobHPBar* CUI_MobHPBar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_MobHPBar* pInstance = new CUI_MobHPBar(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_MobHPBar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_MobHPBar::Clone(void* pArg)
{
	CUI_MobHPBar* pInstance = new CUI_MobHPBar(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_MobHPBar");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_MobHPBar::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_MobHPBar");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
