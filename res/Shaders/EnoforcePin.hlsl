struct PinConstraint
{
    float3 position;
    int idx;
};

RWStructuredBuffer<float3> position_buffer : register(u0);
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

cbuffer PinBitmaskBuffer : register(b3)
{
    int mask;
    int3 padding;
}

[numthreads(1, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if ((mask >> id.x) & 1)
    {
        position_buffer[constraints[id.x].idx] = constraints[id.x].position;
    }
}
