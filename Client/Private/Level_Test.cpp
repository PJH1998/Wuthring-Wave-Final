#include "ClientPch.h"
#include "Level_Test.h"
#include "MapObject.h"
#include "AnimationDummy.h"
#include "MonsterTest.h"

#include "GameSystem.h"
#include "Player.h"

CLevel_Test::CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext)
{
}

HRESULT CLevel_Test::Initialize()
{
	// SetUp OctoTree
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));
    Ready_Layer_Map("../Bin/Resource/Map/MapData/Client_ShadowTest_NonInteraction.dat");

    Ready_Layer_Player();

	//CGameObject::GAMEOBJECT_DESC DummyDesc = {};
	//DummyDesc.fSpeedPerSec = 10.f;
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Dummy"), ENUM_CLASS(LEVEL::LOGO), TEXT("Layer_Dummy"), &DummyDesc)))
	//	CRASH("Dummy");

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

	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();

    return S_OK;
}

void CLevel_Test::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Test"));
    
}

void CLevel_Test::Render()
{
}

void CLevel_Test::Ready_Layer_Player()
{
    _float3 vScale{}, vRotation{}, vPosition{};
    //vScale = { 1.f, 1.f, 1.f };
    vScale = { 0.1f, 0.1f, 0.1f };
    vRotation = { 0.f, 0.f, 0.f };
    vPosition = { -14.1f, 50.f, -180.f };

    CPlayer::PLAYER_DESC Desc{};
    Desc.eCurLevel = m_eCurLevel;
    Desc.vScale = vScale;
    Desc.vRotation = vRotation;
    Desc.vPosition = vPosition;
    Desc.iPlayerCount = CPlayer::CHARACTERTYPE::TYPE_END;
    Desc.wStrInputControllerTag = TEXT("Prototype_Component_PlayerController");

    // 0. vector 크기 정의
    Desc.PlayerSpecs.resize(CPlayer::CHARACTERTYPE::TYPE_END);

    // 1. Augusta 정의.
    Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].CharacterDesc = PlayerData::GetAugustaCloneData(vScale, vRotation, vPosition, m_eCurLevel);
    Desc.PlayerSpecs[CPlayer::CHARACTERTYPE::AUGUSTA].strActorTag = PlayerData::AUGUSTA_ACTOR_TAG;

    // 2. Galbrena 정의


    // 3.주인공 캐릭터 정의


    // 4. Player(Character 모음) 생성.
    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Player"),
        ENUM_CLASS(m_eCurLevel), TEXT("Layer_Players"), &Desc)))
        CRASH("Failed Ready Player");
}



HRESULT CLevel_Test::Ready_Layer_Map(const _char* pFilePath)
{
    _char FileDrive[MAX_PATH] = {};
    _char FileDir[MAX_PATH] = {};
    _char FileName[MAX_PATH] = {};
    _char FileExt[MAX_PATH] = {};

    _splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

    _string PasingDir = FileDir;
    if (strlen(FileName) > 0)
    {
        PasingDir += FileName;
        PasingDir += FileExt;
        Read_Map_Dat(pFilePath);
    }
    else
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().extension() != ".dat")
                continue;

            _string strFilePath = entry.path().string();
            Read_Map_Dat(strFilePath);
        }
    }
    return S_OK;
}

void CLevel_Test::Read_Map_Dat(const _string pFilePath)
{
    ifstream File(pFilePath, ios::binary);

    if (!File.is_open())
    {
        MSG_BOX("Load Failed");
    }
    if (pFilePath.find("Instance") != std::string::npos)
    {
        return;

        //CMapObject::MAP_LOAD Desc{};

        //_matrix PreTransformMatrix = XMMatrixIdentity();
        //_float fSize = 0.01f;
        //PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

        //CMapObject_Instance::MAP_LOAD Desc{};
        //Desc.iNumInstance;
        //Desc.ModelName;
        //Desc.m_WolrdPos;
        //Desc.WorldMatrix;

        //while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
        //{
        //    memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
        //    File.read(Desc.ModelName, NameLength);

        //    File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
        //    File.read(reinterpret_cast<char*>(&Desc.m_WolrdPos), sizeof(_float4));

        //    File.read(reinterpret_cast<char*>(&Desc.iNumInstance), sizeof(_uint));
        //    _float4x4* pMatrix = new _float4x4[Desc.iNumInstance];
        //    File.read(reinterpret_cast<char*>(pMatrix), sizeof(_float4x4) * Desc.iNumInstance);

        //    Safe_Delete_Array(pMatrix);

        //    _wstring PrototypeName = TEXT("Prototype_Component_Model_Instance_");

        //    //프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
        //    _wstring ModelName = StringToWString(Desc.ModelName);


        //    ModelName.pop_back();
        //    _string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
        //    ProjectPath += "/Client/Bin/Resource/Map";
        //    for (const auto& entry : filesystem::recursive_directory_iterator(ProjectPath)) {
        //        if (entry.is_regular_file()) {
        //            if (entry.path().string().find("json") != std::string::npos)
        //                continue;
        //            if (entry.path().string().find(WStringToString(ModelName)) != std::string::npos)
        //            {
        //                _string ModelPath = entry.path().string();

        //                m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(entry.path().stem().string()), Path = ModelPath]() {
        //                    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), Model,
        //                        CModel_Instance::Create(pDevice, pContext, PreTransformMatrix, Path.c_str()))))
        //                        CRASH("Failed");
        //                    });
        //            }
        //        }
        //    }
        //}

    }
    else
    {
        _uint NameLength;

        _matrix PreTransformMatrix = XMMatrixIdentity();
        _float fSize = 0.01f;
        PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

        CMapObject::MAP_LOAD Desc{};

        while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
        {
            memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
            File.read(Desc.ModelName, NameLength);

            File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
            _float4x4 Matrix = {};
            File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
            Desc.WorldMatrix = &Matrix;
            _wstring PrototypeName = TEXT("Prototype_Component_Model_");

            //프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
            _wstring ModelName = StringToWString(Desc.ModelName);

            m_pGameInstance->Clone_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject")
                ,PROTOTYPE::GAMEOBJECT, &Desc);
        }
    }
    File.close();
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
