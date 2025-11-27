#include "ClientPch.h"
#include "UI_GrafflePoint.h"
#include "Animator_UI.h"

#define KSTA_UITEST_GRAFFLE_TOZERO


CUI_GrafflePoint::CUI_GrafflePoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUI_Image(pDevice, pContext)
{
}

CUI_GrafflePoint::CUI_GrafflePoint(const CUI_GrafflePoint& Prototype)
	: CUI_Image(Prototype)
	//, m_pGameSystem (CGameSystem::GetInstance())
{
	//Safe_AddRef(m_pGameSystem);
}

HRESULT CUI_GrafflePoint::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_GrafflePoint::Initialize_Clone(void* pArg)
{
	CGameObject::Initialize_Clone(pArg);

	Ready_Components(pArg);
	//__super::Ready_Events();
	Ready_Presets();

	// Load Objects description & Create Objects. from json.  Textures already pre-loaded by Loader.
	_wstring strFilePath = L"../../Client/Bin/Resource/UI/FJson/UITree/Root_GrafflePoint.json";
	Load_ChildObjects(strFilePath);
	PreAssign_ChildUIs();

	// Load Animations from json.
	vector<_wstring> vecAnimFilePaths = {
		//L"../../Client/Bin/Resource/UI/FJson/UIAnim/LockOn_Initialize.json",
	};
	Load_Animations(vecAnimFilePaths);


	static_cast<CAnimator_UI*>(m_pGrafflePointUI->Get_Component(L"Com_Animator_UI"))->Set_DisableFlag(ENUM_CLASS(CAnimator_UI::UI_ANIM_DISABLE::POS));
	static_cast<CAnimator_UI*>(m_pGrafflePointUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Initialize");

	Reset(_fmatrix(), nullptr);
	m_isActivate = false;

	m_isClone = true;
	m_pGameInstance->Add_RootUI(L"UI_GrafflePoint", this);


	return S_OK;
}

void CUI_GrafflePoint::Priority_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	__super::Priority_Update(fTimeDelta);
}

void CUI_GrafflePoint::Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;



	__super::Update(fTimeDelta);
}

void CUI_GrafflePoint::Late_Update(_float fTimeDelta)
{
	if (!m_isActivate)
		return;

	
	Update_CombinedMatrix();
	Update_CombinedDesc();

	Update_Instances();

	__super::Late_Update(fTimeDelta);
}

void CUI_GrafflePoint::Render()
{
	if (!m_isActivate)
		return;

}

void CUI_GrafflePoint::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	// 최초에는 안보이게 가리기
	static_cast<CAnimator_UI*>(m_pGrafflePointUI->Get_Component(L"Com_Animator_UI"))->Change_Animation(L"LockOn_Show", true);

	//if (pArg != nullptr);
	//	m_pTargetPos = static_cast<UI_GRAFFLEINFO_DESC*>(pArg)->pTargetPos;

	m_isActivate = true;
}

void CUI_GrafflePoint::PreAssign_ChildUIs()
{
	m_pGrafflePointUI = Find_ChildObject(L"SectorA_Static");

}

void CUI_GrafflePoint::Ready_Presets()
{
	
}

void CUI_GrafflePoint::Update_Instances()
{
	CCustom_UI* pTargetUI = m_pGrafflePointUI;
	auto targetDesc = pTargetUI->Get_UIDesc();
	auto& targetInstDesc = targetDesc.vecInstanceDescs;

	const _float fDistancecPivot = 10.f;

	for (_uint i = 0; i < m_vecGraffleInfo.size(); i++)
	{
		// 타겟 위치 반영을 위한 계산..
		UI_GRAFFLE_RT_DESC& graffleInfo = m_vecGraffleInfo[i];
		_float3* targetPos = graffleInfo.tInfoDesc.pTargetPos;




		_float3 vInstSca = {
			XMVectorGetX(XMVector3Length(XMLoadFloat3(&*reinterpret_cast<_float3*>(&targetInstDesc[i].vSInstRight)))),
			XMVectorGetX(XMVector3Length(XMLoadFloat3(&*reinterpret_cast<_float3*>(&targetInstDesc[i].vSInstUp)))),
			XMVectorGetX(XMVector3Length(XMLoadFloat3(&*reinterpret_cast<_float3*>(&targetInstDesc[i].vSInstLook))))
		};
		_float3 targetScreenPos = Calc_PosToScreen(targetPos);		// 계산 결과 (3d상에서 화면으로)
		_float3 targetScreenSca = Calc_CamDistScale_PerInst(&vInstSca, fDistancecPivot);

		// static_cast<CTransform*>(m_pGrafflePointUI->Get_Component(L"Com_Transform"))->Set_State(STATE::POSITION, vPos);

		*reinterpret_cast<_float3*>(&targetInstDesc[i].vSInstTrans.x) = targetScreenPos;
		*reinterpret_cast<_float*>(&targetInstDesc[i].vSInstTrans.w) = 1.f;
	}

	pTargetUI->Set_UIDesc(targetDesc);
}

_float3 CUI_GrafflePoint::Calc_PosToScreen(const _float3* v3DPos)
{
	_float3 targetPos = *v3DPos;
	_matrix matCamView = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
	_matrix matCamProj = m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ);
	const _float2 vScreenSize = { g_iWinSizeX, g_iWinSizeY };

	_vector vTargetWorldPos = (v3DPos) ?
		XMVectorSetW(XMLoadFloat3(&targetPos), 1.f) : _vector();

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

	_vector vPos = XMVectorSet(vScreenPos.x, -vScreenPos.y, 0.f, 1.f);	// 결과 pos
	_float3 vStorePos;
	XMStoreFloat3(&vStorePos, vPos);

	return vStorePos;
}

_float3 CUI_GrafflePoint::Calc_CamDistScale_PerInst(const _float3* vInstSca, _float fPivotDistance)
{
	// 몬스터 hp때와 다르게, position은 손댈생각 말고, 인스턴스 별 크기만 손대면 될 거라 생각함






}

CUI_GrafflePoint* CUI_GrafflePoint::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_GrafflePoint* pInstance = new CUI_GrafflePoint(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_GrafflePoint");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_GrafflePoint::Clone(void* pArg)
{
	CUI_GrafflePoint* pInstance = new CUI_GrafflePoint(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_GrafflePoint");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_GrafflePoint::Free()
{
	if (m_isClone)
		m_pGameInstance->Remove_RootUI(L"UI_GrafflePoint");

	__super::Free();

	for (auto& child : m_vecChildObjects)
		Safe_Release(child);
}