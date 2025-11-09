#include "EditorPch.h"
#include "Level_Map.h"

#include "Event_Level.h"
#include"Edit_MapObject.h"
#include"Edit_MapObject_Instance.h"
#include"Edit_PreViewModel.h"
#include"Edit_LightObject.h"
#include"Edit_Brush.h"
#include"Shader_Interface.h"
#include"AnimationTool.h"
#include"Edit_MapObject_Destruction.h"
#include"Edit_MapObject_Destruction_Piece.h"
#include"Edit_TriggerBox.h"
#include"Mesh_Instance.h"

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

	//m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4096, 4096, 4096));

	//ImGui::GetIO().DisplayFramebufferScale = ImVec2(1.25f, 1.25f);
	pShaderInterface = CShader_Interface::Create(m_pDevice, m_pContext);

	LEVEL m_eCurLevel = LEVEL::MAP;
	//m_pAnimationTool = CAnimationTool::Create(m_pDevice, m_pContext, m_eCurLevel);

	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
	//    CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
	//        , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	//{
	//    CRASH("Failed Load AnimMesh Shader");
	//    return E_FAIL;
	//}

	//SHADER_MACRO eShaderMacro = {
	//    {"THREAD_X", "64" }
	//    ,{"THREAD_Y", "1" }
	//    ,{"THREAD_Z", "1" }
	//    , { NULL, NULL }
	//};

	//string strEntryPoint = "CSMain";
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
	//    CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
	//        , eShaderMacro, strEntryPoint))))
	//{
	//    CRASH("Failed Load AnimMesh Shader");
	//    return E_FAIL;
	//}


	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
	//    CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
	//        , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	//{
	//    CRASH("Failed Load AnimMesh Shader");
	//    return E_FAIL;
	//}

	SHADER_MACRO eShaderMacro = {
		{"THREAD_X", "64" }
		,{"THREAD_Y", "1" }
		,{"THREAD_Z", "1" }
		, { NULL, NULL }
	};

	string strEntryPoint = "CSMain";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
			, eShaderMacro, strEntryPoint))))
	{
		CRASH("Failed Load AnimMesh Shader");
		return E_FAIL;
	}

	m_eObjectType = ENUM_CLASS(OBJECTTYPE::DEFAULT);

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_TriggerBox"),
		CEdit_TriggerBox::Create(m_pDevice, m_pContext));

	//CEdit_TriggerBox::TRIGGER Tri;
	//Tri.iLevel = m_iLevel;
	//Tri.vExtends = _float3(20.f, 20.f, 20.f);
	//_matrix Mat = XMMatrixIdentity();
	//_float4x4 TT;
	//XMStoreFloat4x4(&TT, Mat);
	//Tri.WorldMatrix = &TT;
	//m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_TriggerBox"), m_iLevel, TEXT("Layer_Trigger"), &Tri);
	return S_OK;
}

void CLevel_Map::Update(_float fTimeDelta)
{
    m_fNearDistance = FLT_MAX;
    m_fNearDistance_Instance = FLT_MAX;

    SetWindowText(g_hWnd, TEXT("Map"));
    Menu_Select();

    switch (m_eMenu)
    {
    case Editor::CLevel_Map::MENU_OBJECT:
        Menu_Object();
        break;

    case Editor::CLevel_Map::MENU_RANDSCAPE:
        m_pBrush->Priority_Update(fTimeDelta);
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
    case Editor::CLevel_Map::MENU_OBJECTTYPE:
        Menu_Object_Type();
        break;
        
    }

	if (m_pGameInstance->Get_DIKeyState(DIK_U) == KEYSTATE::DOWN)
	{
		_vector Min = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		_vector Max = XMVectorSet(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);
		MAP_BOUND event(&Min, &Max);
		m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Calc_Size"), event);

		int a = 0;
	}

    Make_MousePos();
    pShaderInterface->Update_Shadow();
    if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE) == KEYSTATE::PRESS && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
        m_pPickedObject = nullptr;
}

