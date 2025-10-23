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
        Read_Map_Dat(PasingDir, eLevel);
    }
    else
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(PasingDir)) {
            if (!entry.is_regular_file())
                continue;
            if (entry.path().extension() != ".dat")
                continue;

            _string strFilePath = entry.path().string();
            Read_Map_Dat(strFilePath, eLevel);
        }
    }
}

void CParser::Read_Map_Dat(const _string pFilePath, LEVEL eLevel)
{

    _matrix PreTransformMatrix = XMMatrixIdentity();
    //_float fSize = 0.01f;
    _float fSize = 0.1f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

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
        //                    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), Model,
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

            ModelName.pop_back();

            _string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
            ProjectPath += "/Client/Bin/Resource/Map";
            for (const auto& entry : filesystem::recursive_directory_iterator(ProjectPath)) {
                if (!entry.is_regular_file())
                    continue;
                if (entry.path().string().find("json") != std::string::npos)
                    continue;
                if (entry.path().string().find(WStringToString(ModelName)) == std::string::npos)
                    continue;

                _string ModelPath = entry.path().string();

                m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(entry.path().stem().string()), Path = ModelPath]() {
                    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), Model,
                        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, Path.c_str()))))
                        CRASH("Failed");
                    });

            }
        }
    }
    File.close();
}

const vector<vector<_string>>& CParser::Load_CSV(const _char* pFilePath)
{
	ifstream InputFile(pFilePath);

	_string strLine;

	while (getline(InputFile, strLine))
	{
		stringstream ss(strLine);
		_string strCell;
		vector<_string> row;

		while (getline(ss, strCell, ','))
			row.push_back(strCell);

		m_Data.push_back(row);
	}

	InputFile.close();

	return m_Data;
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
