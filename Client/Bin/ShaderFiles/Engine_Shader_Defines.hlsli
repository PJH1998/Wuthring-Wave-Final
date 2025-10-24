#include "Engine_Shader_State.hlsli"
#include "Engine_Shader_Function.hlsli"

// Emissive ÃÖ¼ÒÄ¡
float g_fEmissiveThreshold = 0.6f;
// Blur Weight
float g_fWeights[13] =
{
    0.020597f, 0.037981f, 0.062950f, 0.093995f, 0.127324f, 0.153170f, 0.163967f, 0.153170f, 0.127324f, 0.093995f, 0.062950f, 0.037981f, 0.020597f
};

float g_fSSAOWeights[9] =
{
    0.075f, 0.1f, 0.125f, 0.15f, 0.3f, 0.15f, 0.125f, 0.1f, 0.075f
};

float g_fLUT_Size = 16.f;

int g_iSampleSize = 8;