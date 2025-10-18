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
	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
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

    _uint i = 0;
    for (const auto& entry : filesystem::recursive_directory_iterator(FolderPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().string().find("MapData") != std::string::npos)
                continue;

            //LOD 紐⑤뜽?ㅼ? 紐⑸줉??異붽??섏? 留먭퀬 _LOD0 ?대쫫 鍮쇨퀬 1媛쒖뵫留???ν븯寃?
            if (entry.path().extension() == ".dat") {
                if (i > 5)
                    break;
                //m_ModelPaths.push_back(entry.path().string());

                //?ш린???꾨줈?좏???誘몃━ ?앹꽦
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                _wstring ProtoModelPath = TEXT("Prototype_Component_Model_");
                _wstring  ProtoModelName = ProtoModelPath + StringToWString(FileName);

                _wstring  PushName = ProtoModelPath + StringToWString(FileName);
                PushName.pop_back();
                //
                _bool IsExists = { false };

                _string Temp;
                Temp += FileDir;
                Temp += FileName;
                Temp.pop_back();

                //for (_uint i = 0; i < m_PrototypeNames.size(); ++i)
                //{
                //    _wstring PopName = m_PrototypeNames[i];
                //    PopName.pop_back();

                //    if (!lstrcmp(PushName.c_str(), PopName.c_str()))
                //    {
                //        IsExists = true;
                //        break;
                //    }
                //}
                //if (!IsExists)
                //{
                //    m_PrototypeNames.push_back(ProtoModelName);
                //    m_ModelPaths.push_back(Temp);
                //}

                _string FilePath = entry.path().string();
                //m_pGameInstance->Add_Work([=]() {
                if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), ProtoModelName,
                    CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, FilePath.c_str()))))
                    //CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixIdentity(), FilePath.c_str()))))
                    CRASH("Prototype Create Failed");

                i++;
                //硫?곗벐?덈뱶 ?뺤긽???섎㈃ ?닿굅 ?멸쾬.
                //    string Test = entry.path().parent_path().string();
                //    Test += "/Mat/Tex/";
                //    if (filesystem::exists(Test))
                //        m_pPreViewObject->Add_Model(ProtoModelName);
                //});
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
