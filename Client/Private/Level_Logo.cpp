#include "ClientPch.h"
#include "Level_Logo.h"

#include "Event_Level.h"

#include "Dummy.h"

CLevel_Logo::CLevel_Logo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Logo::Initialize()
{
	// Rigidbody Sample
	CRigidbody::BOXBODY_DESC BoxBodyDesc = {};
	BoxBodyDesc.eShape = SHAPE::BOX;
	BoxBodyDesc.vPos = _float3(10.f, 50.f, 0.f);
	BoxBodyDesc.vExtent = _float3(0.5f, 15.f, 0.5f);
	BoxBodyDesc.eType = EMotionType::Dynamic;
	BoxBodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	BoxBodyDesc.eBodyType = CRigidbody::BODYTYPE::BODY;
	
	m_pRigidbody1 = CRigidbody::Create(m_pDevice, m_pContext);
	m_pRigidbody1->Initialize_Clone(&BoxBodyDesc);
	//
	CRigidbody::BOXBODY_DESC BoxBodyDesc2 = {};
	BoxBodyDesc2.eShape = SHAPE::BOX;
	BoxBodyDesc2.vPos = _float3(0.f, -50.f, 0.f);
	XMStoreFloat4(&BoxBodyDesc2.vQuat, XMQuaternionRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(30.f)));
	BoxBodyDesc2.vExtent = _float3(100.f, 3.f, 100.f);
	BoxBodyDesc2.eType = EMotionType::Static;
	BoxBodyDesc2.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	
	m_pRigidbody2 = CRigidbody::Create(m_pDevice, m_pContext);
	m_pRigidbody2->Initialize_Clone(&BoxBodyDesc2);
	//
	//BoxBodyDesc2.vPos = _float3(10.f, 15.f, 0.f);
	//BoxBodyDesc2.eType = EMotionType::Static;
	//
	//m_pRigidbody3 = CRigidbody::Create(m_pDevice, m_pContext);
	//m_pRigidbody3->Initialize_Clone(&BoxBodyDesc2);

	CGameObject::GAMEOBJECT_DESC DummyDesc = {};
	DummyDesc.fSpeedPerSec = 10.f;
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_Dummy"), ENUM_CLASS(LEVEL::LOGO), TEXT("Layer_Dummy"), &DummyDesc)))
		CRASH("Dummy");
	
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_ShadowDummy"), ENUM_CLASS(LEVEL::LOGO), TEXT("Layer_Dummy"), &DummyDesc)))
		CRASH("ShadowDummy");
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::LOGO), TEXT("Prototype_GameObject_Dummy"), ENUM_CLASS(LEVEL::LOGO), TEXT("Layer_Dummy"))))
	//	CRASH("Dummy");

	//Safe_Release(pRigidBody);

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));

    return S_OK;
}

void CLevel_Logo::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Logo"));
	//m_pRigidbody->AddForce(_float3(1000000.f, 10000000.f, 0.f));
	if (m_pGameInstance->Get_DIKeyState(DIK_G) == KEYSTATE::DOWN)
	{
		m_pRigidbody1->OnGravity(false);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_H) == KEYSTATE::DOWN)
	{
		m_pRigidbody1->OnGravity(true);
	}

    if (m_pGameInstance->Get_DIKeyState(DIK_F1) == KEYSTATE::DOWN)
    {
        CHANGE_LEVEL_EVENT event{ LEVEL::GAMEPLAY, true };
        m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
    }
	if (m_pGameInstance->Get_DIKeyState(DIK_F2) == KEYSTATE::DOWN)
	{
		CHANGE_LEVEL_EVENT event{ LEVEL::TEST, true };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
	}
}

void CLevel_Logo::Render()
{
}

CLevel_Logo* CLevel_Logo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Logo* pInstance = new CLevel_Logo(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Logo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Logo::Free()
{
    __super::Free();

	Safe_Release(m_pRigidbody1);
	Safe_Release(m_pRigidbody2);
	//Safe_Release(m_pRigidbody3);
}
