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
    // 랜드스케이프를 위해 메쉬 위에 브러쉬 만드는 메모
    // 뎁스 타겟을 갖고와서 바인딩. 월드 위치로 변환.
    // 내 마우스 위치 점 하나 VS_IN으로 보내고, Range 변수 셰이더 전달.
    // 뎁스 타겟의 w값이 0이면 discard
    // GS셰이더에서 사각 버퍼 생성, 점 기준으로 원형 브러쉬 생성? => 사각할 건지 원형 할 건지 변수 전달?
    // 범위, Y축 기준 회전 랜덤수치, 개수, 색상..? 입력 가능하게 ?
    
    // 버튼 누르면 생성할 수 있게?
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

        if(ImGui::MenuItem("Object Save & Load")) {
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

    //레이어나 오브젝트매니저에서 오브젝트 포인터 갖고오는 거 되면 피킹 말고 BeginChildFrame으로 또 선택해도 될듯.

    if (m_pPickedObject)
        m_pPickedObject->Set_ImGuiOption();

    ImGui::End();
}

void CLevel_Map::Menu_RandSacpe()
{
    ImGui::Begin("Menu_RandScape");

    if (m_pPickedInstanceObject)
        m_pPickedInstanceObject->Set_ImGuiOption();
#pragma region 랜드스케이프 메모
    //풀떼기들은 플레이어랑 가까이 있을 때 플레이어를 중점으로 옆으로 누움. 누운 상태로 바람에 흔들림.
    //플레이어랑 거의 겹친 풀떼기들은 Clip되는듯. 안보임.
    //움직일 때 플레이어 발바닥에 발자국 데칼 생김. 마스킹 이미지 같은 거로 하는듯?
    //그림자 진 곳이든 아닌 곳이든 똑같이 어두움. 무조건 마스킹.

    // 점프는 발자국은 안생기지만 뛸 때와 착지할 때 풀떼기가 심하게 흔들림.(어떻게 함?)
    //벽에서 달리기 할 때 발 위치에 발자국 데칼 대신 이펙트가 생김.

    //맵에 깔려있는 아이템을 먹을 때는 바닥에 나뭇잎 흔들리는 이펙트 생기면서 사라짐.
    //근처에 먹을 수 있는(상호작용 가능한 아이템이 있으면 UI 생성. 일정 주기마다 겉부분이 빛남.
    //바닥 풀떼기 말고 키 큰 풀떼기들이랑 몸 비빌 때 소리 나야함.(콜라이더?) 얘네도 똑같이 플레이어 위치에 맞춰서 흔들리는듯.

    //위치에 따라 디렉셔널라이트 디퓨즈 색이 바뀌는듯? -> 그냥 메쉬가 다른 거일 수도

    //바람에 흔들리는 방향은 모두 같은 방향인?듯 

    //돌은 ㅋㅋ 그냥 에셋스토어에서 떼온듯ㅋㅋ
    //특정 위치에 따라 풀떼기의 색이 ㅈ금씩 바뀜.
    //광물류는 멀리 있으면 빤짝빤짝댐.


    //길찾기. 가만히 있으면 목표 위치로 일렁이는 이펙트 생기면서 길 알려줌. 무조건 1자가 아니라 좌우로 쪼끔씩 흔들리는 이펙트인듯.
    //거리가 좀 있으면 안개가 살짝 깔리는 맵도 있는 거 같음.
#pragma endregion
    ImGui::End();
}

void CLevel_Map::Menu_Light()
{
    // 조명. 일단 Imgui에 List로 현재 내가 넣은 조명들 정보? 순서 띄우기. 버튼형식으로 누르면 그 조명에 대한 정보가 나오게.
       // 라이트 오브젝트를 하나 만들어서 그 놈의 위치 정보를 조명으로. 조절할 수 있게. -> 라이트 객체가 현재 추가된 조명들 중에서 몇 번째 순서인지
       // 각종 색상정보 및 세기, 디퓨즈 앰비언트 기타 등등 다 수정할 수 있게. -> 실시간 적용? or 버튼 누르면 적용. 되돌리기 기능도 있음 좋을듯
       // 점조명에는 그림자 없음.
       //기즈모 달거면 조명에 달기. 
    

    //이샛기 누르면 왜 똥 생김?
}

