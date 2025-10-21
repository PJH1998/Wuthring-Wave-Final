#include "EditorPch.h"
#include "Level_Camera.h"

#include "SpringCamera_Edit.h"
#include "EditDummy_Wolf.h"

#include"Map_Interface.h"


CLevel_Camera::CLevel_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Camera::Initialize()
{
	//Ready_Camera();
	Ready_Dummy();

	//class CMap_Interface* pMap = { nullptr };
	/*pMap = CMap_Interface::Create(m_pDevice, m_pContext);
	if (pMap->Initialize_ModelPath(ENUM_CLASS(LEVEL::CAMERA), XMMatrixScalingFromVector(XMVectorSet(0.1f, 0.1f, 0.1f, 1.f))))
		pMap->Add_MapObject();*/

    return S_OK;
}

void CLevel_Camera::Update(_float fTimeDelta)
{

	SetWindowText(g_hWnd, TEXT("Camera"));
}

void CLevel_Camera::Render()
{
}

void CLevel_Camera::Ready_Camera()
{
	m_pSpringCamera = CSpringCamera_Edit::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pSpringCamera);

	CSpringCamera_Edit::CAMERA_DESC CameraDesc = {};
	CameraDesc.fSpeedPerSec = 100.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 5000.f;
	CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
	CameraDesc.fMouseSensor = 0.004f;

	m_pSpringCamera->Initialize_Clone(&CameraDesc);

	m_pGameInstance->Add_Camera(ENUM_CLASS(LEVEL::STATIC), TEXT("Camera_Spring"), m_pSpringCamera);
	Safe_AddRef(m_pSpringCamera);

	m_pGameInstance->Change_MainCamera(ENUM_CLASS(LEVEL::STATIC), TEXT("Camera_Spring"));
}

void CLevel_Camera::Ready_Dummy()
{
	CEditDummy_Wolf::DUMMY_WOLF_DESC WolfDesc = {};
	_matrix PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));
	WolfDesc.PreTransformMatrix = PreTransformationMatrix;

	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Wolf"),
	//	ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &WolfDesc)))
	//	CRASH("Failed Clone Dummy Wolf");

	WolfDesc.fSpeedPerSec = 100.f;
	WolfDesc.vPosition = XMVectorSet(0.f, -200.f, 0.f, 1.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Wolf"),
		ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &WolfDesc)))
		CRASH("Failed Clone Dummy Wolf");
}

CLevel_Camera* CLevel_Camera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Camera* pInstance = new CLevel_Camera(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Level_Camera");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Camera::Free()
{
	__super::Free();

	Safe_Release(m_pSpringCamera);
}
