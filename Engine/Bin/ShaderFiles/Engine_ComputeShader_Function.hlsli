#pragma pack_matrix(row_major)

// Emissive ÃÖ¼ÒÄ¡

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
    if(vDepthDesc.y == 0.f)
        return vViewPos;
    
    
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
    
    return normalize(vNormal);
}

float4 Compute_Normal_DTID(Texture2D NormalTexture, int3 DTID)
{
    float4 vNormal = NormalTexture.Load(DTID);
    vNormal = vector((vNormal.xyz * 2.f - 1.f), 0.f);
    
    return normalize(vNormal);
}

float4 Compute_SSAO_Blur(float4 vOriginColor, float fOriginDepth, float4 vOriginNormal, float4 vSampleColor, float fSampleDepth, float4 vSampleNormal, float fMinDepthDistance, inout float fWeight)
{
    float4 vColor = 1.f;
    
    if(fSampleDepth == 0.f || vSampleColor.r == 1.f)
    {
        vColor = vOriginColor;
    }
    
    float fDepthDist = abs(fOriginDepth - fSampleDepth);
    
    float fNormalWeight = saturate(dot(vOriginNormal, vSampleNormal));
    
    if (fDepthDist <= fMinDepthDistance)
    {
        vColor = vSampleColor;// * ((1.f - fNormalWeight));
    }
    
    fWeight += (1.f - fNormalWeight);
    
    return vColor;
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

float4 DownScale(int2 vIndex, int2 vInSize, Texture2D<float4> InputTexture)
{
    float4 vColor = 0.f;
    
    int iSampleX0 = min(vIndex.x, vInSize.x - 1);
    int iSampleX1 = min(vIndex.x + 1, vInSize.x - 1);
    
    int iSampleY0 = min(vIndex.y, vInSize.y - 1);
    int iSampleY1 = min(vIndex.y + 1, vInSize.y - 1);
    
    vColor += InputTexture.Load(int3(iSampleX0, iSampleY0, 0));
    vColor += InputTexture.Load(int3(iSampleX1, iSampleY0, 0));
    vColor += InputTexture.Load(int3(iSampleX0, iSampleY1, 0));
    vColor += InputTexture.Load(int3(iSampleX1, iSampleY1, 0));
    
    vColor *= 0.25f;
   
    return vColor;
}

float4 UpScale(int2 DTID, int2 vInSize, int2 vOutSize, Texture2D<float4> InputTexture)
{
    float2 vTexcoord = float2(DTID.xy);
    
    float2 vLowPos = (vTexcoord + 0.5f) * ((float2) vInSize / (float2) vOutSize) - 0.5f;
    
    int2 iLowID = (int2) floor(vLowPos);
    float2 fFrac = vLowPos - (float2) iLowID;
    float4 vColor = 0.f;
       
    int iSampleX0 = min(iLowID.x, vInSize.x - 1);
    int iSampleX1 = min(iLowID.x + 1, vInSize.x - 1);
    
    int iSampleY0 = min(iLowID.y, vInSize.y - 1);
    int iSampleY1 = min(iLowID.y + 1, vInSize.y - 1);
       
    float4 vLT = InputTexture.Load(int3(iSampleX0, iSampleY0, 0));
    float4 vRT = InputTexture.Load(int3(iSampleX1, iSampleY0, 0));
    float4 vLB = InputTexture.Load(int3(iSampleX0, iSampleY1, 0));
    float4 vRB = InputTexture.Load(int3(iSampleX1, iSampleY1, 0));
    
    vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
    
    return vColor;
}