#include "Engine_ComputeShader_Function.hlsli"

#pragma pack_matrix(row_major)

#define THREAD_X 16
#define THREAD_Y 16
#define THREAD_Z 1

#define MAX_RADIUS 15
#define BLUR_RADIUS 6

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

cbuffer BLUR_DATA : register(b0)
{
    float2 fOutSize;
    int iRadius;
    float Paddingblur;
}

StructuredBuffer<float> g_Weights : register(t1);

groupshared float4 vSharedColorX[THREAD_Y][THREAD_X + (2 * MAX_RADIUS)];
groupshared float4 vSharedColorY[THREAD_Y + (2 * MAX_RADIUS)][THREAD_X];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur_X(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = iRadius;
    
    vSharedColorX[GTID.y][GTID.x + iRadius] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x < iRadius)
    {
        int3 LeftID = int3(DTID.x - iRadius, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= (int) fOutSize.x)
            RightID.x = (int) fOutSize.x - 1;
            
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorX[GTID.y][GTID.x + THREAD_X + iRadius] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float4 vColorX = 0.f;
    
    for (int i = -iRadius; i <= iRadius; ++i)
    {
        int iIndexX = GTID.x + iRadius + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        
        vColorX += vSampleColor * g_Weights[i + iRadius];
    }
    
    OutputTexture[DTID.xy] = vColorX;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void GaussianBlur_Y(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float fBlurRadius = (float) iRadius;
    
    vSharedColorY[GTID.y + iRadius][GTID.x] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.y < iRadius)
    {
        int3 LeftID = int3(DTID.x, DTID.y - iRadius, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= (int) fOutSize.y)
            RightID.y = (int) fOutSize.y - 1;
            
        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorY[GTID.y + THREAD_Y + iRadius][GTID.x] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 vColorY = 0.f;
    
    for (int j = -iRadius; j <= iRadius; ++j)
    {
        int iIndexY = GTID.y + iRadius + j;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];
        
        vColorY += vSampleColor * g_Weights[j + iRadius];
    }
    
    OutputTexture[DTID.xy] = vColorY;
}


/* ------------------------------------DOF------------------------------------ */

