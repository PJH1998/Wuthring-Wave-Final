#include "ClientPch.h"
#include "Loader_Test.h"

#include "Dummy.h"
#include"MapObject.h"

CLoader_Test::CLoader_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Test::Initialize()
{
	CoInitializeEx(nullptr, 0);
    Load_Model();
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	//m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Ready_OctoTree(); Complete_Load(); });

    m_pGameInstance->Wait_Thread_End();
    return S_OK;
}

HRESULT CLoader_Test::Load_Texture()
{
	cout << "Texture" << endl;

    return S_OK;
}

HRESULT CLoader_Test::Load_Model()
{
	cout << "Model" << endl;

    _matrix PreTransformMatrix;
    _float fSize = 0.1f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

    string FolderPath = "../../Client/Bin/Resource/Map/";

    _int version = {};
    _int Lastversion = {};
    _wstring LastVersionName;
    _string LastVersionPath;

    for (const auto& entry : filesystem::recursive_directory_iterator(FolderPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().string().find("MapData") != std::string::npos)
                continue;

            if (entry.path().extension() == ".dat") {

                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                _wstring baseName = StringToWString(FileName);

                // LOD 마지막에 붙은 숫자 추출
                size_t pos = baseName.find_last_not_of(L"0123456789");
                _wstring namePart = baseName.substr(0, pos + 1);
                _wstring numberPart = baseName.substr(pos + 1);
                version = stoi(numberPart);

                _wstring key = L"Prototype_Component_Model_" + namePart;
                _string VersionPath = FileDir;
                VersionPath += FileName;
                VersionPath += ".dat";



                _wstring PrototypeName = L"Prototype_Component_Model_";
                PrototypeName += StringToWString(FileName);

                if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), PrototypeName,
                    CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, VersionPath.c_str()))))
                    CRASH("Prototype Create Failed");

            }
        }
    }
    return S_OK;
}

HRESULT CLoader_Test::Load_Shader()
{
	cout << "Shader" << endl;

    return S_OK;
}

HRESULT CLoader_Test::Load_Object()
{
	cout << "Object" << endl;

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_MapObject"),
        CMapObject::Create(m_pDevice, m_pContext));

    return S_OK;
}

HRESULT CLoader_Test::Ready_OctoTree()
{
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

	return S_OK;
}

CLoader_Test* CLoader_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Test* pInstance = new CLoader_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Test::Free()
{
    __super::Free();
}
