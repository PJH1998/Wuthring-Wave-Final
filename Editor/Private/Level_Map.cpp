#include "EditorPch.h"
#include "Level_Map.h"

#include "Event_Level.h"
#include"Edit_MapObject.h"
#include"Edit_MapObject_Instance.h"
#include"Edit_PreViewModel.h"
#include"Edit_LightObject.h"
#include"Edit_Brush.h"

_float3 CLevel_Map::m_vWorldPos = {};
_float3 CLevel_Map:: m_vWorldDir = {};
_float4 CLevel_Map::m_vPickedPos = _float4(0.f,0.f,0.f,1.f);

CLevel_Map::CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
    // ���彺�������� ���� �޽� ���� �귯�� ����� �޸�
    // ���� Ÿ���� ����ͼ� ���ε�. ���� ��ġ�� ��ȯ.
    // �� ���콺 ��ġ �� �ϳ� VS_IN���� ������, Range ���� ���̴� ����.
    // ���� Ÿ���� w���� 0�̸� discard
    // GS���̴����� �簢 ���� ����, �� �������� ���� �귯�� ����? => �簢�� ���� ���� �� ���� ���� ����?
    // ����, Y�� ���� ȸ�� ������ġ, ����, ����..? �Է� �����ϰ� ?
    
    // ��ư ������ ������ �� �ְ�?
}

HRESULT CLevel_Map::Initialize()
{
    Ready_Event();

    if (FAILED(Ready_Static_Component()))
        return E_FAIL;

	// OctoTree SetUp
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

    //ImGui::GetIO().DisplayFramebufferScale = ImVec2(1.25f, 1.25f);

    return S_OK;
}

void CLevel_Map::Update(_float fTimeDelta)
{
    m_fNearDistance = FLT_MAX;
    m_fNearDistance_Instance = FLT_MAX;
    _float3 Test;
    if(m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
    {
        if (m_pGameInstance->isPicked(&Test))
            int a = 0;
    }
    SetWindowText(g_hWnd, TEXT("Map"));
    Menu_Select();

    switch (m_eMenu)
    {
    case Editor::CLevel_Map::MENU_OBJECT:
        Menu_Object();
        break;

    case Editor::CLevel_Map::MENU_RANDSCAPE:
        Menu_RandSacpe();
        m_pBrush->Update(fTimeDelta);
        break;

    case Editor::CLevel_Map::MENU_LIGHT:
        Menu_Light();
        break;

    case Editor::CLevel_Map::MENU_MAPSAVELOAD:
        Menu_Save_Load();
        break;
    case Editor::CLevel_Map::MENU_OBJECTLOAD:
        Menu_Model_Load();
        break;
    }
    Make_MousePos();
}

void CLevel_Map::Render()
{
}

void CLevel_Map::Menu_Select()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::MenuItem("Ojbect")) {
            m_eMenu == MENU_OBJECT ? m_eMenu = END : m_eMenu = MENU_OBJECT;
        }

        if (ImGui::MenuItem("RandScape")) {
            m_eMenu == MENU_RANDSCAPE ? m_eMenu = END : m_eMenu = MENU_RANDSCAPE;
        }

        if (ImGui::MenuItem("Light")) {
            m_eMenu == MENU_LIGHT ? m_eMenu = END : m_eMenu = MENU_LIGHT;
        }

        if (ImGui::MenuItem("Map Save & Load")) {
            m_eMenu == MENU_MAPSAVELOAD ? m_eMenu = END : m_eMenu = MENU_MAPSAVELOAD;
        }

        if(ImGui::MenuItem("Object Load")) {
            m_eMenu == MENU_OBJECTLOAD ? m_eMenu = END : m_eMenu = MENU_OBJECTLOAD;
        }

        _float4 CamPos = *m_pGameInstance->Get_CamPos();
        _char szCamPos[64] = {};
        sprintf_s(szCamPos, "Cam Pos - X : %.1f, Y : %.1f, Z : %.1f", CamPos.x, CamPos.y, CamPos.z);

        ImGui::Text(szCamPos);

        sprintf_s(szCamPos, "Picked Pos - X : %.1f, Y : %.1f, Z : %.1f", m_vPickedPos.x, m_vPickedPos.y, m_vPickedPos.z);
        ImGui::Text(szCamPos);
        ImGui::EndMainMenuBar();
    }
}

