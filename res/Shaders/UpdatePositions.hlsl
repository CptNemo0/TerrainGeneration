struct Vertex
{
    float3 position;
    float3 normal;
};

RWStructuredBuffer<Vertex> output_buffer : register(u0);
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

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    velocity[id.x] += dt * dt * fg * imass;
    output_buffer[id.x].position += dt * velocity[id.x];
}
