#include "EditorPch.h"
#include "Level_UI.h"

#include "Event_Level.h"
#include "Custom_UI.h"
#include "FreeCamera.h"
#include "GameObject.h"
#include "Animator_UI.h"



// 임시로 여기에 매크로로..
#define         STR2WSTR(str)                                   _wstring(str.begin(), str.end())
#define         WSTR2STR(wstr)                                  _string(wstr.begin(), wstr.end())
#define         STR_ONLYFILENAME(str)                           std::filesystem::path(str).stem().string();

#define			TO_RAD(DEGREE)									XMConvertToRadians(DEGREE)
#define			TO_DEG(RADIAN)									XMConvertToDegrees(RADIAN)

#define			IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// 이상 and 미만

#define			ROT_TO_QUAT(ROT_X, ROT_Y, ROT_Z)				XMQuaternionRotationRollPitchYaw(ROT_X, ROT_Y, ROT_Z)
#define			MAT_TO_ROT(FLOAT4X4)							_float3{TO_DEG(asin(-FLOAT4X4._32)), TO_DEG(atan2(FLOAT4X4._31, FLOAT4X4._33)), TO_DEG(atan2(FLOAT4X4._12, FLOAT4X4._22))}
#define			QUAT_TO_MAT(QUAT)								XMMatrixRotationQuaternion(QUAT)



CLevel_UI::CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_UI::Initialize()
{
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::UI);

    // ==============================
    // * Add Prototypes
    // ==============================

    // Custom UI
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI",
        CCustom_UI::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Custom_UI Load Failed. The Custom_UI may have already been loaded.\n");
    
    // Shader
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Editor_Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader Load Failed. The Shader may have already been loaded.\n");

    // VIBuffer_Rect
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
        CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

    // Animator_UI
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
        CAnimator_UI::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");


    // ==============================
    // * Add GameObjects
    // ==============================
    




    // ==============================
    // * Add Lights
    // ==============================
    LIGHT_DESC			LightDesc{};

    LightDesc.eType = LIGHT_DESC::TYPE::DIRECTION;
    LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	// Light 방향
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);		// Light 색상 및 밝기의 세기
    LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);	// Light 환경광으로 가정. 최소 밝기 보장에 관여.
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);		// Light 반사광.

    if (FAILED(m_pGameInstance->Add_Light(L"Light_Default", LightDesc)))
        return E_FAIL;

    return S_OK;
}

void CLevel_UI::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("UI"));

    m_pPreObj = m_pCurObj;

    Update_Picking();
    Update_MenuWindow();

    Update_Hierarchy();
    
    Update_SaveLoad();
    Update_Inspector();
    Update_AnimEditor(fTimeDelta);

}

void CLevel_UI::Render()
{
}

void CLevel_UI::Update_Picking()
{
    // 피킹 선택..?
    //m_pGameInstance->isPicked();
}

