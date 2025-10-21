#include "ClientPch.h"
#include "Level_Test.h"
#include "MapObject.h"
#include "AnimationDummy.h"
#include "MonsterTest.h"

#include "GameSystem.h"
#include "PlayerController.h"

CLevel_Test::CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext)
{
}

HRESULT CLevel_Test::Initialize()
{
	// SetUp OctoTree
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

    ifstream File("../Bin/Resource/Map/MapData/Client_ShadowTest_NonInteraction.dat", ios::binary);

    if (!File.is_open())
    {
        CRASH("File Load Fail");
    }
    _uint NameLength = {};
    CMapObject::MAP_LOAD Desc{};


    while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
    {
        memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
        File.read(Desc.ModelName, NameLength);

        File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
        _float4x4 Matrix = {};
        File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
        Desc.WorldMatrix = &Matrix;

        _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
        _tchar Name[MAX_PATH] = {};
        MultiByteToWideChar(CP_ACP, 0, Desc.ModelName, -1, Name, strlen(Desc.ModelName));
        lstrcat(Model, Name);

        _char ModelPath[MAX_PATH] = "../../Client/Bin/Resource/Map/";
        strcat_s(ModelPath, Desc.ModelName);
        strcat_s(ModelPath, "/");
        strcat_s(ModelPath, Desc.ModelName);
        strcat_s(ModelPath, ".dat");

        m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MapObject"), PROTOTYPE::GAMEOBJECT, &Desc);

        //m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MapObject")
        //    , ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), &Desc);
    }
	File.close();

    //Ready_Layer_Augusta();
    Ready_Layer_PlayerController();

	//CMonsterTest::MONSTERTEST_DESC MobDesc = {};
	//MobDesc.szPrototypeModelTag = TEXT("Prototype_Component_Model_FalseSoverign");
    //MobDesc.fSpeedPerSec = 5.f;
    //if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MonsterTest"), ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Monster"), &MobDesc)))
	//	CRASH("FalseSoverign");
	//
	//if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Test"), ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), nullptr)))
	//	CRASH("Dummy");

    /*if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Test"), ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"))))
        CRASH("Dummy");*/

    LIGHT_DESC LightDesc{};
    LightDesc.eType = LIGHT_DESC::DIRECTION;
    LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

    m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
    m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
    m_pGameInstance->SetUp_ShadowNF();

    return S_OK;
}

void CLevel_Test::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Test"));
}

void CLevel_Test::Render()
{
}

void CLevel_Test::Ready_Layer_PlayerController()
{
    _float3 vScale{}, vRotation{}, vPosition{};
    vScale = { 1.f, 1.f, 1.f };
    vRotation = { 0.f, 0.f, 0.f };
    vPosition = { -20.f, 50.f, -180.f };

    CPlayerController::PLAYER_CONTROLLER_DESC Desc{};
    Desc.eCurLevel = m_eCurLevel;
    Desc.iPlayerCount = CPlayerController::PLAYERTYPE::TYPE_END;

    // 0. vector 크기 정의
    Desc.PlayerSpecs.resize(CPlayerController::PLAYERTYPE::TYPE_END);

    // 1. Augusta 정의.
    Desc.PlayerSpecs[CPlayerController::PLAYERTYPE::AUGUSTA].PlayerDesc = PlayerData::GetAugustaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
    Desc.PlayerSpecs[CPlayerController::PLAYERTYPE::AUGUSTA].strActorTag = PlayerData::AUGUSTA_ACTOR_TAG;

    // 2. Galbrena 정의

    // 3. Player 정의

    // 4. Controller 생성.
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_PlayerController"),
        ENUM_CLASS(m_eCurLevel), TEXT("Layer_PlayerController"), &Desc)))
        CRASH("Failed Ready Layer Augusta");
}

void CLevel_Test::Ready_Layer_Augusta()
{
    _wstring wStrModelTag = L"Prototype_Component_Model_Augusta";
	_wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMesh");
	_wstring wstrComputeShaderTag = TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh");
    _uint iShaderPath = 0;

    CAnimationDummy::ANIMATION_ACTOR_DESC Desc{};
    Desc.fSpeedPerSec = 10.f;
    Desc.fRotationPerSec = XMConvertToRadians(90.f);
    Desc.strModelTag = wStrModelTag;
    Desc.strShaderTag = wstrShaderTag; 
    Desc.strComputeShaderTag = wstrComputeShaderTag;
    Desc.iShaderPath = iShaderPath;
    Desc.vScale = _float3(1.f, 1.f, 1.f);
    Desc.vRotation = _float3(0.f, 0.f, 0.f);
    Desc.vPostion = _float3(-14.1f, 50.f, -180.f);
    
    Desc.eLevel = m_eCurLevel;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Dummy_Augusta"),
        ENUM_CLASS(m_eCurLevel), TEXT("Layer_Augusta"), &Desc)))
        CRASH("Failed Ready Layer Augusta");
}

CLevel_Test* CLevel_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Test* pInstance = new CLevel_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Test::Free()
{
    __super::Free();
    Safe_Release(m_pGameSystem);
}
