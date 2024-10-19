RWStructuredBuffer<float3> position : register(u0);
RWStructuredBuffer<float3> previous_position : register(u1);
RWStructuredBuffer<float3> velocity : register(u2);

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

[numthreads(32, 32, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    int i = id.x * 32 + id.y;
    velocity[i] = (position[i] - previous_position[i]) * idt * 0.99975;
}