void CLevel_UI::Update_MenuWindow()
{
    // ============================== 
    // 이미지 로드해서 UI객체로 추가하는 창
    // ============================== 


    ImGui::Begin("Editor");

#pragma region Load Image

    // ===== [UI] Load Image =====
    if (ImGui::Button("Load Image..", ImVec2(100.f, 50.f)))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Image_Load", "Select Image", ".png,.jpg,.dds,.tga", config);
    }
    // End ==============================


    // ===== [Logic] Load FilePath, Create & Store CustomUI =====
    _wstring strFilePath = {}, strFileName = {};

    if (ImGuiFileDialog::Instance()->Display("UI_Image_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // 파일 선택 시
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _string fileName = STR_ONLYFILENAME(ImGuiFileDialog::Instance()->GetCurrentFileName());

            strFilePath = STR2WSTR(filePath);
            strFileName = STR2WSTR(fileName);


            // 상대경로
            _tchar curPath[256] = {};
            _wgetcwd(curPath, 256);
            filesystem::path basePath = curPath;
            filesystem::path targetPath = filePath;
            filesystem::path relativePath = filesystem::relative(targetPath, basePath);


            CCustom_UI::CUSTOM_UI_DESC tCustomUIDesc = {};
            tCustomUIDesc.fSizeX = 100;
            tCustomUIDesc.fSizeY = 100;
            tCustomUIDesc.fX = g_iWinSizeX / 2.f;
            tCustomUIDesc.fY = g_iWinSizeY / 2.f;
            tCustomUIDesc.strFilePath = relativePath.wstring();
            tCustomUIDesc.strFileName = strFileName;

            // 생성 후 로컬 컨테이너에 추가
            CGameObject* pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Custom_UI", PROTOTYPE::GAMEOBJECT, &tCustomUIDesc));
            if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), L"Layer_UI_Custom", pCustomObj)))
                CRASH(Failed to add Custom_UI gameobject.);

            HIERARCHY_OBJ_DESC tObjDesc = { };
            tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
            tObjDesc.strObjName = STR2WSTR(fileName);

            m_vecCustomUIs.push_back(tObjDesc);
            m_pCurObj = pCustomObj;
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // End ==============================

#pragma endregion

#pragma region [Window Toggle] Save / Load

    // ===== [Button] Save / Load =====
    if (ImGui::Button("Save / Load", ImVec2(100.f, 50.f)))
    {
        m_isOn_SaveLoad = !m_isOn_SaveLoad;
    }

#pragma endregion



    ImGui::End();
}

void CLevel_UI::Update_Hierarchy()
{
    // ============================== 
    // 유사 하이어라키 창, 로드된 객체 선택 가능하도록
    // ============================== 

    ImGui::Begin("Hierarchy");

    if (m_pCurObj)
    {
        CCustom_UI::CUSTOM_UI_DESC tDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
        static _char szUIName[256] = {};
        
        _string strUIName = WSTR2STR(tDesc.strUIName);
        strcpy_s(szUIName, strUIName.c_str());

        ImGui::Text("Name : ");
        ImGui::SameLine();
        if (ImGui::InputText("##Edit Name", szUIName, 256))
        {
            _string strEditUIName = szUIName;
            
            tDesc.strUIName = STR2WSTR(strEditUIName);
            dynamic_cast<CCustom_UI*>(m_pCurObj)->Set_UIDesc(tDesc);
        }
    }
    else
    {
        ImGui::Text("Selected Nothing");
    }

    ImGui::Separator();

    // 매 프레임마다 벡터를 통해 부모 구조를 파악하고,
    // 그걸 컨테이너에 담은 뒤, 하이어라키에서 표시?

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected;

    // 하이어라키 메인
    for (auto& ui : m_vecCustomUIs)
    {
        CCustom_UI* pUI = ui.pCustomUI;
        CCustom_UI::CUSTOM_UI_DESC desc = pUI->Get_UIDesc();

        // 부모가 없는 (최상위) 객체만 먼저 표시
        if (desc.strParentName.empty())
            Update_Hierarchy_CheckTree(pUI, flags);
    }

    // 부모이름은 있지만 해당 부모가 없는 경우 별도 UI로 표시
    ImGuiTreeNodeFlags flags_missingParent = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected;
    if (ImGui::CollapsingHeader("Missing Parent Objects", flags_missingParent))
    {
        for (auto& ui : m_vecCustomUIs)
        {
            _bool isParentMissing = true;

            static _int iSelected = -1;
            _uint iIndex = 0;

            // 부모 이름이 있는 경우 체크X (위에서 이미 찾았으므로)
            if (ui.pCustomUI->Get_UIDesc().strParentName.empty())
                break;
            // 해당하는 부모가 있는지 검사
            for (auto& otherui : m_vecCustomUIs)
                if (otherui.pCustomUI->Get_UIDesc().strUIName ==
                    ui.pCustomUI->Get_UIDesc().strParentName)
                {
                    isParentMissing = false;
                    break;
                    iIndex++;
                }

            
            if (isParentMissing)
            {
                _wstring wstrUIName = ui.pCustomUI->Get_UIDesc().strUIName;
                _string strUIName = WSTR2STR(wstrUIName);
                if (ImGui::Selectable(strUIName.c_str(), iSelected == iIndex))
                    m_pCurObj = ui.pCustomUI;
            }
        }
    }
    
    ImGui::End();
}

