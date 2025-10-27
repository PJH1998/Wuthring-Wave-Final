float2 Compute_Texcoord(int3 DTID, float fWidth, float fHeight)
{
    float2 vTexcoord = 0.f;
    
    vTexcoord.x = (float) DTID.x / fWidth;
    vTexcoord.y = (float) DTID.y / fHeight;

    return vTexcoord;
}

float2 Compute_Texcoord_Proj(float2 vProjXY)
{
    float2 vTexcoord = 0.f;
    
    vTexcoord.x = vProjXY.x * 0.5f + 0.5f;
    vTexcoord.y = vProjXY.y * -0.5f + 0.5f;
        
    return vTexcoord;
}

int2 Compute_Pixel(float4 vProjPos, float fWidth, float fHeight)
{
    int2 Pixel = 0;
    
    Pixel.x = ((vProjPos.x / vProjPos.w) * 0.5f + 0.5f) * fWidth;
    Pixel.y = ((vProjPos.y / vProjPos.w) * -0.5f + 0.5f) * fHeight;

    return Pixel;
}

float4 Compute_WorldPos(float2 vTexcoord, Texture2D DepthTexture, int3 Location, matrix ProjMatrixInv, matrix ViewMatrixInv)
{
    float4 vWorldPos = 0.f;
    
    vector vDepthDesc = DepthTexture.Load(Location);
    
    vWorldPos.x = vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    vWorldPos *= vDepthDesc.y;
    
    vWorldPos = mul(vWorldPos, ProjMatrixInv);
    vWorldPos = mul(vWorldPos, ViewMatrixInv);
    
    return vWorldPos;
}

float4 Compute_ViewPos(float2 vTexcoord, Texture2D DepthTexture, int3 Location, matrix ProjMatrixInv)
{
    float4 vViewPos = 0.f;
    
    vector vDepthDesc = DepthTexture.Load(Location);
    
    vViewPos.x = vTexcoord.x * 2.f - 1.f;
    vViewPos.y = vTexcoord.y * -2.f + 1.f;
    vViewPos.z = vDepthDesc.x;
    vViewPos.w = 1.f;
    
    vViewPos *= vDepthDesc.y;
    
    vViewPos = mul(vViewPos, ProjMatrixInv);
    
    return vViewPos;
}

float4 Compute_ViewPosTexcoord(float2 vTexcoord, Texture2D DepthTexture, sampler Sampler, matrix ProjMatrixInv)
{
    float4 vViewPos = 0.f;
    
    float4 vDepthDesc = DepthTexture.SampleLevel(Sampler, vTexcoord, 0);
    
    vViewPos.x = vTexcoord.x * 2.f - 1.f;
    vViewPos.y = vTexcoord.y * -2.f + 1.f;
    vViewPos.z = vDepthDesc.x;
    vViewPos.w = 1.f;
    
    vViewPos *= vDepthDesc.y;
    
    vViewPos = mul(vViewPos, ProjMatrixInv);
    
    return vViewPos;
}

float4 Compute_Normal(Texture2D NormalTexture, sampler Sampler, float2 vTexcoord)
{
    float4 vNormal = NormalTexture.SampleLevel(Sampler, vTexcoord, 0);
    vNormal = vector((vNormal.xyz * 2.f - 1.f), 0.f);
    
    return vNormal;
}

float Random(float2 St)
{
    return frac(sin(dot(St.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

float Noise(float2 St)
{
    float2 i = floor(St);
    float2 f = frac(St);
    
    float a = Random(i);
    float b = Random(i + float2(1.0, 0.0));
    float c = Random(i + float2(0.0, 1.0));
    float d = Random(i + float2(1.0, 1.0));
    
    float2 u = f * f * (3.0 - 2.0 * f);
    
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}
