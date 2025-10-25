#include "EditorPch.h"
#include "Level_Camera.h"

#include "SpringCamera_Edit.h"
#include "EditDummy_Wolf.h"
#include "EditDummy_Map.h"
#include "EditDummy_Target.h"

#include	"Map_Interface.h"
#include	"Camera_Interface.h"

#include "Sequencer.h"

CLevel_Camera::CLevel_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Camera::Initialize()
{
	//Ready_Camera();
	Ready_Dummy();
	Ready_Ground();

	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
	m_pCameraInterface = CCamera_Interface::Create(m_pDevice, m_pContext);
	
	m_pSequencer = CSequencer::Create();

    return S_OK;
}

void CLevel_Camera::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Camera"));

	ImGui::Begin("Camera Edit");

	if (ImGui::Button("MapInterface"))
		m_isMapInterface = !m_isMapInterface;

	if(true == m_isMapInterface)
		if (m_pMapInterface->Initialize_ModelPath(ENUM_CLASS(LEVEL::CAMERA), XMMatrixScalingFromVector(XMVectorSet(0.1f, 0.1f, 0.1f, 1.f))))
			m_pMapInterface->Add_MapObject();

	ImGui::End();

	m_pSequencer->Update(fTimeDelta);
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
	_matrix PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.005f, 0.005f, 0.005f, 1.f));
	WolfDesc.PreTransformMatrix = PreTransformationMatrix;

	WolfDesc.fSpeedPerSec = 100.f;
	WolfDesc.vPosition = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Wolf"),
		ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &WolfDesc)))
		CRASH("Failed Clone Dummy Wolf");

	CEditDummy_Target::DUMMY_TARGET_DESC TargetDesc = {};
	TargetDesc.PreTransformMatrix = PreTransformationMatrix;
	TargetDesc.vPosition = XMVectorSet(40.f, 0.f, 0.f, 1.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Target"),
		ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &TargetDesc)))
		CRASH("Failed Clone Dummy Target");

	// Dummy Map
	//PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.05f, 0.05f, 0.05f, 1.f));
	//CEditDummy_Map::DUMMY_MAP_DESC MapDesc = {};
	//MapDesc.PreTransformMatrix = PreTransformationMatrix;
	//MapDesc.vPosition = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Map"),
	//	ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &MapDesc)))
	//	CRASH("Failed Clone Dummy Map");
}

void CLevel_Camera::Ready_Ground()
{
	CRigidbody::BOXBODY_DESC BoxBodyDesc = {};
	BoxBodyDesc.eShape = SHAPE::BOX;
	BoxBodyDesc.vPos = _float3(0.f, -50.f, 0.f);
	BoxBodyDesc.vExtent = _float3(1000.f, 10.f, 1000.f);
	BoxBodyDesc.eType = EMotionType::Static;
	BoxBodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	
	m_pGround = CRigidbody::Create(m_pDevice, m_pContext);
	m_pGround->Initialize_Clone(&BoxBodyDesc);
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

	Safe_Release(m_pGround);

	Safe_Release(m_pSpringCamera);
	Safe_Release(m_pMapInterface);
	Safe_Release(m_pCameraInterface);

	Safe_Release(m_pSequencer);
}
