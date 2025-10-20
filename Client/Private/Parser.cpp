#include "ClientPch.h"
#include "Parser.h"
#include"MapObject.h"
IMPLEMENT_SINGLETON(CParser)

CParser::CParser()
	: m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CParser::Create_Map_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, LEVEL eLevel)
{
    ifstream File(pFilePath, ios::binary);

    if (!File.is_open())
    {
        CRASH("File Load Fail");
    }
    _uint NameLength = {};

    m_pGameInstance->Add_Work([=]() {
        m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MapObject"),
            CMapObject::Create(pDevice, pContext));
        });


    //ifstream File(pFilePath, ios::binary);
    m_pGameInstance->Wait_Thread_End();

    if (!File.is_open())
    {
        CRASH("File Load Fail");
    }
    while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
    {
        CMapObject::MAP_LOAD Desc{};
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

        m_pGameInstance->Add_Work([=]() {
            if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), Model,
                CModel::Create(pDevice, pContext, MODELTYPE::MAP, XMMatrixIdentity(), ModelPath))))
                int a = 0;
            });
        File.close();
    }
    //while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
    //{
    //    CMapObject::MAP_LOAD Desc{};
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

    //    m_pGameInstance->Add_Work([=]() {
    //        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), Model,
    //            CModel::Create(pDevice, pContext, MODELTYPE::MAP, XMMatrixIdentity(), ModelPath))))
    //            int a = 0;
    //        });
    //    
    //    //_tchar PrototypeObject[MAX_PATH] = TEXT("Prototype_GameObject_MapObject_");
    //    //lstrcat(PrototypeObject, Name);


    //    //m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::TEST), PrototypeObject
    //    //    , ENUM_CLASS(LEVEL::TEST), TEXT("Layer_Test"), &Desc);
    //}
    //File.close();


    //폴더 아래에 있는 .Dat 다 읽는 거. -> 파일 경로를 폴더 경로로 굳이 안줘도 됨.

    _char FileDrive[MAX_PATH] = {};
    _char FileDir[MAX_PATH] = {};
    _char FileName[MAX_PATH] = {};
    _char FileExt[MAX_PATH] = {};

    _splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

    for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
        if (entry.is_regular_file()) {
            if (entry.path().extension() == ".dat") {

                _string strFilePath = entry.path().string();
                //ifstream File(strFilePath, ios::binary);
            }
        }
    }

}

void CParser::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
