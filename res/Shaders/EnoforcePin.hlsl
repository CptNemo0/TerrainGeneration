struct Vertex
{
    float3 position;
    float3 normal;
};

struct PinConstraint
{
    float x, y, z;
    int idx;
};

RWStructuredBuffer<Vertex> output_buffer : register(u0);
StructuredBuffer<PinConstraint> constraints : register(t0);

cbuffer DeltaTime : register(b0)
{
    float dt;
    float idt;
    float2 padding_dt;
};

cbuffer Compliance : register(b1)
{
    float alpha;
    float3 padding_c;
};

cbuffer Mass : register(b2)
{
    float mass;
    float imass;
    float2 padding_m;
};

[numthreads(1, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    PinConstraint constraint = constraints[id.x];
    output_buffer[constraint.idx].position = float3(constraint.x, constraint.y, constraint.z);
}
