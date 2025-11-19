typedef row_major matrix matrix_rm;

// --- Resources (CModel::Ready_GPU_Buffers에서 생성한 버퍼들) ---
#define THREAD_X 64
#define THREAD_Y 1
#define THREAD_Z 1

// 입력(Input) 버퍼들
StructuredBuffer<matrix_rm> g_LocalMatrices : register(t0);
StructuredBuffer<int> g_ParentBoneIndex : register(t1);
// 출력(Output) 버퍼 - 이제 '로컬' 행렬을 출력합니다.
RWStructuredBuffer<matrix_rm> g_OutModelMatrices : register(u0);

cbuffer InstanceCB : register(b0)
{
    uint g_NumBones;
    uint g_NumInstance;
    uint g_Temp;
    float g_TempFloat;
}
cbuffer PreTrancformR : register(b1)
{
    float g_fRightX;
    float g_fRightY;
    float g_fRightZ;
    float g_fRightW;
}

cbuffer PreTrancformU : register(b2)
{
    float g_fUpX;
    float g_fUpY;
    float g_fUpZ;
    float g_fUpW;
}

cbuffer PreTrancformL : register(b3)
{
    float g_fLookX;
    float g_fLookY;
    float g_fLookZ;
    float g_fLookW;
}
cbuffer PreTrancformP : register(b4)
{
    float g_fPosX;
    float g_fPosY;
    float g_fPosZ;
    float g_fPosW;
}

matrix_rm matrix_rmFromSQT(float4 s, float4 q, float4 t)
{
    matrix_rm m;
    float qx = q.x, qy = q.y, qz = q.z, qw = q.w;

    m._11 = s.x * (1 - 2 * qy * qy - 2 * qz * qz);
    m._12 = s.x * (2 * qx * qy + 2 * qw * qz);
    m._13 = s.x * (2 * qx * qz - 2 * qw * qy);
    m._14 = 0;

    m._21 = s.y * (2 * qx * qy - 2 * qw * qz);
    m._22 = s.y * (1 - 2 * qx * qx - 2 * qz * qz);
    m._23 = s.y * (2 * qy * qz + 2 * qw * qx);
    m._24 = 0;

    m._31 = s.z * (2 * qx * qz + 2 * qw * qy);
    m._32 = s.z * (2 * qy * qz - 2 * qw * qx);
    m._33 = s.z * (1 - 2 * qx * qx - 2 * qy * qy);
    m._34 = 0;

    m._41 = t.x;
    m._42 = t.y;
    m._43 = t.z;
    m._44 = 1;
	
    return m;

}

row_major float4x4 Skinning(matrix_rm localMatrix, int iGlobalBoneIndex)
{
    int iBoneIndex = iGlobalBoneIndex;
    int iInstanceIndex = iGlobalBoneIndex / g_NumBones;
    if ((iBoneIndex % g_NumBones) == 0)
        return localMatrix;
    while (g_ParentBoneIndex[iBoneIndex % g_NumBones] != -1)
    {
        int parentIndex = iInstanceIndex * g_NumBones + g_ParentBoneIndex[iBoneIndex % g_NumBones];
        
        row_major float4x4 parentMatrix = g_LocalMatrices[parentIndex];
        localMatrix = mul(parentMatrix, localMatrix);
        iBoneIndex = parentIndex;
    }
    return localMatrix;
}

[numthreads(THREAD_X, THREAD_Y, THREAD_Z)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint iGlobalIndex = dispatchThreadID.x;
    matrix result_matrix = Skinning(g_LocalMatrices[iGlobalIndex], iGlobalIndex);
    row_major matrix m;
    m._11_12_13_14 = float4(g_fRightX, g_fRightY, g_fRightZ, g_fRightW);
    m._21_22_23_24 = float4(g_fUpX, g_fUpY, g_fUpZ, g_fUpW);
    m._31_32_33_34 = float4(g_fLookX, g_fLookY, g_fLookZ, g_fLookW);
    m._41_42_43_44 = float4(g_fPosX, g_fPosY, g_fPosZ, g_fPosW);
    
    g_OutModelMatrices[iGlobalIndex] = mul(m, result_matrix);
}