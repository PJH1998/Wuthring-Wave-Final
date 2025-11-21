// Single LockOn 

#include "ClientPch.h"
#include "UI_LockOn.h"

#include "GameSystem.h"
#include "Animator_UI.h"

#define KSTA_UITEST_LOCKON_TOZERO


// 락온UI 생성 자체는 그냥  데미지나 상호작용 만들듯이 만들고 (pooling으로 관리), 
// 보이는 위치를 항시 전달받아온 포인터의 좌표를 기반으로 (transformCom을 받아오든 등) 셰이더에 계속 갱신 
// 
// LockOnUI_Attach
// LockOnUI_Detach 와 같이 게임시스템에 제작

CUI_LockOn::CUI_LockOn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_LockOn::CUI_LockOn(const CUI_LockOn& Prototype)
	: CUI_Image(Prototype)
	//, m_pGameSystem (CGameSystem::GetInstance())
{
	//Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_LockOn::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_LockOn::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_LockOn.json";
	Load_ChildObjects(strFilePath);

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/LockOn_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/LockOn_Show.json",
	};
	Load_Animations(vecAnimFilePaths);

	CCustom_UI* pLockOnUI = Find_ChildObject(L"SectorA_LockOn");
	static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Initialize");

	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_LockOn", this);


	return S_OK;
}

void CUI_LockOn::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

#ifndef KSTA_UITEST_LOCKON_TOZERO
	if (!m_pTargetPos)
		return;
#endif // KSTA_UITEST_LOCKON_TOZERO

	__super::Priority_Update(fTimeDelta);
}

void CUI_LockOn::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

#ifndef KSTA_UITEST_LOCKON_TOZERO
	if (!m_pTargetPos)
		return;
#endif // KSTA_UITEST_LOCKON_TOZERO

	// 타겟 위치 반영
	CCustom_UI* pLockOnUI = Find_ChildObject(L"SectorA_LockOn");


	_matrix matCamView = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
	_matrix matCamProj = m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ);

	const _float2 vScreenSize = { g_iWinSizeX, g_iWinSizeY };
	_vector vTargetWorldPos = (m_pTargetPos) ?
		XMVectorSetW(XMLoadFloat3(m_pTargetPos), 1.f) : _vector();
#ifdef KSTA_UITEST_LOCKON_TOZERO
	vTargetWorldPos = XMVectorSet(0.f, -10.f, 0.f, 1.f);
#endif // KSTA_UITEST_LOCKON_TOZERO


	_matrix matViewProj = matCamView * matCamProj;
	_vector vTargetClipRaw = XMVector3Transform(vTargetWorldPos, matViewProj);

	_float fTargetW = XMVectorGetW(vTargetClipRaw);
	_bool isBehindCamera = (fTargetW <= 0.0f);

	_float2 vScreenPos = {};

	if (!isBehindCamera)
	{
		_vector vTargetNDC = XMVector3TransformCoord(vTargetWorldPos, matViewProj);

		vScreenPos.x = (XMVectorGetX(vTargetNDC) + 1.0f) * 0.5f * vScreenSize.x - vScreenSize.x * 0.5f;
		vScreenPos.y = (1.0f - XMVectorGetY(vTargetNDC)) * 0.5f * vScreenSize.y - vScreenSize.y * 0.5f;
	}
	else
		vScreenPos = { -2000.f, -2000.f }; // 카메라 뒤면 밖으로 쫒아냄

#ifndef KSTA_UITEST_LOCKON_TOZERO
	if (!m_pTargetTransform)
		vScreenPos = { -2000.f, -2000.f }; // 타겟 없으면 밖으로 쫒아냄
#endif
	
	_vector vPos = XMVectorSet(vScreenPos.x, -vScreenPos.y, 0.f, 1.f);
	static_cast<CTransform*>(pLockOnUI->Get_Component(L"Com_Transform"))->Set_State(STATE::POSITION, vPos);


	__super::Update(fTimeDelta);
}

void CUI_LockOn::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

#ifndef KSTA_UITEST_LOCKON_TOZERO
	if (!m_pTargetPos)
		return;
#endif // KSTA_UITEST_LOCKON_TOZERO

	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_LockOn::Render()
{
	if (!m_isActivate)
		return;

#ifndef KSTA_UITEST_LOCKON_TOZERO
	if (!m_pTargetPos)
		return;
#endif // KSTA_UITEST_LOCKON_TOZERO

}

void CUI_LockOn::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// 최초에는 안보이게 가리기

	CCustom_UI* pLockOnUI = Find_ChildObject(L"SectorA_LockOn");
	static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Show", true);

	if (pArg != nullptr)
		m_pTargetPos = static_cast<UI_LOCKON_DESC*>(pArg)->pTargetPos;
	
	m_isActivate = true;
}

void CUI_LockOn::Ready_Presets()
{

}

CUI_LockOn* CUI_LockOn::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_LockOn* pInstance = new CUI_LockOn(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_LockOn");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_LockOn::Clone(void* pArg)
{
	CUI_LockOn* pInstance = new CUI_LockOn(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_LockOn");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_LockOn::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_LockOn");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
