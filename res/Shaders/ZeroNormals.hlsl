struct Vertex
{
    float3 position;
    float3 normal;
};

RWStructuredBuffer<float3> normal_buffer : register(u0);

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    normal_buffer[id.x] = float3(0.0, 0.0, 0.0);
}
