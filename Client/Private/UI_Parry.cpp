// Single Parry


#include "ClientPch.h"

#include "UI_Parry.h"
#include "Animator_UI.h"

CUI_Parry::CUI_Parry(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_Parry::CUI_Parry(const CUI_Parry& Prototype)
	: CUI_Image(Prototype)
{
}

HRESULT CUI_Parry::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Parry::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_Parry.json";
	Load_ChildObjects(strFilePath);

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Main_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Main_Play.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Eff_Initialize.json",
		L"../../Client/Bin/Resource/UI/FJson/UIAnim/ParryA_Eff_Play.json",

		L"../../Client/Bin/Resource/UI/FJson/UIAnim/Parry_ApprCirc_Play.json",
	};
	Load_Animations(vecAnimFilePaths);

	//CCustom_UI* pLockOnUI = Find_ChildObject(L"SectorA_LockOn");
	//static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	//static_cast<CAnimator_UI*>(pLockOnUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Initialize");

	CCustom_UI* pCircle_Appr	= Find_ChildObject(L"ParryCircle_Approach");
	CCustom_UI* pSectorAMain	= Find_ChildObject(L"SectorA_Parry");
	CCustom_UI* pSectorAEff		= Find_ChildObject(L"SectorA_ParryEffect");

	CAnimator_UI* pAnim_Circle_Appr		= static_cast<CAnimator_UI*>(pCircle_Appr->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnim_SectorAMain	= static_cast<CAnimator_UI*>(pSectorAMain->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnim_SectorAEff		= static_cast<CAnimator_UI*>(pSectorAEff->Get_Component(L"Com_Animator_UI"));


	// 최초에 비활성화 애니메이션. + 위치 비고정형 UI이기 떄문에 애니메이션에서 위치는 적용 안되도록.
	pAnim_Circle_Appr->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	pAnim_SectorAMain->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	pAnim_SectorAEff->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));

	pAnim_SectorAMain->Change_Animation(L"ParryA_Main_Initialize");
	pAnim_SectorAEff->Change_Animation(L"ParryA_Eff_Initialize");


	//m_vOriginScale = m_pTransformCom->Get_Scaled();
	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_Parry", this);
	//m_vOriginSca = m_pTransformCom->Get_Scaled();

    return S_OK;
}

void CUI_Parry::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


	__super::Priority_Update(fTimeDelta);
}

void CUI_Parry::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	// UI를 타겟 위치로.
	// 단일 UI까지는 CPU로 돌려도 될듯
	CCustom_UI* pCircle_Appr = Find_ChildObject(L"ParryCircle_Approach");
	CCustom_UI* pCircle_Stat = Find_ChildObject(L"ParryCircle_Static");

	CCustom_UI* pSectorA = Find_ChildObject(L"SectorA_Parry");
	CCustom_UI* pSectorAEff = Find_ChildObject(L"SectorA_ParryEffect");


	_float3 vTargetPos = m_vTargetPos; // ksta : 나중에 수정. 받아온 타겟 좌표로.

	Update_ApplyTargetPos(pSectorA, vTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.
	Update_ApplyTargetPos(pSectorAEff, vTargetPos);	// 해당 UI를 타겟 위치로 이동시킴.


	// 시간 갱신
	m_fElapsedTime += fTimeDelta;


	// 시간 경과 시 비활성화.
	const _float fNormalEndTime = 0.50f;
	const _float fParriedEndTime = 0.34f;

	if (!m_isParried &&
		m_fElapsedTime >= fNormalEndTime)
	{
		m_fElapsedTime = 0.f;
		m_isActivate = false;
	}
	
	if (m_isParried &&
		m_fElapsedTime >= fParriedEndTime)
	{
		m_isParried = false;
		m_fElapsedTime = 0.f;
		m_isActivate = false;
	}

	__super::Update(fTimeDelta);
}

void CUI_Parry::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;


	CCustom_UI* pSectorAMain = Find_ChildObject(L"SectorA_Parry");
	CCustom_UI* pSectorAEff = Find_ChildObject(L"SectorA_ParryEffect");
	Update_CamDistScale(pSectorAMain, 40.f);
	Update_CamDistScale(pSectorAEff, 40.f);

	//Update_CamDistScale(this, 40.f);



	Update_CombinedMatrix();
	Update_CombinedDesc();

	__super::Late_Update(fTimeDelta);
}

