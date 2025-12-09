#include "ClientPch.h"
#include "UI_GrappleController.h"


#include "GameInstance.h"


CUI_GrappleController::CUI_GrappleController()
	: m_pGameInstance(CGameInstance::GetInstance())
{
	m_pGameInstance->AddRef();
}

HRESULT CUI_GrappleController::Initialize()
{
	return S_OK;
}

HRESULT CUI_GrappleController::Create_GrapplePoint(const _float3& vPointPos, UI_GRAPPLE_TYPE eType)
{
	_uint iDestLevel = m_pGameInstance->Get_CurrentLevel();


	CUI_GrapplePoint::UI_GRAPPLEPOINT_DESC pDesc = {};

	pDesc.vTargetPos = vPointPos;
	pDesc.eType = eType;

	CUI_GrapplePoint* pTargetPoint = dynamic_cast<CUI_GrapplePoint*>(m_pGameInstance->Clone_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI_GrapplePoint", PROTOTYPE::GAMEOBJECT, &pDesc));
	ASSERT_CRASH(pTargetPoint);

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_Image_GrapplePoint", pTargetPoint)))
		return E_FAIL;

	m_vecGrapplePoints.push_back(pTargetPoint);

	return S_OK;
}

CUI_GrapplePoint* CUI_GrappleController::Find_NearGrapplePoint(const _float3& vBasePos, UI_GRAPPLE_TYPE eType, _float* pOutDistance)
{
	if (m_vecGrapplePoints.empty())
		return nullptr;

	_float				fNearDistance	= FLT_MAX;
	CUI_GrapplePoint*	pNearPoint		= nullptr;

	for (auto& grapplePoint : m_vecGrapplePoints)
	{
		_bool isSameType = (grapplePoint->Get_GrappleType() == eType);
		_bool isActivate = grapplePoint->IsActivate();
		if (!isSameType || !isActivate)
			continue;

		_float3 vBasePosition = vBasePos;
		_float3 vPointPosition = grapplePoint->Get_TargetPos();		// UI의 Transform은 Screen 좌표계 기준이므로, 타겟으로 삼는 월드 좌표를 따로 받아옴

		_float fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vBasePosition) - XMLoadFloat3(&vPointPosition)));
		
		if (fDistance < fNearDistance)
		{
			fNearDistance = fDistance;
			pNearPoint = grapplePoint;
		}
	}

	if (pOutDistance)
		*pOutDistance = fNearDistance;

	return pNearPoint;
}

//HRESULT CUI_GrappleController::Disable_GrapplePoint(CUI_GrapplePoint* pUIPoint)
//{
//	pUIPoint->
//
//	return S_OK;
//}

CUI_GrappleController* CUI_GrappleController::Create()
{
	CUI_GrappleController* pInstance = new CUI_GrappleController();

	if (FAILED(pInstance->Initialize()))
	{
		Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CUI_GrappleController::Free()
{
	Safe_Release(m_pGameInstance);

	__super::Free();
}
