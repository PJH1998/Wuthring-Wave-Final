#include "ClientPch.h"
#include "Level_GamePlay.h"
#include"GameSystem.h"
#include "MonsterTest.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext), m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_GamePlay::Initialize()
{
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

	m_pGameSystem->Clone_MapObjects(m_eCurLevel, 0);

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_ShadowNF();


	// UI
	const   _uint       iDestLevel = m_pGameInstance->Get_CurrentLevel();
	const _wstring strLayertag_UI = L"Layer_Custom_UI";
	const _wstring strPrototypeTag_UI[] = {
		 L"Prototype_GameObject_Custom_UI_Container_HUD"
	};
	for (auto& strPrototypeTag : strPrototypeTag_UI)
	{
		CUIObject* pTargetUI = static_cast<CUIObject*>(m_pGameInstance->Clone_Prototype(iDestLevel, strPrototypeTag, PROTOTYPE::GAMEOBJECT));
		if (FAILED(m_pGameInstance->Add_RootUI(L"UI_UHD", pTargetUI)))
			CRASH("Failed to Add RootUI to UI_Manager.");
		if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, strLayertag_UI, pTargetUI)))
			CRASH("Failed to Add RootUI to Object_Manager.");
	}
	// _UI

	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();
	return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("GamePlay"));
}

void CLevel_GamePlay::Render()
{
}

void CLevel_GamePlay::Ready_Layer_Player()
{
}

void CLevel_GamePlay::Ready_Dummy()
{
}

void CLevel_GamePlay::Ready_MonsterTest()
{
	CMonsterTest::MONSTERTEST_DESC MobDesc{};
	MobDesc.eCurLevel = LEVEL::GAMEPLAY;
	MobDesc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	MobDesc.computeShaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"));
	MobDesc.modelData = make_pair(LEVEL::GAMEPLAY, TEXT("Prototype_Component_Model_FalseSovereign"));
	MobDesc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	MobDesc.fRotationPerSec = XMConvertToRadians(90.f);
	MobDesc.fSpeedPerSec = 10.f;
	MobDesc.vInitPosition = _float3(0.f, -8.f, 4.f);
	MobDesc.pAnimationTag = "Born1";
	if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_MonsterTest"),
		ENUM_CLASS(LEVEL::GAMEPLAY), TEXT("Layer_MonsterTest"), &MobDesc)))
		CRASH("Failed Ready MonsterTest");
}

CLevel_GamePlay* CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_GamePlay* pInstance = new CLevel_GamePlay(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_GamePlay::Free()
{
    __super::Free();
	Safe_Release(m_pGameSystem);
}
