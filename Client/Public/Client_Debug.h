#pragma once
#include "ClientPch.h"

#ifdef _DEBUG
namespace ClientDebug
{
    static void Edit_TransformRotate(CTransform* pTransform)
    {
        ImGuiIO& io = ImGui::GetIO();

        // ±âÁ¸ Player Debug Window

        ImVec2 windowSize = ImVec2(300.f, 300.f);
        ImVec2 windowPos = ImVec2(io.DisplaySize.x - windowSize.x, 0.f);

        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

        string strDebug = "Transform Debug";
        ImGui::Begin(strDebug.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);

        static _float vRotation[3] = { 0.f, 0.f, 0.f };
        ImGui::Text("Rotation(Radian) : (%.2f, %.2f, %.2f)", vRotation[0], vRotation[1], vRotation[2]);
        ImGui::Separator();

        ImGui::InputFloat3("Rotation(Radian)", vRotation);

        _float3 vRot = {};
        if (ImGui::Button("Apply Transform Rotate"))
        {
            vRot = { XMConvertToRadians(vRotation[0]),
            XMConvertToRadians(vRotation[1]),
            XMConvertToRadians(vRotation[2]) };
            pTransform->Rotation_Quaternion(vRot);
        }

        ImGui::Separator();

        if (ImGui::Button("Apply Transform Reverse Rotate"))
        {
            vRot = { XMConvertToRadians(-vRotation[0]),
            XMConvertToRadians(-vRotation[1]),
            XMConvertToRadians(-vRotation[2]) };
        }

        ImGui::Separator();

        if (ImGui::Button("Reset Rotation"))
        {
            _float3 vRot = { 0.f, 0.f, 0.f };
            pTransform->Rotation_Quaternion(vRot);
            vRotation[0] = vRotation[1] = vRotation[2] = 0.f;
        }

        ImGui::End();
    }
}


#endif // _DEBUG