void CLevel_Map::Menu_Object()
{
    ImGui::Begin("Menu_Object");


    //
    if (m_pPickedObject)
        m_pPickedObject->Set_ImGuiOption();

    ImGui::End();
}

void CLevel_Map::Menu_RandSacpe()
{
    ImGui::Begin("Menu_RandScape");

    if (m_pPickedInstanceObject)
        m_pPickedInstanceObject->Set_ImGuiOption();
#pragma region 랜드스케이프
    //??쇨린?ㅼ? ?뚮젅?댁뼱??媛源뚯씠 ?덉쓣 ???뚮젅?댁뼱瑜?以묒젏?쇰줈 ?놁쑝濡??꾩?. ?꾩슫 ?곹깭濡?諛붾엺???붾뱾由?
    //?뚮젅?댁뼱??嫄곗쓽 寃뱀튇 ??쇨린?ㅼ? Clip?섎뒗?? ?덈낫??
    //?吏곸씪 ???뚮젅?댁뼱 諛쒕컮?μ뿉 諛쒖옄援??곗뭡 ?앷?. 留덉뒪???대?吏 媛숈? 嫄곕줈 ?섎뒗??
    //洹몃┝??吏?怨녹씠???꾨땶 怨녹씠???묎컳???대몢?. 臾댁“嫄?留덉뒪??

    // ?먰봽??諛쒖옄援?? ?덉깮湲곗?留????뚯? 李⑹???????쇨린媛 ?ы븯寃??붾뱾由?(?대뼸寃???)
    //踰쎌뿉???щ━湲?????諛??꾩튂??諛쒖옄援??곗뭡 ????댄럺?멸? ?앷?.

    //留듭뿉 源붾젮?덈뒗 ?꾩씠?쒖쓣 癒뱀쓣 ?뚮뒗 諛붾떏???섎춪???붾뱾由щ뒗 ?댄럺???앷린硫댁꽌 ?щ씪吏?
    //洹쇱쿂??癒뱀쓣 ???덈뒗(?곹샇?묒슜 媛?ν븳 ?꾩씠?쒖씠 ?덉쑝硫?UI ?앹꽦. ?쇱젙 二쇨린留덈떎 寃됰?遺꾩씠 鍮쏅궓.
    //諛붾떏 ??쇨린 留먭퀬 ??????쇨린?ㅼ씠??紐?鍮꾨퉴 ???뚮━ ?섏빞??(肄쒕씪?대뜑?) ?섎꽕???묎컳???뚮젅?댁뼱 ?꾩튂??留욎떠???붾뱾由щ뒗??

    //?꾩튂???곕씪 ?붾젆?붾꼸?쇱씠???뷀벂利??됱씠 諛붾뚮뒗?? -> 洹몃깷 硫붿돩媛 ?ㅻⅨ 嫄곗씪 ?섎룄

    //諛붾엺???붾뱾由щ뒗 諛⑺뼢? 紐⑤몢 媛숈? 諛⑺뼢????

    //?뚯? ?뗣뀑 洹몃깷 ?먯뀑?ㅽ넗?댁뿉???쇱삩??뀑??
    //?뱀젙 ?꾩튂???곕씪 ??쇨린???됱씠 ?덇툑??諛붾?
    //愿묐Ъ瑜섎뒗 硫由??덉쑝硫?鍮ㅼ쭩鍮ㅼ쭩??


    //湲몄갼湲? 媛留뚰엳 ?덉쑝硫?紐⑺몴 ?꾩튂濡??쇰쟻?대뒗 ?댄럺???앷린硫댁꽌 湲??뚮젮以? 臾댁“嫄?1?먭? ?꾨땲??醫뚯슦濡?履쇰걫???붾뱾由щ뒗 ?댄럺?몄씤??
    //嫄곕━媛 醫 ?덉쑝硫??덇컻媛 ?댁쭩 源붾━??留듬룄 ?덈뒗 嫄?媛숈쓬.
#pragma endregion
    ImGui::End();
}