void CUI_Parry::Render()
{
	if (!m_isActivate)
		return;
}

void CUI_Parry::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// pooling 꺼내질 시 초기화
	CCustom_UI* pCircle_Appr = Find_ChildObject(L"ParryCircle_Approach");
	CCustom_UI* pSectorAMain = Find_ChildObject(L"SectorA_Parry");
	CCustom_UI* pSectorAEff = Find_ChildObject(L"SectorA_ParryEffect");

	CAnimator_UI* pAnim_Circle_Appr = static_cast<CAnimator_UI*>(pCircle_Appr->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnim_pSectorAMain = static_cast<CAnimator_UI*>(pSectorAMain->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnim_pSectorAEff = static_cast<CAnimator_UI*>(pSectorAEff->Get_Component(L"Com_Animator_UI"));

	// 패링용 애니메이션 진행
	pAnim_pSectorAMain->Change_Animation(L"ParryA_Main_Play", true);
	pAnim_Circle_Appr->Change_Animation(L"Parry_ApprCirc_Play", true);
	pAnim_pSectorAEff->Change_Animation(L"ParryA_Eff_Initialize", true);

	m_isActivate = true;
	m_fElapsedTime = 0.f;
	m_isParried = false;
}

void CUI_Parry::Enable_Parried()
{
	CCustom_UI* pCircle_Appr = Find_ChildObject(L"ParryCircle_Approach");
	CCustom_UI* pSectorAMain = Find_ChildObject(L"SectorA_Parry");
	CCustom_UI* pSectorAEff = Find_ChildObject(L"SectorA_ParryEffect");

	CAnimator_UI* pAnim_Circle_Appr = static_cast<CAnimator_UI*>(pCircle_Appr->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnim_pSectorAMain = static_cast<CAnimator_UI*>(pSectorAMain->Get_Component(L"Com_Animator_UI"));
	CAnimator_UI* pAnim_pSectorAEff = static_cast<CAnimator_UI*>(pSectorAEff->Get_Component(L"Com_Animator_UI"));

	// 패링 애니메이션 제거, 이펙트 애니메이션 진행
	pAnim_pSectorAMain->Change_Animation(L"ParryA_Main_Initialize", true);	// Circle_Appr 은 pSectorAMain 의 자식이기에 같이 비활성화됨.
	pAnim_pSectorAEff->Change_Animation(L"ParryA_Eff_Play", true);

	m_isParried = true;
	m_fElapsedTime = 0.f;
}

void CUI_Parry::Ready_Presets()
{

}

void CUI_Parry::Update_ApplyTargetPos(CCustom_UI* pTargetUI, _float3 vTargetPos)
{
	const _matrix matCamView = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
	const _matrix matCamProj = m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ);

	const _float2 vScreenSize = { g_iWinSizeX, g_iWinSizeY };
	_vector vTargetWorldPos = XMVectorSetW(XMLoadFloat3(&vTargetPos), 1.0f);

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

	_vector vPos = XMVectorSet(vScreenPos.x, -vScreenPos.y, 0.f, 1.f);
	static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"))->Set_State(STATE::POSITION, vPos);
}

void CUI_Parry::Update_CamDistScale(CCustom_UI* pTargetUI, _float fPivotDistance)
{
	CTransform* pTargetTransform = static_cast<CTransform*>(pTargetUI->Get_Component(L"Com_Transform"));

	_float3 vScale =/* (pTargetUI == this)? m_vOriginSca :*/ pTargetTransform->Get_Scaled();

	_float3 vTargetPos = m_vTargetPos;
	_float fDist = XMVectorGetX(XMVector3Length(XMLoadFloat4(m_pGameInstance->Get_CamPos()) - XMLoadFloat3(&vTargetPos)));
	_float fScaleMultiple = fPivotDistance / fDist;

	_float3 vFinalScale = _float3{
		vScale.x * fScaleMultiple,
		vScale.y * fScaleMultiple,
		vScale.z * fScaleMultiple
	};

	pTargetTransform->Scale(vFinalScale);
}

CUI_Parry* CUI_Parry::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Parry* pInstance = new CUI_Parry(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Parry");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Parry::Clone(void* pArg)
{
	CUI_Parry* pInstance = new CUI_Parry(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Parry");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_Parry::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_Parry");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}