void CLevel_UI::Update_Hierarchy_CheckTree(CCustom_UI* pParentUI, ImGuiTreeNodeFlags flags)
{
    CCustom_UI::CUSTOM_UI_DESC desc = pParentUI->Get_UIDesc();

    // TreeNode 생성
    _string strLabel = WSTR2STR(desc.strUIName);
    if (ImGui::TreeNodeEx(strLabel.c_str(), flags))
    {
        // 클릭 시 선택.
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_pCurObj = pParentUI;

        // m_vecCustomUIs 전체를 돌면서, 부모 이름이 일치하는 객체를 찾음
        for (auto& ui : m_vecCustomUIs)
        {
            CCustom_UI* pChild = ui.pCustomUI;
            CCustom_UI::CUSTOM_UI_DESC childDesc = pChild->Get_UIDesc();

            if (childDesc.strParentName == desc.strUIName)
                Update_Hierarchy_CheckTree(pChild, flags); // 재귀 호출
        }

        ImGui::TreePop();
    }
}

void CLevel_UI::Update_SaveLoad()
{
    if (!m_isOn_SaveLoad)
        return;
        
    ImGui::Begin("Save / Load");
    const ImVec2 buttonSize = { 100.f, 20.f };
#pragma region [UI] Open Dialog for Save / Load
    // ==============================
    // * [UI] UI Save
    // ==============================
    ImGui::Text("..Current UI Info");
    if (ImGui::Button("Save##InfoSave", buttonSize) &&
        m_pCurObj)
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIInfo/";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Info_Save", "Select Info Save Path", ".json", config);
    }
    ImGui::SameLine();
    // ==============================
    // * [UI] UI Load
    // ==============================
    if (ImGui::Button("Load##InfoLoad", buttonSize))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIInfo/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Info_Load", "Select Info", ".json", config);
    }

    ImGui::Separator();


    // ==============================
    // * [UI] Anim Save
    // ==============================
    ImGui::Text("..Current Anim");

    static _char szAnimName[256] = {};
    ImGui::Text("[Save] Anim Name");
    ImGui::InputText("##Anim Name", szAnimName, 256);

    if (ImGui::Button("Save##AnimSave", buttonSize) &&
        m_pCurObj)
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIAnim/";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Anim_Save", "Select Anim Save Path", ".json", config);
    }
    ImGui::SameLine();
    // ==============================
    // * [UI] Anim Load
    // ==============================
    if (ImGui::Button("Load##AnimLoad", buttonSize) && 
        m_pCurObj)
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIAnim/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Anim_Load", "Select Anim", ".json", config);
    }
#pragma endregion
    ImGui::End();