Texture2D<float4> DepthTexture : register(t1);

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void DOF_X(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float4 vDofData = DepthTexture.Load(int3(DTID.xy, 0));
    
    int2 vOutSize = 0;
    OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    
    vSharedColorX[GTID.y][GTID.x + MAX_RADIUS] = InputTexture.Load(int3(DTID.xy, 0));
    
    if (GTID.x < MAX_RADIUS)
    {
        int3 LeftID = int3(DTID.x - MAX_RADIUS, DTID.y, 0);
        int3 RightID = int3(DTID.x + THREAD_X, DTID.y, 0);
        
        if (LeftID.x < 0)
            LeftID.x = 0;
        
        if (RightID.x >= (int) vOutSize.x)
            RightID.x = (int) vOutSize.x - 1;
            
        vSharedColorX[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorX[GTID.y][GTID.x + THREAD_X + MAX_RADIUS] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float fCoc = vDofData.x;
    
    int iDofRadius = fCoc * (float) MAX_RADIUS;
    
    if(fCoc <= vDofData.z)
    {
        OutputTexture[DTID.xy] = vSharedColorX[GTID.y][GTID.x + MAX_RADIUS];
        return;
    }
    
    float fGaussianSigma = (float) iDofRadius / 3.f;
    
    float4 vColorX = 0.f;
    float fTotalWeight = 0.f;
    for (int i = -iDofRadius; i <= iDofRadius; ++i)
    {
        int iIndexX = GTID.x + MAX_RADIUS + i;
        
        float4 vSampleColor = vSharedColorX[GTID.y][iIndexX];
        
        float fGaussianWeight = exp(-(i * i) / (2.f * fGaussianSigma * fGaussianSigma));
        
        vColorX += vSampleColor * fGaussianWeight;
        fTotalWeight += fGaussianWeight;
    }
    
    OutputTexture[DTID.xy] = vColorX / fTotalWeight;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void DOF_Y(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    float4 vDofData = DepthTexture.Load(int3(DTID.xy, 0));
    
    int2 vOutSize = 0;
    OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    
    vSharedColorY[GTID.y + MAX_RADIUS][GTID.x] = InputTexture.Load(int3(DTID.xy, 0));

    if (GTID.y < MAX_RADIUS)
    {
        int3 LeftID = int3(DTID.x, DTID.y - MAX_RADIUS, 0);
        int3 RightID = int3(DTID.x, DTID.y + THREAD_Y, 0);
        
        if (LeftID.y < 0)
            LeftID.y = 0;
        
        if (RightID.y >= (int) vOutSize.y)
            RightID.y = (int) vOutSize.y - 1;
            
        vSharedColorY[GTID.y][GTID.x] = InputTexture.Load(LeftID);
        vSharedColorY[GTID.y + THREAD_Y + MAX_RADIUS][GTID.x] = InputTexture.Load(RightID);
    }
    
    GroupMemoryBarrierWithGroupSync();

    float fCoc = vDofData.x;
    
    int iDofRadius = fCoc * (float) MAX_RADIUS;
    
    if (fCoc <= vDofData.z)
    {
        OutputTexture[DTID.xy] = vSharedColorY[GTID.y + MAX_RADIUS][GTID.x];
        return;
    }
    
    float fGaussianSigma = (float) iDofRadius / 3.f;
    
    float4 vColorY = 0.f;
    float fTotalWeight = 0.f;
    for (int i = -iDofRadius; i <= iDofRadius; ++i)
    {
        int iIndexY = GTID.y + MAX_RADIUS + i;
        
        float4 vSampleColor = vSharedColorY[iIndexY][GTID.x];

        float fGaussianWeight = exp(-(i * i) / (2.f * fGaussianSigma * fGaussianSigma));

        vColorY += vSampleColor * fGaussianWeight;
        fTotalWeight += fGaussianWeight;
    }
    
    OutputTexture[DTID.xy] = vColorY / fTotalWeight;
}

/* ------------------------------------MOTION-BLUR------------------------------------ */

Texture2D<float4> VelocityMap : register(t2);

SamplerState CS_DefaultSampler : register(s0);


cbuffer MOTION_DATA : register(b1)
{
    float fLimitVelocity;
    float fLimitDepth;
    float fLengthScale;
    float PaddingMotion;
}

float3 Compute_Velocity(int2 vIndex, int2 vVelocitySize)
{
    float3 vVelocity = 0.f;
    
    int iSampleX0 = min(vIndex.x, vVelocitySize.x - 1);
    int iSampleX1 = min(vIndex.x + 1, vVelocitySize.x - 1);
    
    int iSampleY0 = min(vIndex.y, vVelocitySize.y - 1);
    int iSampleY1 = min(vIndex.y + 1, vVelocitySize.y - 1);
    
    float3 Vector[4];
    
    Vector[0] = VelocityMap.Load(int3(iSampleX0, iSampleY0, 0)).xyz;
    Vector[1] = VelocityMap.Load(int3(iSampleX1, iSampleY0, 0)).xyz;
    Vector[2] = VelocityMap.Load(int3(iSampleX0, iSampleY1, 0)).xyz;
    Vector[3] = VelocityMap.Load(int3(iSampleX1, iSampleY1, 0)).xyz;
   
    float fMaxLength = 0.f;
    
    for (int i = 0; i < 4; ++i)
    {
        float fLength = length(Vector[i].xy);
        
        if(fLength == 0.f)
            return 0.f;
            
        if(fLength > fMaxLength)
        {
            fMaxLength = fLength;
            vVelocity = Vector[i];
        }
    }
   
    return vVelocity;
}

float4 ComputeMotionBlur(uint3 DTID, int2 vInSize, int2 vOutSize)
{
    int2 iIndex = DTID.xy * 2;
    
    float3 vVelocity = Compute_Velocity(iIndex, vOutSize);
    
    float2 vDir = normalize(vVelocity.xy) * - 1.f;
    
    float fVelocityLength = length(vVelocity.xy) * fLengthScale;

    fVelocityLength *= smoothstep(fLimitDepth, 0.f, vVelocity.z);
    
    float2 vTexcoord = (float2) DTID.xy / (float2) vInSize;
    
    //if (fVelocityLength <= fLimitVelocity)
    //    return InputTexture.SampleLevel(CS_DefaultSampler, vTexcoord, 0);
    
    float2 vTexelSize = 1.f / (float2) vInSize;
    float2 vMotionScale = vDir * fVelocityLength * vTexelSize;
    int iSampleCount = 15;
    
    float4 vColor = 0.f;
    float fTotalWeight = 0.f;
    
    for (int i = 1; i <= iSampleCount; ++i)
    {
        float2 vDistance = (float) i * vTexelSize;
        float2 vOffset = vMotionScale * vDistance;
        
        float fSampleDepth = DepthTexture.SampleLevel(CS_DefaultSampler, vTexcoord + vOffset, 0).y;

        if (fSampleDepth < vVelocity.z)
            continue;
            
        float4 vSampleColor = InputTexture.SampleLevel(CS_DefaultSampler, vTexcoord + vOffset, 0);
        
        float fWeight = exp2(-(float) i / (float) iSampleCount * 3.f);
        //float fWeight = 1.f - (float) i / 15.f;

        vColor += vSampleColor * fWeight;
        fTotalWeight += fWeight;
    }
    
    float4 vFinalColor = 0.f;
    
    if(fTotalWeight > 0.f)
        vFinalColor = vColor / fTotalWeight; 
    else
        vFinalColor = InputTexture.SampleLevel(CS_DefaultSampler, vTexcoord, 0);
   
   vFinalColor.a = 1.f;
    
    return vFinalColor;
}


groupshared float4 vSharedMotionColor[THREAD_Y + 1][THREAD_X + 1];

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void Motion_Blur(uint3 GruopID : SV_GroupID, uint3 DTID : SV_DispatchThreadID, uint3 GTID : SV_GroupThreadID, uint GruopIndex : SV_GroupIndex)
{
    int2 vInSize;
    InputTexture.GetDimensions(vInSize.x, vInSize.y);
    
    int2 vOutSize;
    OutputTexture.GetDimensions(vOutSize.x, vOutSize.y);
    
    vSharedMotionColor[GTID.y][GTID.x] = ComputeMotionBlur(DTID, vInSize, vOutSize);

    if (GTID.y  == THREAD_Y - 1)
    {
        int3 OffsetID = int3(DTID.x, DTID.y + 1, 0);
        if (OffsetID.y >= (int) vInSize.y)
            OffsetID.y = (int) vInSize.y - 1;
            
        vSharedMotionColor[GTID.y + 1][GTID.x] = ComputeMotionBlur(OffsetID, vInSize, vOutSize);
    }
    
    if (GTID.x >= THREAD_X - 1)
    {
        int3 OffsetID = int3(DTID.x + 1, DTID.y, 0);
        if (OffsetID.x >= (int) vInSize.x)
            OffsetID.x = (int) vInSize.x - 1;
            
        vSharedMotionColor[GTID.y][GTID.x + 1] = ComputeMotionBlur(OffsetID, vInSize, vOutSize);
    }
    
    if (GTID.x >= THREAD_X - 1 && GTID.y >= THREAD_Y -1 )
    {
        int3 OffsetID = int3(DTID.x + 1, DTID.y + 1, 0);
        
        if (OffsetID.x >= (int) vInSize.x)
            OffsetID.x = (int) vInSize.x - 1;
            
        if (OffsetID.y >= (int) vInSize.y)
            OffsetID.y = (int) vInSize.y - 1;
            
        vSharedMotionColor[GTID.y + 1][GTID.x + 1] = ComputeMotionBlur(OffsetID, vInSize, vOutSize);
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    
    for (int i = 0; i < 4; i++)
    {   
        int2 OutIndex = DTID.xy * 2;
        float2 vTexcoord = float2(GTID.xy);
        
        int2 Offset = int2(i % 2, clamp(i - 1, 0, 1));
        
        OutIndex += Offset;
        
        vTexcoord += Offset;
    
        float2 vLowPos = (OutIndex + 0.5f) * ((float2) vInSize / (float2) vOutSize) - 0.5f;
    
        int2 iLowID = (int2) floor(vLowPos);
        float2 fFrac = vLowPos - (float2) iLowID;
        
        float4 vColor = 0.f;
       
        int iSampleX0 = clamp(vTexcoord.x, 0, THREAD_X); //min(iLowID.x, THREAD_X + 1);
        int iSampleX1 = clamp(vTexcoord.x + 1, 0, THREAD_X); //min(iLowID.x + 1, THREAD_X + 1);
    
        int iSampleY0 = clamp(vTexcoord.y, 0, THREAD_Y); //min(iLowID.y, THREAD_Y + 1);
        int iSampleY1 = clamp(vTexcoord.y + 1, 0, THREAD_Y); //min(iLowID.y + 1, THREAD_Y + 1);
       
        float4 vLT = vSharedMotionColor[iSampleY0][iSampleX0];
        float4 vRT = vSharedMotionColor[iSampleY0][iSampleX1];
        float4 vLB = vSharedMotionColor[iSampleY1][iSampleX0];
        float4 vRB = vSharedMotionColor[iSampleY1][iSampleX1];
    
        vColor = lerp(lerp(vLT, vRT, fFrac.x), lerp(vLB, vRB, fFrac.x), fFrac.y);
        
        OutputTexture[OutIndex] = vColor;
    }
}