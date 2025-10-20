#include "ClientPch.h"
#include "Parser.h"
#include	"MapObject.h"

CParser::CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance{ CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

void CParser::Create_Map_Model(const _char* pFilePath, LEVEL eLevel)
{
    ifstream File(pFilePath, ios::binary);

    if (!File.is_open())
    {
        CRASH("File Load Fail");
    }
    _uint NameLength = {};

    m_pGameInstance->Add_Work([=]() {
        m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MapObject"),
            CMapObject::Create(m_pDevice, m_pContext));
        });

    // 
    for (size_t i = 0; i < 10; i++)
    {
        ifstream File(pFilePath, ios::binary);

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
                    CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixIdentity(), ModelPath))))
                    int a = 0;
                });
        }
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
}

void CParser::Load_CSV(const _char* pFilePath)
{
}

HRESULT CParser::Initialize()
{
	return S_OK;
}

CParser* CParser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParser* pInstance = new CParser(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Parser");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParser::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
