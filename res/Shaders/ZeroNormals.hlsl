struct Vertex
{
    float3 position;
    float3 normal;
};

RWStructuredBuffer<Vertex> output_buffer : register(u0);

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    output_buffer[id.x].normal = float3(0.0, 0.0, 0.0);
}
