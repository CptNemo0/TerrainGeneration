struct Vertex
{
    float3 position;
    float3 normal;
};

struct Face
{
    int a;
    int b;
    int c;
};

StructuredBuffer<Face> face_buffer : register(t0);
RWStructuredBuffer<Vertex> output_buffer : register(u0);

[numthreads(256, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    Face f = face_buffer[id.x];
    Vertex a = output_buffer[f.a];
    Vertex b = output_buffer[f.b];
    Vertex c = output_buffer[f.c];
    
    float3 diff1 = b.position - a.position;
    float3 diff2 = b.position - c.position;
    float3 normal = normalize(cross(diff1, diff2));
    
    output_buffer[f.a].normal += normal;
    output_buffer[f.b].normal += normal;
    output_buffer[f.c].normal += normal;
}
