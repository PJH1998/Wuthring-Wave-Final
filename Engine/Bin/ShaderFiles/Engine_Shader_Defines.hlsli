#include "Engine_Shader_State.hlsli"

// Emissive 최소치
// Blur Weight
float g_fWeights[13] =
{
    0.020597f, 0.037981f, 0.062950f, 0.093995f, 0.127324f, 0.153170f, 0.163967f, 0.153170f, 0.127324f, 0.093995f, 0.062950f, 0.037981f, 0.020597f
};

float g_fLUT_Size = 16.f;

//낮아질수록 번져지는 색이 더 많아진다 ?
float g_fEmissiveThreshold = 0.6f;

float g_fLuminence[3] = { 0.2126f, 0.7152f, 0.0722f };

float Luminance(float3 vColor)
{
    float fWeight;
    
    fWeight = (vColor.r * g_fLuminence[0]) + (vColor.g * g_fLuminence[1]) + (vColor.b * g_fLuminence[2]);
    
    return fWeight;
}