void CLevel_Map::Menu_Light()
{
    // 議곕챸. ?쇰떒 Imgui??List濡??꾩옱 ?닿? ?ｌ? 議곕챸???뺣낫? ?쒖꽌 ?꾩슦湲? 踰꾪듉?뺤떇?쇰줈 ?꾨Ⅴ硫?洹?議곕챸??????뺣낫媛 ?섏삤寃?
       // ?쇱씠???ㅻ툕?앺듃瑜??섎굹 留뚮뱾?댁꽌 洹??덉쓽 ?꾩튂 ?뺣낫瑜?議곕챸?쇰줈. 議곗젅?????덇쾶. -> ?쇱씠??媛앹껜媛 ?꾩옱 異붽???議곕챸??以묒뿉??紐?踰덉㎏ ?쒖꽌?몄?
       // 媛곸쥌 ?됱긽?뺣낫 諛??멸린, ?뷀벂利??곕퉬?명듃 湲고? ?깅벑 ???섏젙?????덇쾶. -> ?ㅼ떆媛??곸슜? or 踰꾪듉 ?꾨Ⅴ硫??곸슜. ?섎룎由ш린 湲곕뒫???덉쓬 醫뗭쓣??
       // ?먯“紐낆뿉??洹몃┝???놁쓬.
       //湲곗쫰紐??ш굅硫?議곕챸???ш린. 
    

    //?댁깫湲??꾨Ⅴ硫??????앷??
}

