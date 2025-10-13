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



    // FreeCamera
    //if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Camera_Free",
    //    CFreeCamera::Create(m_pDevice, m_pContext))))
    //    CRASH(Failed to add FreeCamera prototype.);

    // ==============================
    // * Add GameObjects
    // ==============================
    

    // FreeCamera
    //CFreeCamera::CAMERA_DESC	CameraDesc{};
    //
    //CameraDesc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
    //CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
    //CameraDesc.fFovy = XMConvertToRadians(60.0f);
    //CameraDesc.fNear = 0.1f;
    //CameraDesc.fFar = 500.f;
    //CameraDesc.fSpeedPerSec = 20.f;
    //CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);
    //CameraDesc.fMouseSensor = .2f;
    //
    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, TEXT("Prototype_GameObject_Camera_Free"),
    //    ENUM_CLASS(LEVEL::UI), L"Layer_Camera", &CameraDesc)))
    //    return E_FAIL;


    //CCamera::CAMERA_DESC CameraDesc = {};
    //CameraDesc.fFovy = XMConvertToRadians(60.f);
    //CameraDesc.fNear = 0.1f;
    //CameraDesc.fFar = 100000.f;
    //CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
    //CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
    //CameraDesc.fSpeedPerSec = 1000.f;
    //CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
    //CameraDesc.fMouseSensor = 0.004f;
    //
    //CFreeCamera* pFreeCamera = CFreeCamera::Create(m_pDevice, m_pContext);
    //ASSERT_CRASH(pFreeCamera);
    //if (FAILED(pFreeCamera->Initialize_Clone(&CameraDesc)))
    //    CRASH("Free Camera");
    //m_pGameInstance->Add_GameObject_ToLayer(iDestLevel, L"Layer_Camera", pFreeCamera);


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
    Update_AnimEditor();

}

void CLevel_UI::Render()
{
}

void CLevel_UI::Update_Picking()
{
    // 피킹 선택..?

}

void CLevel_UI::Update_MenuWindow()
{
    // ============================== 
    // 이미지 로드해서 UI객체로 추가하는 창
    // ============================== 


    ImGui::Begin("Editor");

#pragma region Load Image

    // ===== [Button] Load Image =====
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

    static _int iSelected = -1;

    for (_uint i = 0; i < m_vecCustomUIs.size(); i++)
    {
        // 오브젝트 갯수만큼 목록화, 클릭 시 해당 객체를 선택된 객체로
        if (ImGui::Selectable(WSTR2STR(m_vecCustomUIs[i].strObjName).c_str(), iSelected == i))
        {
            m_pCurObj = m_vecCustomUIs[i].pCustomUI;
        }
    }


    ImGui::End();
}

void CLevel_UI::Update_SaveLoad()
{
    if (!m_isOn_SaveLoad)
        return;
    
    // ksta : 이거 이 창에서 분리해야 할 듯
    ImGui::Begin("Save / Load");


    const ImVec2 buttonSize = { 100.f, 20.f };

    ImGui::Text("..Current UI Info");
    if (ImGui::Button("Save##InfoSave", buttonSize) &&
        m_pCurObj)
    {
        UI_INFO_DESC tCurUIInfoDesc = {};

        tCurUIInfoDesc.tUIDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
        tCurUIInfoDesc.vPos = m_vCurObjPos;
        tCurUIInfoDesc.vRot = m_vCurObjRot;
        tCurUIInfoDesc.vSca = m_vCurObjSca;

        json jUIInfoData = {};
        to_json(jUIInfoData, tCurUIInfoDesc);

        ofstream file("../../Client/Bin/Resource/UI/Test/Json/testCurUIInfo.json"); // 나중에 여럿 저장 되도록..
        file << jUIInfoData.dump(4);
        file.close();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load##InfoLoad", buttonSize))
    {
        CCustom_UI::CUSTOM_UI_DESC tLoadUIInfoDesc = {};

        // Output 용이라서 vPos vRot 이런게 무의미하게 날아가는듯
        // 이걸 그냥 Transform에 다이렉트로?
        ifstream file("../../Client/Bin/Resource/UI/Test/Json/testCurUIInfo.json");
        json jUIInfoData = {};
        if (file.is_open()) {
            file >> jUIInfoData;
        }

        tLoadUIInfoDesc.iNumFiles        = jUIInfoData["tUIDesc"]["iNumFiles"];
        _string strFileName = jUIInfoData["tUIDesc"]["strFileName"].get<string>();
        tLoadUIInfoDesc.strFileName      = STR2WSTR(strFileName);
        _string strFilePath = jUIInfoData["tUIDesc"]["strFilePath"].get<string>();
        tLoadUIInfoDesc.strFilePath      = STR2WSTR(strFilePath);

        _float3 vPos = {jUIInfoData["vPos"][0], jUIInfoData["vPos"][1], jUIInfoData["vPos"][2]};    m_vCurObjPos = vPos;
        _float3 vRot = {jUIInfoData["vRot"][0], jUIInfoData["vRot"][1], jUIInfoData["vRot"][2]};    m_vCurObjRot = vRot;
        _float3 vSca = {jUIInfoData["vSca"][0], jUIInfoData["vSca"][1], jUIInfoData["vSca"][2]};    m_vCurObjSca = vSca;

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

    ImGui::Separator();

    ImGui::Text("..Current Anim");

    _char szAnimName[256] = {};
    ImGui::Text("[Save] Anim Name");
    ImGui::InputText("##Anim Name", szAnimName, 256);

    if (ImGui::Button("Save##AnimSave", buttonSize) &&
        m_pCurObj)
    {
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

        ofstream file("../../Client/Bin/Resource/UI/Test/Json/testCurUIAnim.json"); // 나중에 여럿 저장 되도록..
        file << jUIAnimData.dump(4);
        file.close();

    }
    ImGui::SameLine();
    if (ImGui::Button("Load##AnimLoad", buttonSize))
    {

    }

    ImGui::End();

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

        if (m_pPreObj != m_pCurObj)
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
        pTargetTransform->Set_WorldMatrix(matXMEditResult);
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

void CLevel_UI::Update_AnimEditor()
{
    if (!m_pCurObj ||
        !m_isOn_AnimEdit)
        return;

    static _int iKeyFrame = 0;
    static _int iRecentKeyFrame = 0;
    static _int iTexIndex = 0;
    static _int iMaxTexIndex = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().iNumFiles;
    static _float fAlpha = 0.f;

    static _int iSelected = -1;

    UI_ANIM_KEYFRAME_DESC tKeyFrameDesc = {};

    ImGui::Begin("Animation Editor");

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
            if (ImGui::Button("Deselect"))
            {
                m_pSelectedKeyFrameDesc = nullptr;
                iRecentKeyFrame = m_vecUIKeyFrameDescs.back().iKeyframeIndex;
            }
        }

    }
    if (ImGui::CollapsingHeader("Keyframe List"))
    {
        if (m_vecUIKeyFrameDescs.empty())
            ImGui::Selectable("(Empty)", false);

        for (_uint i = 0; i < m_vecUIKeyFrameDescs.size(); i++)
        {
            _string strLabel = "Keyframe [" + to_string(i + 1) + "] | [" + to_string(m_vecUIKeyFrameDescs[i].iKeyframeIndex) + "]";
            if (ImGui::Selectable(strLabel.c_str(), iSelected == i))
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

    



    ImGui::End();
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
