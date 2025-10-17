// Emissive효과를 넣을지 판단할 때 사용하는 RGB 계수
float g_fLuminence[3] = { 0.2126, 0.7152, 0.0722 };

float Luminame(float3 vColor)
{
    float fWeight;
    
    fWeight = (vColor.r * g_fLuminence[0]) + (vColor.g * g_fLuminence[1]) + (vColor.b * g_fLuminence[2]);
    
    return fWeight;
}

float Random(float2 vRange)
{
    return frac(sin(dot(vRange.xy, float2(12.9898, 78.233))) * 43758.5453123);
}