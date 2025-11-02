#include "Engine_Shader_State.hlsli"

// Emissive �ּ�ġ
// Blur Weight
float g_fWeights[13] =
{
    0.020597f, 0.037981f, 0.062950f, 0.093995f, 0.127324f, 0.153170f, 0.163967f, 0.153170f, 0.127324f, 0.093995f, 0.062950f, 0.037981f, 0.020597f
};

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