void CLevel_Map::Render()
{
    //m_pAnimationTool->Render();
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
        if (ImGui::MenuItem("Object Type")) {
            m_eMenu == MENU_OBJECTTYPE ? m_eMenu = END : m_eMenu = MENU_OBJECTTYPE;
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

	if (m_eObjectType != static_cast<_uint>(OBJECTTYPE::TRIGGERBOX))
	{
		if (m_pPickedObject)
			m_pPickedObject->Set_ImGuiOption();
		else if (m_pPickedDestructObject)
			m_pPickedDestructObject->Set_ImGuiOption();
	}
	else
	{
		
		ImGui::Text("Current Triggers");

		ImGuiID ShaderId = ImGui::GetID("TriggerBox");
		ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));

		for (auto& pContainer : m_ContainerObjects)
			if (ImGui::Button(pContainer.second->Get_ModelName())) {
				int a = 0;
			}
		for (_uint i=0; i< m_SaveObjects["Map_Object_TriggerBox"].size();++i)
		{
			if (ImGui::Button(to_string(i).c_str())) {
				m_pPickedTriggerBox = dynamic_cast<CEdit_TriggerBox*>(m_SaveObjects["Map_Object_TriggerBox"][i]);
			}
		}
		ImGui::EndChildFrame();
		
		if (m_pPickedTriggerBox)
			m_pPickedTriggerBox->Set_ImGuiOption();

		Create_TriggerBox();
	}

    ImGui::End();
}

