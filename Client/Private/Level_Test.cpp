#include "ClientPch.h"
#include "Level_Test.h"
#include "MapObject.h"
#include "MonsterTest.h"

CLevel_Test::CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CLevel(pDevice,pContext)
{
}

HRESULT CLevel_Test::Initialize()
{
    //ifstream File("../Bin/Resource/Map/MapData/Client_Test3_NonInteraction.dat", ios::binary);

    //if (!File.is_open())
    //{
    //    CRASH("File Load Fail");
    //}
    //_uint NameLength = {};
    //CMapObject::MAP_LOAD Desc{};

    //while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
    //{
    //    memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
    //    File.read(Desc.ModelName, NameLength);

    //    File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
    //    _float4x4 Matrix = {};
    //    File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
    //    Desc.WorldMatrix = &Matrix;

    //    _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
    //    _tchar Name[MAX_PATH] = {};
    //    MultiByteToWideChar(CP_ACP, 0, Desc.ModelName, -1, Name, strlen(Desc.ModelName));
    //    lstrcat(Model, Name);

    //    _char ModelPath[MAX_PATH] = "../../Client/Bin/Resource/Map/";
    //    strcat_s(ModelPath, Desc.ModelName);
    //    strcat_s(ModelPath, "/");
    //    strcat_s(ModelPath, Desc.ModelName);
    //    strcat_s(ModelPath, ".dat");

    //    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), Model,
    //        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixIdentity(), ModelPath));

    //    _tchar PrototypeObject[MAX_PATH] = TEXT("Prototype_GameObject_MapObject_");
    //    lstrcat(PrototypeObject, Name);

    //    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), PrototypeObject,
    //        CMapObject::Create(m_pDevice, m_pContext));

    //    m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), PrototypeObject
    //        , ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), &Desc);
    //}
	CMonsterTest::MONSTERTEST_DESC MobDesc = {};
	MobDesc.szPrototypeModelTag = TEXT("Prototype_Component_Model_FalseSoverign");
    MobDesc.fSpeedPerSec = 5.f;
    if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MonsterTest"), ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Monster"), &MobDesc)))
		CRASH("FalseSoverign");

	if(FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Test"), ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), nullptr)))
		CRASH("Dummy");

    /*if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Test"), ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"))))
        CRASH("Dummy");*/

    return S_OK;
}

void CLevel_Test::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Test"));
}

void CLevel_Test::Render()
{
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
}