void CLevel_Map::Menu_Model_Load()
{
    //클릭 하기 전까지 마우스 위치 따라다니기?.
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

    static _char exportText[128] = ""; // 입력 저장용 버퍼
    ImGui::InputText("파일 이름", exportText, IM_ARRAYSIZE(exportText));

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
        //상호작용할 객체들 따로, 인스턴싱 객체들 따로, 일반 맵 따로.

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

                    _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
                    _tchar Name[MAX_PATH] = {};
                    MultiByteToWideChar(CP_ACP, 0, Desc.ModelName, -1, Name, strlen(Desc.ModelName));
                    lstrcat(Model, Name);

                    _char ModelPath[MAX_PATH] = "../../Client/Bin/Resource/Map/";
                    strcat_s(ModelPath, Desc.ModelName);
                    strcat_s(ModelPath, "/");
                    strcat_s(ModelPath, Desc.ModelName);
                    strcat_s(ModelPath, ".dat");

                    m_pGameInstance->Add_Prototype(m_iLevel, Model,
                        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixIdentity(), ModelPath));
                    /*m_pGameInstance->Add_Prototype(m_iLevel, Model,
                        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, ModelPath));
                    */
                    _tchar PrototypeObject[MAX_PATH] = TEXT("Prototype_GameObject_MapObject_");
                    lstrcat(PrototypeObject, Name);

                    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, PrototypeObject
                        , m_iLevel, TEXT("Layer_Test"), &Desc);

                }
                File.close();
                //레이어나 오브젝트매니저 전체 순회가능한 함수 생기면 변경 고려 해볼것.


                //LOD를 카메라 거리 기반으로 하지 말고, 모델의 최소 최대 픽셀로 큐브를 만들었을 때 그 큐브가
                //현재 화면을 기준으로 픽셀을 얼마나 많이 차지하고 있나로 LOD 단계 구별하기. => 스크린 픽셀 사이즈 기법
                //LOD 모델은 상태머신을 갈아끼우듯 LOD 단계에 따라 바꾸기. => 어차피 모델의 크기는 변하지 않음. 디테일이 달라짐.
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
    _float fSize = 0.01f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

    for (const auto& entry : filesystem::recursive_directory_iterator(FolderPath)) {
        if (entry.is_regular_file()) {
            if (entry.path().string().find("MapData") != std::string::npos)
                continue;

            //LOD 모델들은 목록에 추가하지 말고 _LOD0 이름 빼고 1개씩만 저장하게.
            if (entry.path().extension() == ".dat") {
                //m_ModelPaths.push_back(entry.path().string());

                //여기에 프로토타입 미리 생성
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                _wstring ProtoModelPath = TEXT("Prototype_Component_Model_");
                _wstring  ProtoModelName = ProtoModelPath + StringToWString(FileName);
                _wstring  PushName = ProtoModelPath + StringToWString(FileName);
                PushName.pop_back();
                //멀티 쓰레드 쓸 때 중단점 걸면 터지니까 걸지마쇼

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
                   CRASH("Prototype Create Failed");

                    //멀티쓰레드 정상화 되면 이거 쓸것.
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


    //일반 모델
    //m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf"), CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));

    //인스턴스 모델
    
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

    //오브젝트매니저에서 레이어 전부 돌면서 순차적으로 저장.
    //LOD 개수 LOD0, LOD1, LOD2같이 LOD 수도 저장??

    //큐브 안에 모델 찍기 / 월드 최대 크기 안에 찍어야한다.
    //일단 텍스쳐 없이 모델만 로드해놓기 세이브 & 로드.
    
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
    //뷰포트에서 투영스페이스로 옮기기. => -1~ 1로 변환.
    _float3 vMousePos{};
    vMousePos.x = ptMousePos.x / (g_iWinSizeX * 0.5f) - 1.f;
    vMousePos.y = -1 * ptMousePos.y / (g_iWinSizeY * 0.5f) + 1.f;
    vMousePos.z = 0.f;

    //뷰스페이스로 전환을 위한 투영 행렬 나누기
    XMStoreFloat3(&vMousePos, XMVector3TransformCoord(XMLoadFloat3(&vMousePos), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ)));

    //뷰 스페이스 기준 마우스 레이, 시작 위치 계산.
    m_vWorldPos = {};
    m_vWorldDir = vMousePos;

    //뷰 스페이스에서 월드 매트릭스 전환.
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

    for (auto& Pair : m_SaveObjects)
    {
        for (auto& pGameObject : Pair.second)
            Safe_Release(pGameObject);
        Pair.second.clear();
    }
    
    m_SaveObjects.clear();
}
