struct Vertex
{
    float4 position;
    float4 normal;
};

StructuredBuffer<Vertex> readBuffer : register(t5);
RWStructuredBuffer<Vertex> vertexBuffer : register(u5);

[numthreads(1024, 1, 1)]
void CSMain(uint3 dispatchID : SV_DispatchThreadID)
{
    uint idx = dispatchID.x;

    Vertex v = readBuffer[idx];

    v.position += float4(0.0, -0.1, 0.0, 0.0);
    vertexBuffer[idx] = v;
}