void CLevel_Map::Menu_Model_Load()
{
    //?대┃ ?섍린 ?꾧퉴吏 留덉슦???꾩튂 ?곕씪?ㅻ땲湲?.
    ImGui::Begin("Model Table", nullptr, ImGuiWindowFlags_NoTitleBar);
    if (ImGui::BeginTable("Test", 1, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TableNextColumn();

        for (_uint i = 0; i < m_ModelPaths.size(); ++i)
        {
            _char FileDrive[MAX_PATH] = {};
            _char FileDir[MAX_PATH] = {};
            _char FileName[MAX_PATH] = {};
            _char FileExt[MAX_PATH] = {};
            _splitpath_s(m_ModelPaths[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);


            if (ImGui::Selectable(FileName))
            {
                CEdit_MapObject::MAP_LOAD Desc{};
                _float4x4 DefaultMatrix{};
                XMStoreFloat4x4(&DefaultMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&m_vPickedPos)));
                Desc.WorldMatrix = &DefaultMatrix;
                strcpy_s(Desc.ModelName, FileName);
                //m_pGameInstance->Clone_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject"), PROTOTYPE::GAMEOBJECT, &Desc);
                m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
                    , m_iLevel, TEXT("Layer_Test"), &Desc);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::Begin("PreView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
                m_szPreViewModelName = StringToWString(FileName);
                ImGui::Image(m_pGameInstance->Get_Debug_RT_Resource(TEXT("RT_Debug")), ImVec2(128, 128));
                ImGui::End();
                m_pPreViewObject->Late_Update(0.016f, m_szPreViewModelName);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void CLevel_Map::Menu_Save_Load()
{
    IGFD::FileDialogConfig config;

    config.path = "../../Client/Bin/Resource/Map/MapData/";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    static _char exportText[128] = ""; // ?낅젰 ??μ슜 踰꾪띁
    ImGui::InputText("?뚯씪 ?대쫫", exportText, IM_ARRAYSIZE(exportText));

    if (ImGui::BeginMenu("Save"))
    {
        for (auto& Pair : m_SaveObjects)
            if (ImGui::MenuItem(Pair.first.c_str()))
            {
                string MapName = config.path;
                MapName += exportText;
                MapName += "_";
                MapName += Pair.first;
                MapName += ".dat";
                ofstream File(MapName, ios::binary);

                MAP_SAVE event(File);
                m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), event);
                File.close();
            }
        //?곹샇?묒슜??媛앹껜???곕줈, ?몄뒪?댁떛 媛앹껜???곕줈, ?쇰컲 留??곕줈.

        ImGui::EndMenu();
    }
    ImGui::MenuItem("Load", nullptr, &m_LoadMenu);
    if (m_LoadMenu)
    {

        ImGuiFileDialog::Instance()->OpenDialog("Map File Load", "Import File", ".dat", config);

        if (ImGuiFileDialog::Instance()->Display("Map File Load")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                _string ModelPath;
                ModelPath+= config.path;
                
                _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
                ModelPath+= ImGuiFileDialog::Instance()->GetCurrentFileName();

                //ifstream File(ModelPath, ios::binary);
                ifstream File(strFilePath, ios::binary);

                if (!File.is_open())
                {
                    MSG_BOX("Load Failed");
                }

                _uint NameLength;

                _matrix PreTransformMatrix = XMMatrixIdentity();
                _float fSize = 0.01f;
                PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

                CEdit_MapObject::MAP_LOAD Desc{};

                while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
                {
                    memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
                    File.read(Desc.ModelName, NameLength);
                    
                    File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
                    _float4x4 Matrix = {};
                    File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
                    Desc.WorldMatrix = &Matrix;

                    //_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
                    //_tchar Name[MAX_PATH] = {};
                    //MultiByteToWideChar(CP_ACP, 0, Desc.ModelName, -1, Name, strlen(Desc.ModelName));
                    //lstrcat(Model, Name);

                    //_char ModelPath[MAX_PATH] = "../../Client/Bin/Resource/Map/";
                    //strcat_s(ModelPath, Desc.ModelName);
                    //strcat_s(ModelPath, "/");
                    //strcat_s(ModelPath, Desc.ModelName);
                    //strcat_s(ModelPath, ".dat");

                    //m_pGameInstance->Add_Prototype(m_iLevel, Model,
                    //    CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixIdentity(), ModelPath));

                    //_tchar PrototypeObject[MAX_PATH] = TEXT("Prototype_GameObject_MapObject");
                    //lstrcat(PrototypeObject, Name);

                    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
                        , m_iLevel, TEXT("Layer_Test"), &Desc);

                }
                File.close();
                //?덉씠?대굹 ?ㅻ툕?앺듃留ㅻ땲? ?꾩껜 ?쒗쉶媛?ν븳 ?⑥닔 ?앷린硫?蹂寃?怨좊젮 ?대낵寃?


                //LOD瑜?移대찓??嫄곕━ 湲곕컲?쇰줈 ?섏? 留먭퀬, 紐⑤뜽??理쒖냼 理쒕? ?쎌?濡??먮툕瑜?留뚮뱾?덉쓣 ??洹??먮툕媛
                //?꾩옱 ?붾㈃??湲곗??쇰줈 ?쎌????쇰쭏??留롮씠 李⑥??섍퀬 ?덈굹濡?LOD ?④퀎 援щ퀎?섍린. => ?ㅽ겕由??쎌? ?ъ씠利?湲곕쾿
                //LOD 紐⑤뜽? ?곹깭癒몄떊??媛덉븘?쇱슦??LOD ?④퀎???곕씪 諛붽씀湲? => ?댁감??紐⑤뜽???ш린??蹂?섏? ?딆쓬. ?뷀뀒?쇱씠 ?щ씪吏?
                m_LoadMenu = !m_LoadMenu;
                ImGuiFileDialog::Instance()->Close();

            }
            else
            {
                m_LoadMenu = !m_LoadMenu;
                ImGuiFileDialog::Instance()->Close();
            }
        }
    }
}