void CLevel_Map::Menu_RandSacpe()
{
    ImGui::Begin("Menu_RandScape");


    Load_Foliage();
    ImGui::Button("Undo");
    if (!m_SaveObjects["Map_Object_Instance"].empty())
    {
        if (ImGui::IsItemHovered() && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
        {

            CEdit_MapObject_Instance* pObject = dynamic_cast<CEdit_MapObject_Instance*>(m_SaveObjects["Map_Object_Instance"].back());
            pObject->SetActivate(false);
            Safe_Release(pObject);
            m_SaveObjects["Map_Object_Instance"].pop_back();
        }
    }

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
				if (static_cast<OBJECTTYPE>(m_eObjectType) == OBJECTTYPE::DESTRUCTION)
				{
					_string Name = FileName;
					if ((Name.find("Roc_24BS") == string::npos) && (Name.find("Roc_28BS") == string::npos))
						continue;

					CEdit_MapObject_Destruction::MAP_LOAD Desc{};
					_float4x4 DefaultMatrix{};
					XMStoreFloat4x4(&DefaultMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&m_vPickedPos)));
					Desc.WorldMatrix = &DefaultMatrix;
					strcpy_s(Desc.ModelName, FileName);
					Desc.eObjectType = static_cast<OBJECTTYPE>(m_eObjectType);
					Desc.iLevel = m_iLevel;
					m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Destruction")
						, m_iLevel, TEXT("Layer_MapObject_Destruction"), &Desc);
				}
				else
				{
					CEdit_MapObject::MAP_LOAD Desc{};
					_float4x4 DefaultMatrix{};
					XMStoreFloat4x4(&DefaultMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&m_vPickedPos)));
					Desc.WorldMatrix = &DefaultMatrix;
					strcpy_s(Desc.ModelName, FileName);
					Desc.eObjectType = static_cast<OBJECTTYPE>(m_eObjectType);
					Desc.iLevel = m_iLevel;

					m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
						, m_iLevel, TEXT("Layer_MapObject"), &Desc);
				}
			}

            if (ImGui::IsItemHovered())
            {
                ImGui::Begin("PreView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
                m_szPreViewModelName = StringToWString(FileName);
#ifdef _DEBUG
                ImGui::Image(m_pGameInstance->Get_Debug_RT_Resource(TEXT("RT_Debug")), ImVec2(128, 128));
#endif
                ImGui::End();
                m_pPreViewObject->Late_Update(0.016f, m_szPreViewModelName);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void CLevel_Map::Menu_Object_Type()
{
    ImGui::Begin("Type");

	const _char* pObejceTType[] = { "Default","Sonoro","InterAction","MonsterSpawn","Destruction","NonRigid","TriggerBox" ,"NonSonoro","Sonoro_Floor" };
    if (ImGui::BeginCombo("Object_Type", pObejceTType[m_eObjectType]))
    {
        for (_uint i = 0; i < ENUM_CLASS(OBJECTTYPE::END); ++i)
        {
            if (ImGui::Selectable(pObejceTType[i]))
            {
                m_eObjectType = static_cast<_uint>(i);
            }
        }
        ImGui::EndCombo();
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
                MapName += "/";
                if (filesystem::exists(MapName))
                    filesystem::create_directories(MapName);
                MapName += exportText;
                MapName += "_";
                MapName += Pair.first;
                MapName += ".dat";
                unordered_set<_string> Test;
                ofstream File(MapName, ios::binary);

                MAP_SAVE event(File, Test);
                //if (Pair.first.find("Instance") != std::string::npos)
                //    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Instance"), event);
                //else if(Pair.first.find("Destruction") != std::string::npos)
                //    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Destruction"), event);
				//else
				//	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), event);

                File.close();
            }
        //?곹샇?묒슜??媛앹껜???곕줈, ?몄뒪?댁떛 媛앹껜???곕줈, ?쇰컲 留??곕줈.

        ImGui::EndMenu();
    }
	if (ImGui::BeginMenu("Save All"))
	{
		if (ImGui::MenuItem("Save All Check"))
		{
			unordered_set<_string> UsingPrototypeNames;
			for (auto& Pair : m_SaveObjects)
			{
				string MapName = config.path;
				MapName += exportText;
				MapName += "/";
				if (!filesystem::exists(MapName))
					filesystem::create_directories(MapName);
				MapName += exportText;
				MapName += "_";
				MapName += Pair.first;
				MapName += ".dat";
				ofstream File(MapName, ios::binary);

				MAP_SAVE event(File, UsingPrototypeNames);
				if (Pair.first.find("Instance") != std::string::npos)
					m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Instance"), event);
				else if (Pair.first.find("Destruction") != std::string::npos)
					m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Destruction"), event);
				else if (Pair.first.find("Trigger") != std::string::npos)
					m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Trigger"), event);
				else
					m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), event);
				File.flush();
				File.close();
			}
			_string PrototypeSave = config.path;
			PrototypeSave += exportText;
			PrototypeSave += "/";
			if (!filesystem::exists(PrototypeSave))
				filesystem::create_directories(PrototypeSave);
			PrototypeSave += exportText;
			PrototypeSave += "_";
			_string PrototypeSaveNames = PrototypeSave;
			PrototypeSaveNames += ".txt";
			PrototypeSave += "_Prototype.dat";
			ofstream File2(PrototypeSave, ios::binary);
			ofstream File3(PrototypeSaveNames, ios::trunc);

			_uint i = 0;
			for (const auto& Data : UsingPrototypeNames)
			{
				//_uint i = strlen(Data.c_str());
				_uint StrSize = static_cast<_uint>(strlen(Data.c_str()));
				File2.write(reinterpret_cast<const _char*>(&StrSize), sizeof(_uint));
				File2.write(reinterpret_cast<const _char*>(Data.c_str()), StrSize);
				File3 << Data.c_str() << endl;
				i++;
			}
			File2.flush();
			File2.close();
			File3 << i << endl;
			File3.close();
		}
		ImGui::EndMenu();
	}


    ImGui::MenuItem("Load", nullptr, &m_LoadMenu);
    if (m_LoadMenu)
    {
        ImGui::Begin("Map Save & Load");

        ImGuiFileDialog::Instance()->OpenDialog("Map File Load", "Import File", ".dat", config);

        if (ImGuiFileDialog::Instance()->Display("Map File Load")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                _string DatFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
                for (const auto& entry : filesystem::recursive_directory_iterator(DatFolderPath)) {
                    if (entry.is_regular_file())
                    {
						_uint NameLength = {};

                        _string strFilePath = entry.path().string();
                        if (strFilePath.find("Prototype") != std::string::npos)
                            continue;
                        if (strFilePath.find(".txt") != std::string::npos)
                            continue;
                        ifstream File(strFilePath, ios::binary);

                        if (!File.is_open())
                        {
                            MSG_BOX("Load Failed");
                        }

						if (strFilePath.find("Instance") != std::string::npos)
						{

							_matrix PreTransformMatrix = XMMatrixIdentity();
							_float fSize = 0.01f;
							PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

							CEdit_MapObject_Instance::MAP_LOAD Desc{};

							while (File.read(reinterpret_cast<char*>(&Desc.iSaveIndex), sizeof(_uint)))
							{
								File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
								memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
								File.read(Desc.ModelName, NameLength);

								File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
								if(Desc.iShaderPassIndex == 2)
									File.read(reinterpret_cast<char*>(&Desc.vDiffuseColor), sizeof(_float4));

								File.read(reinterpret_cast<char*>(&Desc.iNumInstance), sizeof(_uint));

								_float4x4* InstanceMatrix = new _float4x4[Desc.iNumInstance];

								File.read(reinterpret_cast<char*>(InstanceMatrix), sizeof(_float4x4) * Desc.iNumInstance);
								Desc.InstanceWorldMatrix = InstanceMatrix;

								File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

								_float3 vBoundingPos;
								_float3 vBoundingExtends;
								File.read(reinterpret_cast<char*>(&vBoundingPos), sizeof(_float3));
								File.read(reinterpret_cast<char*>(&vBoundingExtends), sizeof(_float3));
								Desc.IsLoaded = true;

								//이거를 프로토타입으로 만든 이후 바로 클론하기.

								CMesh_Instance::MESH_INST_DESC MeshDesc{};
								MeshDesc.iNumInstance = Desc.iNumInstance;
								MeshDesc.pTransformMatrix = InstanceMatrix;
								//파일시스템으로 해당 모델 찾기.
								_string ModelPath = Desc.ModelName;
								ModelPath.pop_back();
								for (const auto& entry : filesystem::recursive_directory_iterator(m_FolderPath)) {
									if (entry.is_regular_file()) {
										if (entry.path().string().find("Foliage") == std::string::npos)
											continue;

										if (entry.path().string().find(ModelPath) == std::string::npos)
											continue;

										if (entry.path().extension() != ".dat")
											continue;

										_char FileDrive[MAX_PATH] = {};
										_char FileDir[MAX_PATH] = {};
										_char FileName[MAX_PATH] = {};
										_char FileExt[MAX_PATH] = {};
										_splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);


										_wstring PrototypeName = L"Prototype_Component_Model_Instance_";
										_wstring ModelName = StringToWString(FileName) + to_wstring(Desc.iSaveIndex);
										PrototypeName += ModelName;

										_string VersionPath = FileDir;
										VersionPath += FileName;
										VersionPath += ".dat";
										if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, PrototypeName,
											CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, VersionPath.c_str(), false, &MeshDesc))))
											CRASH("Prototype Create Failed");

										memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
										strcpy_s(Desc.ModelName, WStringToString(ModelName).c_str());
									}
								}
								m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance")
									, m_iLevel, TEXT("Layer_Instance"), &Desc);
								Safe_Delete_Array(InstanceMatrix);
							}
						}
						else if (strFilePath.find("Destruction") != std::string::npos)
						{
							//continue;

							_matrix PreTransformMatrix = XMMatrixIdentity();
							_float fSize = 0.01f;
							PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

							CEdit_MapObject_Destruction::MAP_LOAD Desc{};
	
							while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
							{
								memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
								File.read(Desc.ModelName, NameLength);
								_string Name = Desc.ModelName;

								File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
								File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
								_float4x4 Matrix = {};
								File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
								Desc.WorldMatrix = &Matrix;
								Desc.iLevel = m_iLevel;

								File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
								File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

								File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePos), sizeof(_float3));
								File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePower), sizeof(_float3));

								File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint));
								m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Destruction")
									, m_iLevel, TEXT("Layer_Test"), &Desc);
							}
						}
						else if (strFilePath.find("TriggerBox") != std::string::npos)
						{
							_uint iTriggerIndex;
							CEdit_TriggerBox::TRIGGER Desc{};
							while (File.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(_uint)))
							{
								File.read(reinterpret_cast<char*>(&Desc.vExtends), sizeof(_float3));
								_float4x4 Matrix = {};
								File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
								Desc.WorldMatrix = &Matrix;
								m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_TriggerBox")
									, m_iLevel, TEXT("Layer_Test"), &Desc);
							}
						}
                        else
                        {

                            CEdit_MapObject::MAP_LOAD Desc{};

							while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
							{
								memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
								File.read(Desc.ModelName, NameLength);
								_string Name = Desc.ModelName;

								File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
								File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
								_float4x4 Matrix = {};
								File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
								Desc.WorldMatrix = &Matrix;
								File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
								File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));


								m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
									CEdit_MapObject::MAP_LOAD pDesc{};
									strcpy_s(pDesc.ModelName, ModelName.c_str());
									pDesc.iShaderPassIndex = ShaderPass;
									pDesc.eObjectType = eObjectType;
									pDesc.WorldMatrix = &Matrix;
									pDesc.iLevel = m_iLevel;

									m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
										, m_iLevel, TEXT("Layer_Test"), &pDesc);
									});

							}
                        }
						m_pGameInstance->Wait_Thread_End();
						File.close();

                    }
                }
                m_LoadMenu = !m_LoadMenu;
                ImGuiFileDialog::Instance()->Close();
            }
            else
            {
                m_LoadMenu = !m_LoadMenu;
                ImGuiFileDialog::Instance()->Close();
            }
        }
        ImGui::End();
    }
}