#pragma region [Logic] Save / Load with Dialog
    // ==============================
    // * [Logic] UI Save
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Info_Save"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = STR2WSTR(filePath);

            _string fileName = STR_ONLYFILENAME(ImGuiFileDialog::Instance()->GetCurrentFileName());
            _wstring strFileName = STR2WSTR(fileName);

            UI_INFO_DESC tCurUIInfoDesc = {};

            tCurUIInfoDesc.tUIDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
            tCurUIInfoDesc.vPos = m_vCurObjPos;
            tCurUIInfoDesc.vRot = m_vCurObjRot;
            tCurUIInfoDesc.vSca = m_vCurObjSca;

            json jUIInfoData = {};
            to_json(jUIInfoData, tCurUIInfoDesc);

            ofstream file(strFilePath);
            file << jUIInfoData.dump(4);
            file.close();
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // ==============================
    // * [Logic] UI Load
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Info_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // 파일 선택 시
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = STR2WSTR(filePath);

            ifstream file(strFilePath);
            json jUIInfoData = {};
            if (file.is_open()) {
                file >> jUIInfoData;
            }

            CCustom_UI::CUSTOM_UI_DESC tLoadUIInfoDesc = {};
            from_json(jUIInfoData["tUIDesc"], tLoadUIInfoDesc);


            _float3 vPos = { jUIInfoData["vPos"][0], jUIInfoData["vPos"][1], jUIInfoData["vPos"][2] };    m_vCurObjPos = vPos;
            _float3 vRot = { jUIInfoData["vRot"][0], jUIInfoData["vRot"][1], jUIInfoData["vRot"][2] };    m_vCurObjRot = vRot;
            _float3 vSca = { jUIInfoData["vSca"][0], jUIInfoData["vSca"][1], jUIInfoData["vSca"][2] };    m_vCurObjSca = vSca;

            CGameObject* pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Custom_UI", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc));
            if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), L"Layer_UI_Custom", pCustomObj)))
                CRASH(Failed to add Custom_UI gameobject.);

            HIERARCHY_OBJ_DESC tObjDesc = { };
            tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
            tObjDesc.strObjName = STR2WSTR(tLoadUIInfoDesc.strFileName);

            _matrix matScale = XMMatrixScaling(vSca.x, vSca.y, vSca.z);
            _matrix matRotX = XMMatrixRotationX(TO_RAD(vRot.x));
            _matrix matRotY = XMMatrixRotationY(TO_RAD(vRot.y));
            _matrix matRotZ = XMMatrixRotationZ(TO_RAD(vRot.z));
            _matrix matRot = matRotZ * matRotY * matRotX;
            _matrix matTrans = XMMatrixTranslation(vPos.x, vPos.y, vPos.z);

            _matrix matWorld = matScale * matRot * matTrans;
            static_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"))->Set_WorldMatrix(matWorld);


            m_vecCustomUIs.push_back(tObjDesc);
            m_pCurObj = pCustomObj;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // ==============================
    // * [Logic] Anim Save
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Anim_Save"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // 파일 선택 시
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = STR2WSTR(filePath);

            UI_ANIM_DESC tAnimDesc = {};

            tAnimDesc.tUIDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
            _string strAnimName = _string(szAnimName);
            tAnimDesc.strAnimName = STR2WSTR(strAnimName);
            tAnimDesc.iLerpType = m_iLerpType;
            tAnimDesc.isLoop = m_isAnimLoop;

            for (auto& keyframeDesc : m_vecUIKeyFrameDescs)
                tAnimDesc.vecKeyFrames.push_back(keyframeDesc);

            json jUIAnimData = {};
            to_json(jUIAnimData, tAnimDesc);

            ofstream file(filePath);
            file << jUIAnimData.dump(4);
            file.close();

            memset(szAnimName, 0, sizeof(szAnimName));
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // ==============================
    // * [Logic] Anim Load
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Anim_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // 파일 선택 시
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = STR2WSTR(filePath);

            ifstream file(strFilePath);
            json jUIAnimData = {};
            if (file.is_open()) {
                file >> jUIAnimData;
            }

            UI_ANIM_DESC tLoadAnimDesc = {};
            from_json(jUIAnimData, tLoadAnimDesc);

            CAnimator_UI* ObjAnimatorCom = dynamic_cast<CAnimator_UI*> (m_pCurObj->Get_Component(L"Com_Animator_UI"));
            if (!ObjAnimatorCom) CRASH();

            _wstring strCurObjName = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().strFileName;
            _wstring strReqObjName = tLoadAnimDesc.tUIDesc.strFileName;
            if (strCurObjName == strReqObjName)
            {
                if (FAILED(ObjAnimatorCom->Insert_Animation(tLoadAnimDesc)))
                {
                    _wstring strLog = L"[Level_UI][Update_SaveLoad] Insert Animation Failed. Animation [" + tLoadAnimDesc.strAnimName + L"] Already Exist.";
                    OutputDebugString(strLog.c_str());
                }
                else
                {
                    ObjAnimatorCom->Change_Animation(tLoadAnimDesc.strAnimName); // 불러온 애니메이션으로 할당
                }
            }
            else
            {
                _wstring strLog = L"[Level_UI][Update_SaveLoad] Load Failed. This Animation is not for this object.\nRequired Object Name : " + tLoadAnimDesc.tUIDesc.strFileName;
                OutputDebugString(strLog.c_str());
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
#pragma endregion

}

