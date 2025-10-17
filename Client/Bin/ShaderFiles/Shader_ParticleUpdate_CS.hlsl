struct ParticleState
{
    float4 Right;
    float4 Up;
    float4 Look;
    float4 Pos;
    float2 LifeTime;
};

struct ParticleStatic
{
    float4 DefaultPos;
    float Speed; float3 _pad0;
};


//상수버퍼
cbuffer CB : register(b0)
{
    float3 Pivot; float DeltaTime; // 16B(= float3 + float)}      16으로 맞춰줘야함 만약 float3 , 다음 float3가 또오면 float3 fTest; float _pad0; 이런식으로 맞춰줘야한다는거 같음.
    uint IsLoop;
    float SpreadWeight;
    float DropWeight;
    float RotationWeight;
    float Gravity;
    
    float3 _pad0;
}

//SRV
StructuredBuffer<ParticleStatic> g_ParticleStatic : register(t0);

//UAV
//RWStructuredBuffer<float4> g_Right : register(u0);
//RWStructuredBuffer<float4> g_Up : register(u1);
//RWStructuredBuffer<float4> g_Look : register(u2);
//RWStructuredBuffer<float4> g_Pos : register(u3);
//RWStructuredBuffer<float2> g_LifeTime : register(u4);

//UAV 구조체로 정보입력
RWStructuredBuffer<ParticleState> g_ParticleState : register(u0);

float4 Spread(float4 Pos, float Speed)
{
    float4 MoveDir = float4(normalize(Pos.xyz - Pivot), 0.f);
    
    float Speedw = Speed * SpreadWeight;
  
    float4 Position = Pos + MoveDir * Speedw * DeltaTime;
    
    return Position;
}

float4 Drop(float4 Pos, float Speed)
{
    float4 MoveDir = float4(0.f, -1.f, 0.f, 0.f);
    
    float Speedw = Speed * DropWeight;
    
    float4 Position = Pos + MoveDir * Speedw * DeltaTime;
    
    return Position;
}

float4 Rotation(float4 Pos, float Speed)
{
    float4 LocalPos = Pos - float4(Pivot.xyz, 1.f);
    
    float Speedw = Speed * RotationWeight;
    
    float fAngle = Speedw * DeltaTime;
    
    float sin = 0.f;
    float cos = 0.f;
    
    sincos(fAngle, sin, cos);
    
    LocalPos.x = (cos * LocalPos.x) - (sin * LocalPos.z);
    LocalPos.z = (sin * LocalPos.x) + (cos * LocalPos.z);
    
    float4 Position = LocalPos + float4(Pivot.xyz, 1.f);
  
    return Position;
}


[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    
    if (g_ParticleState[i].LifeTime.x <= g_ParticleState[i].LifeTime.y)
    {
    
        float4 Position = g_ParticleState[i].Pos;
        float Speed = g_ParticleStatic[i].Speed;
    
        if (SpreadWeight > 0)
        {
            Position = Spread(Position, Speed);
        }

        if (DropWeight > 0)
        {
            Position = Drop(Position, Speed);
        }
    
        if (RotationWeight > 0)
        {
            Position = Rotation(Position, Speed);
        }
    
    //여러 동작 처리들 다해서 나온 포지션 값 대입
        g_ParticleState[i].Pos = Position;
    
        g_ParticleState[i].LifeTime.x += DeltaTime;
    }
    if(IsLoop == 1)
    {
        if(g_ParticleState[i].LifeTime.x >= g_ParticleState[i].LifeTime.y)
        {
            g_ParticleState[i].LifeTime.x = 0;
            g_ParticleState[i].Pos = g_ParticleStatic[i].DefaultPos;
        }
    }
    else
    {
        if (g_ParticleState[i].LifeTime.x >= g_ParticleState[i].LifeTime.y)
        {
            float4 DeltaPosition = float4(g_ParticleStatic[i].DefaultPos.xyz, 1.f);
            
            DeltaPosition.xy += Gravity * DeltaTime * g_ParticleStatic[i].Speed;
           
            g_ParticleState[i].Pos.xy -= DeltaPosition * DeltaTime;
        }
    }
}