void CLevel_Map::Load_Objects()
{
    m_ModelPaths.clear();

    m_pPreViewObject = CEdit_PreViewModel::Create(m_pDevice, m_pContext);
	//m_FolderPath = "../../Client/Bin/Resource/Map/Asphodel_Barrens/";
	//m_FolderPath= "../../Client/Bin/Resource/Map/Test/";
	m_FolderPath = "../../Client/Bin/Resource/Map/The_False_Sovereign/";
	//m_FolderPath= "../../Client/Bin/Resource/Map/The_False_Sovereign/Sonoro/";
	//m_FolderPath= "../../Client/Bin/Resource/Map/";

    vector<_wstring> m_PrototypeNames;
    vector<_wstring> m_FoliageNames;

    _matrix PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.01f;
    //_float fSize = 0.02f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

    _int version={};
    _int Lastversion = {};
    _wstring LastVersionName;
    _string LastVersionPath;

    //마지막 폴더 못읽음. 프로토타입 안생김.
	for (const auto& entry : filesystem::recursive_directory_iterator(m_FolderPath)) {
		if (entry.is_regular_file()) {
			if (entry.path().string().find("MapData") != std::string::npos)
				continue;

			if (entry.path().string().find("Anim") != std::string::npos)
				continue;

			if (entry.path().extension() == ".dat") {

				_char FileDrive[MAX_PATH] = {};
				_char FileDir[MAX_PATH] = {};
				_char FileName[MAX_PATH] = {};
				_char FileExt[MAX_PATH] = {};
				_splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

				_wstring PrototypeName = L"Prototype_Component_Model_";
				PrototypeName += StringToWString(FileName);

				_string VersionPath = FileDir;
				VersionPath += FileName;
				VersionPath += ".dat";

				if (entry.path().string().find("_Bone") != std::string::npos)
				{
					m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, Path = VersionPath]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel::Create(m_pDevice, m_pContext, MODELTYPE::ECO, PreTransformMatrix, Path.c_str()))))
							CRASH("Prototype Create Failed");
						});
					continue;
				}

				_wstring baseName = StringToWString(FileName);

				// LOD 마지막에 붙은 숫자 추출
				size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
				_wstring namePart = baseName.substr(0, pos + 1);

				_wstring numberPart = baseName.substr(pos + 1);
				version = stoi(numberPart);

				_wstring key = L"Prototype_Component_Model_" + namePart;




				m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, Path = VersionPath]() {
					if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
						CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, Path.c_str()))))
						CRASH("Prototype Create Failed");
					});

				if (entry.path().string().find("Foliage") != std::string::npos)
				{
					_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
					ProtoName += StringToWString(FileName);

					m_pGameInstance->Add_Work([&, ProtoName = ProtoName, Path = VersionPath]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, Path.c_str(),true))))
							CRASH("Prototype Create Failed");
						});
				}
				if (lstrcmp(LastVersionName.c_str(), key.c_str()) && !LastVersionName.empty())
				{
					// ✅ (수정됨) '이전' 경로를 검사합니다.
					if (LastVersionPath.find("Foliage") != std::string::npos)
					{
						// ✅ (수정됨) '이전' 버전을 사용합니다.
						m_FoliageNames.push_back(LastVersionName + to_wstring(Lastversion));
						m_FoliagePaths.push_back(LastVersionPath);
					}
					else
					{
						m_PrototypeNames.push_back(LastVersionName + to_wstring(Lastversion));
						m_ModelPaths.push_back(LastVersionPath);
					}
				}
				Lastversion = version;
				LastVersionName = key;
				LastVersionPath = VersionPath;
			}
		}
	}

	if (!LastVersionName.empty())
	{
		// ✅ (수정됨) '이전' 경로를 검사합니다.
		if (LastVersionPath.find("Foliage") != std::string::npos)
		{
			// ✅ (수정됨) '이전' 버전을 사용합니다.
			m_FoliageNames.push_back(LastVersionName + to_wstring(Lastversion));
			m_FoliagePaths.push_back(LastVersionPath);
		}
		else
		{
			m_PrototypeNames.push_back(LastVersionName + to_wstring(Lastversion));
			m_ModelPaths.push_back(LastVersionPath);
		}
	}

    m_pGameInstance->Wait_Thread_End();
    
    for (_uint i = 0; i < m_PrototypeNames.size(); ++i)
    {
        m_pPreViewObject->Add_Model(m_PrototypeNames[i]);
    }


    for (_uint i = 0; i < m_FoliageNames.size(); ++i)
    {
        m_pPreViewObject->Add_Model(m_FoliageNames[i]);
    }
    
    m_pGameInstance->Wait_Thread_End();

}