void CLevel_UI::Update_Inspector()
{
    // ============================== 
    // 유사 인스펙터 창, 컴포넌트 조작 가능하도록
    // ============================== 


    if (m_pCurObj == nullptr)
        return;

    // 선택중인 오브젝트 값 불러와서 Transform 수정 가능하도록

    ImGui::Begin("Inspector");
    
#pragma region [Component] Transform

    CTransform* pTargetTransform = dynamic_cast<CTransform*>(m_pCurObj->Get_Component(L"Com_Transform"));
    _bool   isOn_TransformCom = pTargetTransform != nullptr;

    if (isOn_TransformCom)
    {
        //static _float3 vSelectedObjPos = {};      // changed to m_vCurObjPos
        //static _float3 vSelectedObjRot = {};      // changed to m_vCurObjRot
        //static _float3 vSelectedObjSca = {};      // changed to m_vCurObjSca

        if (m_pPreObj != m_pCurObj ||
            m_isPlayAnimation)
        {
            _vector		vXMObjPosition = {}, vXMObjQuaternion = {}, vXMObjScale = {};
            _float3		vStoreObjPosition = {}, vStoreObjRotation = {}, vStoreObjScale = {};
            XMMatrixDecompose(&vXMObjScale, &vXMObjQuaternion, &vXMObjPosition, pTargetTransform->Get_WorldMatrix());

            _float4x4	matStoreObjQuaternion = {};	// 쿼터니언
            XMStoreFloat4x4(&matStoreObjQuaternion, QUAT_TO_MAT(vXMObjQuaternion));

            XMStoreFloat3(&vStoreObjPosition, vXMObjPosition);
            vStoreObjRotation = MAT_TO_ROT(matStoreObjQuaternion);
            XMStoreFloat3(&vStoreObjScale, vXMObjScale);

            // 대입하여 보여줌
            m_vCurObjPos = vStoreObjPosition;
            m_vCurObjRot = vStoreObjRotation;
            m_vCurObjSca = vStoreObjScale;
        }
        

        if (ImGui::CollapsingHeader("Transform"))
        {
            if (ImGui::BeginMenu("Reset Menu"))
            {
                if (ImGui::MenuItem("Reset Position"))  { m_vCurObjPos = { 0.f, 0.f, 0.f }; }
                if (ImGui::MenuItem("Reset Rotation"))  { m_vCurObjRot = { 0.f, 0.f, 0.f }; }
                if (ImGui::MenuItem("Reset Scale"))     { m_vCurObjSca = { 100.f, 100.f, 1.f }; }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Transform")) {
                    m_vCurObjPos = { 0.f, 0.f, 0.f };
                    m_vCurObjRot = { 0.f, 0.f, 0.f };
                    m_vCurObjSca = { 1.f, 1.f, 1.f };
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            ImGui::PushItemWidth(60);

            // Position Ctrl
            ImGui::Text("Position");
            ImGui::DragFloat("X##pos", &m_vCurObjPos.x, 1.f);   ImGui::SameLine();
            ImGui::DragFloat("Y##pos", &m_vCurObjPos.y, 1.f);   ImGui::SameLine();
            ImGui::DragFloat("Z##pos", &m_vCurObjPos.z, 1.f);
            ImGui::Separator();

            // Rotation Ctrl
            ImGui::Text("Rotation");
            ImGui::DragFloat("X##rot", &m_vCurObjRot.x, 1.f);   ImGui::SameLine();
            ImGui::DragFloat("Y##rot", &m_vCurObjRot.y, 1.f);   ImGui::SameLine();
            ImGui::DragFloat("Z##rot", &m_vCurObjRot.z, 1.f);
            ImGui::Separator();

            // Scale Ctrl
            ImGui::Text("Scale");
            ImGui::DragFloat("X##sca", &m_vCurObjSca.x, 1.f);   ImGui::SameLine();
            ImGui::DragFloat("Y##sca", &m_vCurObjSca.y, 1.f);   ImGui::SameLine();
            ImGui::DragFloat("Z##sca", &m_vCurObjSca.z, 1.f);
            ImGui::Separator();

            ImGui::PopItemWidth();
        }


        _matrix matXMEditPosition = XMMatrixTranslationFromVector(XMLoadFloat3(&m_vCurObjPos));
        _matrix matXMEditRotation = XMMatrixRotationRollPitchYaw(TO_RAD(m_vCurObjRot.x), TO_RAD(m_vCurObjRot.y), TO_RAD(m_vCurObjRot.z));
        _matrix matXMEditScale = XMMatrixScalingFromVector(XMLoadFloat3(&m_vCurObjSca));

        _matrix matXMEditResult = matXMEditScale * matXMEditRotation * matXMEditPosition;

        // UI 내의 Begin 때문에 적용 안되는듯. 임시로 비활성화함
        if (!m_isPlayAnimation)
            pTargetTransform->Set_WorldMatrix(matXMEditResult);
    }

#pragma endregion

#pragma region [Other] Description Edit
    if (ImGui::CollapsingHeader("Edit UI Desciption"))
    {
        static _char szUIName[256] = {};
        static _uint iUIType = {};
        static _char szParentName[256] = {};
        
        CCustom_UI::CUSTOM_UI_DESC tDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();


        // 기존 값 반영
        _string strUIName = _string(tDesc.strUIName.begin(), tDesc.strUIName.end());
        strcpy_s(szUIName, strUIName.c_str());

        iUIType = tDesc.iUIType;

        _string strParentName = _string(tDesc.strParentName.begin(), tDesc.strParentName.end());
        strcpy_s(szParentName, strParentName.c_str());



        // 값 수정 UI
        ImGui::Text("UI Name");
        ImGui::InputText("##UI Name", szUIName, 256);

        ImGui::Separator();

        ImGui::Text("UI Type");
        const char* szUITypeNames[] = { "NONE", "BUTTON", "INTERACT" };
        const _uint iTypeCount = ENUM_CLASS(CCustom_UI::UI_TYPE::END);
        const char* szCurrentItem = szUITypeNames[iUIType];

        if (ImGui::BeginCombo("##UI Type", szCurrentItem))
        {
            for (_uint i = 0; i < iTypeCount; ++i)
            {
                const _bool isSelected = (iUIType == i);
                if (ImGui::Selectable(szUITypeNames[i], isSelected))
                    iUIType = i;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        ImGui::Text("Parent Name");
        ImGui::InputText("##Parent Name", szParentName, 256);



        // 다시 값 할당
        _string strEditedUIName = szUIName;
        tDesc.strUIName = _wstring(strEditedUIName.begin(), strEditedUIName.end());

        tDesc.iUIType = iUIType;

        _string strEditedParentName = szParentName;
        tDesc.strParentName = _wstring(strEditedParentName.begin(), strEditedParentName.end());

        dynamic_cast<CCustom_UI*>(m_pCurObj)->Set_UIDesc(tDesc);
    }
#pragma endregion


#pragma region [Other] AnimEdit Toggle

    if (ImGui::CollapsingHeader("Animation Editor"))
    {
        if (ImGui::Button("Open Anim Editor", ImVec2(200.f, 20.f)))
            m_isOn_AnimEdit = !m_isOn_AnimEdit;
    }

#pragma endregion

    
    ImGui::End();



}

void CLevel_UI::Update_AnimEditor(_float fTimeDelta)
{
    if (!m_pCurObj ||
        !m_isOn_AnimEdit)
        return;

    static _int iKeyFrame = 0;
    static _int iRecentKeyFrame = 0;
    static _int iTexIndex = 0;
    static _int iMaxTexIndex = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().iNumFiles;
    static _float fAlpha = 0.f;

    static _int iAnimEditorSelected = -1;
    static _int iAnimListSelected = -1;


    CCustom_UI* pTargetUI = dynamic_cast<CCustom_UI*>(m_pCurObj);
    CAnimator_UI* pTargetAnimator = dynamic_cast<CAnimator_UI*>(pTargetUI->Get_Component(L"Com_Animator_UI"));


    UI_ANIM_KEYFRAME_DESC tKeyFrameDesc = {};

    ImGui::Begin("Animation Editor");

#pragma region Add Keyframe Menu

    if (ImGui::CollapsingHeader("Add Menu"))
    {
        ImGui::Text("Keyframe Index");
        ImGui::InputInt("##KeyFrame Index", &iKeyFrame);
        if          (m_vecUIKeyFrameDescs.empty())  iKeyFrame = 0;
        else if     (m_pSelectedKeyFrameDesc)       {}
        else if     (iRecentKeyFrame >= iKeyFrame)  iKeyFrame = iRecentKeyFrame + 1;

        ImGui::Separator();

        ImGui::Text("Texture Index (NumTex : %d)", iMaxTexIndex);
        ImGui::InputInt("##Texture Index", &iTexIndex);
        if          (iTexIndex < 0)                 iTexIndex = 0;
        else if     (m_pSelectedKeyFrameDesc)       {}
        else if     (iMaxTexIndex <= iTexIndex)     iTexIndex = iMaxTexIndex - 1;

        ImGui::Separator();

        ImGui::Text("Texture Alpha");
        ImGui::DragFloat("##Texture Alpha", &fAlpha, 0.001f, 0.f, 1.f);

        ImGui::Separator();

        ImGui::Text("Lerp Type");
        if      (m_iLerpType == 0)
        {
            if (ImGui::Button("Linear")) { m_iLerpType = 1; } // To Cubic
        }
        else if (m_iLerpType == 1)
        {
            if (ImGui::Button("Cubic")) { m_iLerpType = 0; } // To Linear
        }
            
        ImGui::Separator();

        ImGui::Checkbox("Loop", &m_isAnimLoop);

        ImGui::Separator();

        if (m_pSelectedKeyFrameDesc == nullptr)
        {
            if (ImGui::Button("Add Keyframe"))
            {
                UI_ANIM_KEYFRAME_DESC tTempDesc = {
                    iKeyFrame,
                    iTexIndex,
                    fAlpha,
                    m_vCurObjPos,
                    m_vCurObjRot,
                    m_vCurObjSca
                };

                m_vecUIKeyFrameDescs.push_back(tTempDesc);
            }
        }
        else
        {
            if (ImGui::Button("Edit Keyframe"))
            {
                m_pSelectedKeyFrameDesc->fAlpha = fAlpha;
                m_pSelectedKeyFrameDesc->iKeyframeIndex = iKeyFrame;
                m_pSelectedKeyFrameDesc->iTexIndex = iTexIndex;

                m_pSelectedKeyFrameDesc->vPos = m_vCurObjPos;
                m_pSelectedKeyFrameDesc->vRot = m_vCurObjRot;
                m_pSelectedKeyFrameDesc->vSca = m_vCurObjSca;

                m_pSelectedKeyFrameDesc = nullptr;
                iRecentKeyFrame = m_vecUIKeyFrameDescs.back().iKeyframeIndex;
            }
            ImGui::SameLine();
            if (ImGui::Button("Deselect##KeyFrame Deselect"))
            {
                m_pSelectedKeyFrameDesc = nullptr;
                iRecentKeyFrame = m_vecUIKeyFrameDescs.back().iKeyframeIndex;
            }
        }

    }

#pragma endregion

#pragma region Keyframe List

    if (ImGui::CollapsingHeader("Keyframe List"))
    {
        if (m_vecUIKeyFrameDescs.empty())
            ImGui::Selectable("(Empty)##AnimEdit", false);

        for (_uint i = 0; i < m_vecUIKeyFrameDescs.size(); i++)
        {
            _string strLabel = "Keyframe [" + to_string(i + 1) + "] | [" + to_string(m_vecUIKeyFrameDescs[i].iKeyframeIndex) + "]";
            if (ImGui::Selectable(strLabel.c_str(), iAnimEditorSelected == i))
            {
                m_pSelectedKeyFrameDesc = &m_vecUIKeyFrameDescs[i];

                iKeyFrame = m_pSelectedKeyFrameDesc->iKeyframeIndex;
                iTexIndex = m_pSelectedKeyFrameDesc->iTexIndex;
                fAlpha = m_pSelectedKeyFrameDesc->fAlpha;

                m_vCurObjPos = m_pSelectedKeyFrameDesc->vPos;
                m_vCurObjRot = m_pSelectedKeyFrameDesc->vRot;
                m_vCurObjSca = m_pSelectedKeyFrameDesc->vSca;
            }
        }

    }

#pragma endregion

#pragma region Animation List

    if (ImGui::CollapsingHeader("Animation List"))
    {

        if (m_pSelectedUIAnim != nullptr &&
            !m_isPlayAnimation)
        {
            if (ImGui::Button("Play Animation"))
                m_isPlayAnimation = true;
        }
        else if (m_pSelectedUIAnim != nullptr &&
            m_isPlayAnimation)
        {
            if (ImGui::Button("Stop Animation"))
                m_isPlayAnimation = false;
        }
        else if (ImGui::Button("Nothing Selected")) {}


        if (m_pSelectedUIAnim)
        {
            ImGui::SameLine();
            if (ImGui::Button("Deselect##AnimList Deselect"))
                m_pSelectedUIAnim = nullptr;
        }


        if (pTargetAnimator->Find_Animation(0) == nullptr)
            ImGui::Selectable("(Empty)##AnimList", false);

        _uint iIndex = 0;
        while (true)
        {
            CLevel_UI::UI_ANIM_DESC* pDesc = pTargetAnimator->Find_Animation(iIndex);
            if (!pDesc) break;

            _string strLabel = "Anim [" + to_string(iIndex) + "] | [" + _string(pDesc->strAnimName.begin(), pDesc->strAnimName.end()) + "]";
            if (ImGui::Selectable(strLabel.c_str(), iAnimListSelected == iIndex))
            {
                m_pSelectedUIAnim = pDesc;
            }

            iIndex++;
        }

    }

#pragma endregion

    ImGui::End();



#pragma region Update Animation
    if (pTargetUI &&
        pTargetAnimator->Find_Animation(0) != nullptr &&
        m_isPlayAnimation)
    {
        if (pTargetAnimator->Get_CurAnimation() == nullptr)
        {
            if (m_pSelectedUIAnim != nullptr)
                pTargetAnimator->Change_Animation(m_pSelectedUIAnim->strAnimName);
        }
    }
    else
    {
        pTargetAnimator->Deselect_Animation();
    }

#pragma endregion



}

CLevel_UI* CLevel_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_UI* pInstance = new CLevel_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_UI::Free()
{
    __super::Free();

}
