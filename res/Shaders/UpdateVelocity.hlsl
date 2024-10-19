RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float3> previous_position : register(u1);
RWStructuredBuffer<float3> velocity : register(u2);
RWStructuredBuffer<float3> jacobi : register(u3);

cbuffer DeltaTime : register(b0)
{
    float dt;
    float idt;
    float2 padding_dt;
};

cbuffer Gravity : register(b1)
{
    float3 fg;
    float padding_g;
};

cbuffer Mass : register(b2)
{
    float mass;
    float imass;
    float2 padding_m;
};

cbuffer Resolution : register(b3)
{
    uint resolution;
    uint z_multiplier;
    float2 padding_r;
}

[numthreads(32, 8, 1)]
void CSMain(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    uint sz = z_multiplier * gid.z;
    uint sx = gid.x * 32;
    uint sy = gid.y * 8;
    uint gx = sx + tid.x;
    uint gy = sy + tid.y;
    int i = sz + gx + gy * resolution;
    
    position[i] += 0.25 * jacobi[i];
    velocity[i] = (position[i] - previous_position[i]) * idt * 0.99975;
}