void CLevel_Map::Create_TriggerBox()
{
	ImGui::Text("TriggerBox Info");
	ImGui::InputFloat3("TriggerBox Pos", reinterpret_cast<_float*>(&m_vPickedPos), "%.1f");

	ImGui::InputFloat3("TriggerBox Extends", m_TriggerBoxExtends);
	if (ImGui::Button("Create"))
	{
	CEdit_TriggerBox::TRIGGER Tri;
		Tri.iLevel = m_iLevel;
		Tri.vExtends = _float3(m_TriggerBoxExtends[0], m_TriggerBoxExtends[1], m_TriggerBoxExtends[2]);
		_matrix Mat = XMMatrixTranslationFromVector(XMVectorSet(m_vPickedPos.x, m_vPickedPos.y, m_vPickedPos.z, 1.f));
		_float4x4 TT;
		XMStoreFloat4x4(&TT, Mat);
		Tri.WorldMatrix = &TT;
		m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_TriggerBox"), m_iLevel, TEXT("Layer_Trigger"), &Tri);
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

    m_pGameInstance->Add_Work([&]() {

        m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance"),
            CEdit_MapObject_Instance::Create(m_pDevice, m_pContext));
        });

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_Brush"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxPoint.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_VIBuffer_Point"),
        CVIBuffer_Point::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject"),
        CEdit_MapObject::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_LightObject"),
        CEdit_LightObject::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_Destruction_Peice"),
		CEdit_MapObject_Destruction_Piece::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_LightObject")
        , m_iLevel, TEXT("Layer_Light"));

    Load_Objects();
    m_pBrush = CEdit_Brush::Create(m_pDevice, m_pContext);

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Destruction"),
		CEdit_MapObject_Destruction::Create(m_pDevice, m_pContext));

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


		switch (m_eMenu)
		{
		case Editor::CLevel_Map::MENU_OBJECT:
		{
			if (event.fDistance <= m_fNearDistance)
			{
				if (m_pPickedObject)
					m_pPickedObject->Set_ShaderPass(0);
				if (m_pPickedDestructObject)
					m_pPickedDestructObject->Set_ShaderPass(0);
				if (m_pPickedObject = dynamic_cast<CEdit_MapObject*>(reinterpret_cast<CGameObject*>(event.pObject)))
				{


					m_pPickedObject->Set_ShaderPass(3);

					if (m_pPickedDestructObject)
					{
						m_pPickedDestructObject->Set_ShaderPass(0);
						m_pPickedDestructObject = nullptr;
					}

					if (m_pChildObject)
					{
						m_pPickedObject->Add_Child(m_pChildObject);
						m_pChildObject = nullptr;
					}
				}
				else if (m_pPickedDestructObject = dynamic_cast<CEdit_MapObject_Destruction*>(reinterpret_cast<CGameObject*>(event.pObject)))
				{

					m_pPickedDestructObject->Set_ShaderPass(3);

					if (m_pPickedObject)
					{
						m_pPickedObject->Set_ShaderPass(0);
						m_pPickedObject = nullptr;
					}
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

		});
	m_pGameInstance->Subscribe<MAP_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), [this](const MAP_CREATE& event) {
		CGameObject* pObject = reinterpret_cast<CGameObject*>(event.pObject);
		{
			lock_guard<mutex> lock(m_Mutex);
			if (m_pPickedObject = dynamic_cast<CEdit_MapObject*>(pObject))
			{

				m_SaveObjects["Map_Object"].push_back(m_pPickedObject);
				Safe_AddRef(m_pPickedObject);
			}
			else if (m_pPickedInstanceObject = dynamic_cast<CEdit_MapObject_Instance*>(pObject))
			{
				//m_SaveInstanceObjects[m_pPickedInstanceObject->Get_Num()].push_back(m_pPickedInstanceObject);
				//Safe_AddRef(m_pPickedInstanceObject);
			}
			else if (m_pPickedDestructObject = dynamic_cast<CEdit_MapObject_Destruction*>(pObject))
			{
				m_SaveObjects["Map_Object_Destruction"].push_back(m_pPickedDestructObject);
				Safe_AddRef(m_pPickedDestructObject);
			}
			else if (m_pPickedTriggerBox = dynamic_cast<CEdit_TriggerBox*>(pObject))
			{
				m_SaveObjects["Map_Object_TriggerBox"].push_back(m_pPickedTriggerBox);
				Safe_AddRef(m_pPickedTriggerBox);
			}
		}

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

    _float3 vMousePos{};
    vMousePos.x = ptMousePos.x / (g_iWinSizeX * 0.5f) - 1.f;
    vMousePos.y = -1 * ptMousePos.y / (g_iWinSizeY * 0.5f) + 1.f;
    vMousePos.z = 0.f;
    
    XMStoreFloat3(&vMousePos, XMVector3TransformCoord(XMLoadFloat3(&vMousePos), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ)));
    
    m_vWorldPos = {};
    m_vWorldDir = vMousePos;

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

void CLevel_Map::Load_Foliage()
{
    ImGui::Text("Foliage Models");

    ImGuiID Models = ImGui::GetID("Foliage Models");
    
    ImGui::Text(WStringToString(m_szPreViewModelName).c_str());

    ImGui::BeginChildFrame(Models, ImVec2(100, 200));
    for (_uint i = 0; i < m_FoliagePaths.size(); ++i)
    {
        _char FileDrive[MAX_PATH] = {};
        _char FileDir[MAX_PATH] = {};
        _char FileName[MAX_PATH] = {};
        _char FileExt[MAX_PATH] = {};
        _splitpath_s(m_FoliagePaths[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
		m_szPreViewModelName = StringToWString(FileName);
		if (ImGui::Selectable(FileName))
        {
            //m_szPreViewModelName = StringToWString(FileName);
            _wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
            ProtoName += StringToWString(FileName);
			m_pBrush->Set_ModelName(StringToWString(FileName));
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::Begin("PreView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
#ifdef _DEBUG
            ImGui::Image(m_pGameInstance->Get_Debug_RT_Resource(TEXT("RT_Debug")), ImVec2(128, 128));
#endif
            ImGui::End();
            m_pPreViewObject->Late_Update(0.016f, m_szPreViewModelName);
        }
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
	m_pPickedDestructObject = nullptr;
	m_pPickedTriggerBox = nullptr;

    Safe_Release(m_pPreViewObject);
    Safe_Release(m_pBrush);

    for (auto& Pair : m_SaveObjects)
    {
        for (auto& pGameObject : Pair.second)
            Safe_Release(pGameObject);
        Pair.second.clear();
    }
    Safe_Release(pShaderInterface);
    Safe_Release(m_pAnimationTool);
    
    m_SaveObjects.clear();
}
