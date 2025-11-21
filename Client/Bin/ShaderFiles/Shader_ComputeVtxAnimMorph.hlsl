#define THREAD_X 256
#define THREAD_Y 1
#define THREAD_Z 1

struct MorphDelta
{
    float3 vPosDelta;    // 위치 변화량
    float3 vNormalDelta; // 노멀 변화량
};

// 렌더링 파이프 라인으로 넘겨질 최종 정점 정보.
struct OutputVertex
{
    float3 vPosition;
    float3 vNormal;
};

// 변형되지 않은 원본 정점 데이터
struct BaseVertex
{
    float3 vPosition;
    float3 vNormal;
};

// 1. 원본 정점 데이터 (Read - Only)
StructuredBuffer<BaseVertex> g_BaseVertices : register(t0);

// 2. 모든 쉐이프 키의 Delta Data가 일렬로 담긴 버퍼
StructuredBuffer<MorphDelta> g_AllMorphDeltas : register(t1);

// 3. 현재 프레임의 Shape Key 가중치 => CPU에서 계산해서 넘겨줍니다.
StructuredBuffer<float> g_MorphWeights : register(t2);

// 4. 최종 결과물 RW
RWStructuredBuffer<OutputVertex> g_OutVertices : register(u0);

// 5. Constant Buffer
cbuffer MorphInfoCB : register(b0)
{
    uint g_NumVertices;     // 전체 정점 개수
    uint g_NumActiveMorphs; // 현재 활성화된 쉐이프 키 개수.
    float2 vPadding;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) // SV_DispatchThreadID : 전체 작업에서의 스레드 ID
{
    
    // 1. 현재 스레드가 처리할 정점 인덱스.
    uint iVertexID = dispatchThreadID.x;
    
    // 2. 범위 체크.
    if (iVertexID >= g_NumVertices)
        return;
    
    // 3. 원본 정점 정보 가져오기.
    BaseVertex baseVert = g_BaseVertices[iVertexID];
    
    float3 finalPos    = baseVert.vPosition;
    float3 finalNormal = baseVert.vNormal;
    
    // 4. 활성화된 모든 Morph Target을 순회하고 누적합니다.
    for (uint i = 0; i < g_NumActiveMorphs; ++i)
    {
        float fWeight = g_MorphWeights[i];
        
        if (fWeight < 0.0001f)
            continue;
        
        // 데이터 접근 인덱스 계산.
        uint deltaIndex = (i * g_NumVertices) + iVertexID;
        
        MorphDelta delta = g_AllMorphDeltas[deltaIndex]; // 전역인덱스로 접근한다는 관점.
        
        finalPos += delta.vPosDelta * fWeight;
        finalNormal += delta.vNormalDelta * fWeight;
    }
    
    // 5. 결과 저장.
    OutputVertex result;
    result.vPosition = finalPos;
    result.vNormal = normalize(finalNormal); 

    g_OutVertices[iVertexID] = result;
}

//[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
//void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) // SV_DispatchThreadID : 전체 작업에서의 스레드 ID
//{
//    uint iVertexID = dispatchThreadID.x;
//    if (iVertexID >= g_TotalVerts)
//        return;

//    // 1. 기본 위치 가져오기 => 원본 위치에서 노말을 가져옵니다.
//    float3 vPos = g_OriginPos[iVertexID];
//    float3 vNormal = g_OriginNormal[iVertexID];

//    // 2. 누적 : 활성화된 모든 쉐이프 키의 변화량을 더함.
//    for (int i = 0; i < g_NumShapeKeys; ++i)
//    {
//        uint vecIndex = i / 4;
//        uint compIndex = i % 4;
        
//        //float fWeight = g_MorphWeights[i];
//        float fWeight = g_MorphWeights[vecIndex][compIndex];
        
//        if (fWeight <= 0.001f)
//            continue;

//        // 변화량 누적.
//        uint iBufferIndex = (i * g_TotalVerts) + iVertexID;
//        vPos += g_MorphDeltaPositions[iBufferIndex] * fWeight;
//        vNormal += g_MorphDeltaNormals[iBufferIndex] * fWeight;
//    }

//    // 3. 결과 저장 (RWBuffer)
//    g_OutMorphedPos[iVertexID] = vPos;
//    g_OutMorphedNormal[iVertexID] = normalize(vNormal); // Normal은 합산 후 정규화.
//}