void CLevel_Map::Load_Objects()
{
    m_ModelPaths.clear();

    m_pPreViewObject = CEdit_PreViewModel::Create(m_pDevice, m_pContext);
    string FolderPath = "../../Client/Bin/Resource/Map/";
    vector<_wstring> m_PrototypeNames;

    _matrix PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.1f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

    for (const auto& entry : filesystem::recursive_directory_iterator(FolderPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().string().find("MapData") != std::string::npos)
                continue;

            //LOD 紐⑤뜽?ㅼ? 紐⑸줉??異붽??섏? 留먭퀬 _LOD0 ?대쫫 鍮쇨퀬 1媛쒖뵫留???ν븯寃?
            if (entry.path().extension() == ".dat") {
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

                for (_uint i = 0; i < m_PrototypeNames.size(); ++i)
                {
                    _wstring PopName = m_PrototypeNames[i];
                    PopName.pop_back();

                    if (!lstrcmp(PushName.c_str(), PopName.c_str()))
                    {
                        IsExists = true;
                        break;
                    }
                }
                if (!IsExists)
                {
                    m_PrototypeNames.push_back(ProtoModelName);
                    m_ModelPaths.push_back(Temp);
                }

                _string FilePath = entry.path().string();
                //m_pGameInstance->Add_Work([=]() {
               if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoModelName,
                   CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, FilePath.c_str()))))
                   //CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixIdentity(), FilePath.c_str()))))
                   CRASH("Prototype Create Failed");

                    //硫?곗벐?덈뱶 ?뺤긽???섎㈃ ?닿굅 ?멸쾬.
                    //    string Test = entry.path().parent_path().string();
                    //    Test += "/Mat/Tex/";
                    //    if (filesystem::exists(Test))
                    //        m_pPreViewObject->Add_Model(ProtoModelName);
                    //});
            }
        }
    }
    m_pGameInstance->Wait_Thread_End();

    for (_uint i = 0; i < m_PrototypeNames.size(); ++i)
    {
        m_pPreViewObject->Add_Model(m_PrototypeNames[i]);
    }

}

HRESULT CLevel_Map::Ready_Static_Component()
{
    _matrix PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.001f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.0f));


    //?쇰컲 紐⑤뜽
    //m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf"), CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));

    //?몄뒪?댁뒪 紐⑤뜽
    
    /*m_pGameInstance->Add_Work([&](){
        m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf_Instance"),
            CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));
        });*/

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf_Instance"),
        CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Test_Instance"),
        CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, "../../Client/Bin/Resource/Test/Test.dat"));


    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf"), 
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance_Wolf"),
        CEdit_MapObject_Instance::Create(m_pDevice, m_pContext));



    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
     CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_Brush"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxPoint.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_VIBuffer_Point"),
        CVIBuffer_Point::Create(m_pDevice, m_pContext));

    //VTXMESHINSTANCE
    
    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject"),
        CEdit_MapObject::Create(m_pDevice, m_pContext));
    
    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_LightObject"),
        CEdit_LightObject::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_Brush"),
        CEdit_Brush::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_LightObject")
        , m_iLevel, TEXT("Layer_Light"));

    //?ㅻ툕?앺듃留ㅻ땲??먯꽌 ?덉씠???꾨? ?뚮㈃???쒖감?곸쑝濡????
    //LOD 媛쒖닔 LOD0, LOD1, LOD2媛숈씠 LOD ?섎룄 ????

    //?먮툕 ?덉뿉 紐⑤뜽 李띻린 / ?붾뱶 理쒕? ?ш린 ?덉뿉 李띿뼱?쇳븳??
    //?쇰떒 ?띿뒪爾??놁씠 紐⑤뜽留?濡쒕뱶?대넃湲??몄씠釉?& 濡쒕뱶.
    
    //CEdit_MapObject::MAP_LOAD Desc{};
    //CEdit_MapObject_Instance::MAP_LOAD InstanceDesc{};
    //_float4x4 DefaultMatrix{};
    //XMStoreFloat4x4(&DefaultMatrix, XMMatrixIdentity());
    //InstanceDesc.WorldMatrix = Desc.WorldMatrix = &DefaultMatrix;

    //strcpy_s(InstanceDesc.ModelName, "Wolf_Instance");

    //strcpy_s(Desc.ModelName, "Test1");

    //m_pGameInstance->Add_GameObject_ToLayer()
    Load_Objects();
    m_pBrush = CEdit_Brush::Create(m_pDevice, m_pContext);
    return S_OK;
}

