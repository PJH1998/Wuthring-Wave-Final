#include "EditorPch.h"
#include "Level_Map.h"

#include "Event_Level.h"
#include"Model_Instance.h"
#include"MapObject.h"
#include"Mesh_Instance.h"
#include"MapObject_Instance.h"

CLevel_Map::CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Map::Initialize()
{
    Ready_Event();

    if (FAILED(Ready_Static_Component()))
        return E_FAIL;

    ImGui::GetIO().DisplayFramebufferScale = ImVec2(1.25f, 1.25f);
    return S_OK;
}

void CLevel_Map::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Map"));
    Menu_Select();

    switch (m_eMenu)
    {
    case Editor::CLevel_Map::MENU_OBJECT:
        Menu_Object();
        break;

    case Editor::CLevel_Map::MENU_RANDSCAPE:
        Menu_RandSacpe();
        break;

    case Editor::CLevel_Map::MENU_LIGHT:
        Menu_Light();
        break;

    case Editor::CLevel_Map::MENU_SAVELOAD:
        Menu_Save_Load();
        break;

    }
}

void CLevel_Map::Render()
{

}

void CLevel_Map::Menu_Select()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::MenuItem("Ojbect")) {
            m_eMenu = MENU_OBJECT;
        }

        if (ImGui::MenuItem("RandScape")) {
            m_eMenu = MENU_RANDSCAPE;
        }

        if (ImGui::MenuItem("Light")) {
            m_eMenu = MENU_LIGHT;
        }

        if(ImGui::MenuItem("Save & Load")) {
            m_eMenu = MENU_SAVELOAD;
        }
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
    ImGui::End();
}

void CLevel_Map::Menu_Light()
{
    // 조명. 일단 Imgui에 List로 현재 내가 넣은 조명들 정보? 순서 띄우기. 버튼형식으로 누르면 그 조명에 대한 정보가 나오게.
       // 라이트 오브젝트를 하나 만들어서 그 놈의 위치 정보를 조명으로. 조절할 수 있게. -> 라이트 객체가 현재 추가된 조명들 중에서 몇 번째 순서인지
       // 각종 색상정보 및 세기, 디퓨즈 앰비언트 기타 등등 다 수정할 수 있게. -> 실시간 적용? or 버튼 누르면 적용. 되돌리기 기능도 있음 좋을듯
       // 
       //기즈모 달거면 조명에 달기. 

}

void CLevel_Map::Menu_Save_Load()
{
    IGFD::FileDialogConfig config;

    config.path = "../../Client/Bin/Resource/";
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

                ifstream File(ModelPath, ios::binary);
                //ifstream File(strFilePath, ios::binary);

                if (!File.is_open())
                {
                    MSG_BOX("Load Failed");
                }

                _uint NameLength;

                _matrix PreTransformMatrix = XMMatrixIdentity();
                _float fSize = 0.001f;
                PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.0f));

                CMapObject::MAP_LOAD Desc{};

                while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
                {
                    File.read(Desc.ModelName, NameLength);
                    
                    File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
                    _float4x4 Matrix = {};
                    File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
                    Desc.WorldMatrix = &Matrix;

                    _tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
                    _tchar Name[MAX_PATH] = {};
                    MultiByteToWideChar(CP_ACP, 0, Desc.ModelName, -1, Name, strlen(Desc.ModelName));
                    lstrcat(Model, Name);

                    _char ModelPath[MAX_PATH] = "../../Client/Bin/Resource/";
                    strcat_s(ModelPath, Desc.ModelName);
                    strcat_s(ModelPath, "/");
                    strcat_s(ModelPath, Desc.ModelName);
                    strcat_s(ModelPath, ".dat");

                    //m_pGameInstance->Add_Prototype(m_iLevel, Model,
                    //    CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Test1/Test1.dat"));

                    _tchar PrototypeObject[MAX_PATH] = TEXT("Prototype_GameObject_MapObject_");
                    lstrcat(PrototypeObject, Name);

                    //m_pGameInstance->Add_Prototype(m_iLevel, PrototypeObject,
                    //    CMapObject::Create(m_pDevice, m_pContext));

                    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, PrototypeObject
                        , m_iLevel, TEXT("Layer_Test"), &Desc);

                }
                File.close();
                //레이어나 오브젝트매니저 전체 순회가능한 함수 생기면 변경 고려 해볼것.
            }
        }
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
    
    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf_Instance"),
        CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Wolf"), 
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat"));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Test"),
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Test/Test.dat"));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Model_Test1"),
        CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Test1/Test1.dat"));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance_Wolf"),
        CMapObject_Instance::Create(m_pDevice, m_pContext));

    //m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Wolf"),
    //    CMapObject::Create(m_pDevice, m_pContext));

    //m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Test"),
    //    CMapObject::Create(m_pDevice, m_pContext));


    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Test1"),
        CMapObject::Create(m_pDevice, m_pContext));


    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
     CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements));

    //VTXMESHINSTANCE
    

    //오브젝트매니저에서 레이어 전부 돌면서 순차적으로 저장.
    //LOD 개수 LOD0, LOD1, LOD2같이 LOD 수도 저장??

    //큐브 안에 모델 찍기 / 월드 최대 크기 안에 찍어야한다.
    //일단 텍스쳐 없이 모델만 로드해놓기 세이브 & 로드.
    
    CMapObject::MAP_LOAD Desc{};
    CMapObject_Instance::MAP_LOAD InstanceDesc{};
    _float4x4 DefaultMatrix{};
    XMStoreFloat4x4(&DefaultMatrix, XMMatrixIdentity());
    InstanceDesc.WorldMatrix = Desc.WorldMatrix = &DefaultMatrix;

    strcpy_s(InstanceDesc.ModelName, "Wolf_Instance");
    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance_Wolf")
        , m_iLevel, TEXT("Layer_Test"), &InstanceDesc);

    /*strcpy_s(Desc.ModelName, "Wolf");
    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Wolf")
        , m_iLevel, TEXT("Layer_Test"), Desc.ModelName);*/

    /*strcpy_s(Desc.ModelName, "Test");
    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Test")
        , m_iLevel, TEXT("Layer_Test"), &Desc.ModelName);*/

    strcpy_s(Desc.ModelName, "Test1");
    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Test1")
        , m_iLevel, TEXT("Layer_Test"), &Desc);

    return S_OK;
}

void CLevel_Map::Ready_Event()
{
    m_pGameInstance->Subscribe<MAP_PICK>(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), [this](const MAP_PICK& event) {
        if (CMapObject* pObject = dynamic_cast<CMapObject*>(reinterpret_cast<CGameObject*>(event.pObject)))
            m_pPickedObject = pObject;
        else if (CMapObject_Instance* pObject = dynamic_cast<CMapObject_Instance*>(reinterpret_cast<CGameObject*>(event.pObject)))
            m_pPickedInstanceObject = pObject;
        });
    m_pGameInstance->Subscribe<MAP_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), [this](const MAP_CREATE& event) {
        CGameObject* pObject = reinterpret_cast<CGameObject*>(event.pObject);
        m_SaveObjects[event.ModelName].push_back(pObject);
        Safe_AddRef(pObject);
        });
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
    m_pGameInstance->Unscribe();

    for (auto& Pair : m_SaveObjects)
    {
        for (auto& pGameObject : Pair.second)
            Safe_Release(pGameObject);
        Pair.second.clear();
    }
    
    m_SaveObjects.clear();
}
