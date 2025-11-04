#include "Engine_Shader_State.hlsli"


float g_fLUT_Size = 16.f;

//���������� �������� ���� �� �������� ?
float g_fEmissiveThreshold = 0.7f;

float g_fLuminence[3] = { 0.2126f, 0.7152f, 0.0722f };

//PBR
float g_fGlobalMetallic = 0.f;  // PBR.x
float g_fGlobalRoughness = 0.25f; // PBR.y

float Luminance(float3 vColor)
{
    float fWeight;
    
    fWeight = (vColor.r * g_fLuminence[0]) + (vColor.g * g_fLuminence[1]) + (vColor.b * g_fLuminence[2]);
    
    return fWeight;
}

bool IsInNDC(float4 vProjPos)
{
    if (vProjPos.x > 1.f || vProjPos.x < -1.f)
        return false;
    
    if (vProjPos.y > 1.f || vProjPos.y < -1.f)
        return false;
    
    if (vProjPos.z > 1.f || vProjPos.z < 0.f)
        return false;
    
    return true;
}