void CLevel_Map::Ready_Event()
{
    m_pGameInstance->Subscribe<MAP_PICK>(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), [this](const MAP_PICK& event) {
        if (event.fDistance <= m_fNearDistance)
        {
            m_fNearDistance = event.fDistance;
            XMStoreFloat4(&m_vPickedPos, XMVectorSetW(XMLoadFloat3(&m_vWorldPos) + m_fNearDistance * XMLoadFloat3(&m_vWorldDir), 1.f));
        }
        if (m_pGameInstance->Get_DIKeyState(DIK_X) == KEYSTATE::PRESS)
        {
            switch (m_eMenu)
            {
            case Editor::CLevel_Map::MENU_OBJECT:
            {
                if (event.fDistance <= m_fNearDistance)
                {
                    if (m_pPickedObject)
                        m_pPickedObject->Set_ShaderPass(0);

                    m_pPickedObject = dynamic_cast<CEdit_MapObject*>(reinterpret_cast<CGameObject*>(event.pObject));
                    m_pPickedObject->Set_ShaderPass(3);

                    if (m_pChildObject)
                    {
                        m_pPickedObject->Add_Child(m_pChildObject);
                        m_pChildObject = nullptr;
                    }
                }
            }
            break;

            case Editor::CLevel_Map::MENU_RANDSCAPE:
                if (event.fDistance <= m_fNearDistance_Instance)
                {
                    m_fNearDistance_Instance = event.fDistance;
                    m_pPickedInstanceObject = dynamic_cast<CEdit_MapObject_Instance*>(reinterpret_cast<CGameObject*>(event.pObject));
                }
                break;

            case Editor::CLevel_Map::MENU_LIGHT:
                int a = 0;
                break;
            }
        }
        });
    m_pGameInstance->Subscribe<MAP_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), [this](const MAP_CREATE& event) {
        CGameObject* pObject = reinterpret_cast<CGameObject*>(event.pObject);
        m_SaveObjects[event.ModelName].push_back(pObject);
        Safe_AddRef(pObject);

        m_pPickedObject = dynamic_cast<CEdit_MapObject*>(reinterpret_cast<CGameObject*>(event.pObject));
        });

    m_pGameInstance->Subscribe<MAP_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Set_Parent"), [this](const MAP_CREATE& event) {
        if (!m_pChildObject)
            m_pChildObject = reinterpret_cast<CEdit_MapObject*>(event.pObject);
        else
            m_pChildObject = nullptr;
        });
}

void CLevel_Map::Make_MousePos()
{
    POINT ptMousePos = m_pGameInstance->Get_MousePoint();
    //酉고룷?몄뿉???ъ쁺?ㅽ럹?댁뒪濡???린湲? => -1~ 1濡?蹂??
    _float3 vMousePos{};
    vMousePos.x = ptMousePos.x / (g_iWinSizeX * 0.5f) - 1.f;
    vMousePos.y = -1 * ptMousePos.y / (g_iWinSizeY * 0.5f) + 1.f;
    vMousePos.z = 0.f;

    //酉곗뒪?섏씠?ㅻ줈 ?꾪솚???꾪븳 ?ъ쁺 ?됰젹 ?섎늻湲?
    XMStoreFloat3(&vMousePos, XMVector3TransformCoord(XMLoadFloat3(&vMousePos), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ)));

    //酉??ㅽ럹?댁뒪 湲곗? 留덉슦???덉씠, ?쒖옉 ?꾩튂 怨꾩궛.
    m_vWorldPos = {};
    m_vWorldDir = vMousePos;

    //酉??ㅽ럹?댁뒪?먯꽌 ?붾뱶 留ㅽ듃由?뒪 ?꾪솚.
    XMStoreFloat3(&m_vWorldPos, XMVector3TransformCoord(XMLoadFloat3(&m_vWorldPos), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW)));
    XMStoreFloat3(&m_vWorldDir, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&m_vWorldDir), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW))));
}

void CLevel_Map::Container_Info()
{
    ImGuiID ShaderId = ImGui::GetID("Container");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("Current Container");

    for (auto& pContainer : m_ContainerObjects)
        if (ImGui::Button(pContainer.second->Get_ModelName())) {
            int a = 0;
        }
    ImGui::EndChildFrame();
}

CLevel_Map* CLevel_Map::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Map* pInstance = new CLevel_Map(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Map");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Map::Free()
{
    __super::Free();
    m_pPickedObject = nullptr;
    m_pPickedInstanceObject = nullptr;
    m_pPickedLightObject = nullptr;
    m_pGameInstance->Unscribe();

    Safe_Release(m_pPreViewObject);
    Safe_Release(m_pBrush);

    for (auto& Pair : m_SaveObjects)
    {
        for (auto& pGameObject : Pair.second)
            Safe_Release(pGameObject);
        Pair.second.clear();
    }
    
    m_SaveObjects.clear();
}
