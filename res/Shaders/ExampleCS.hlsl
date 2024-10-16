struct Vertex
{
    float3 position;
    float3 normal;
};

RWStructuredBuffer<Vertex> output_buffer : register(u0);

cbuffer TimeBuffer : register(b0)
{
    float time;
    float3 padding;
};

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{   
    float3 pos = output_buffer[id.x].position;
    
    output_buffer[id.x].position.z = sin(pos.x * 6.28 + time) * 0.1;
}
