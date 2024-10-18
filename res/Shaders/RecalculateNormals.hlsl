struct Face
{
    int a;
    int b;
    int c;
};

StructuredBuffer<Face> face_buffer : register(t0);
RWStructuredBuffer<float3> position_buffer : register(u0);
RWStructuredBuffer<float3> normal_buffer : register(u1);

[numthreads(256, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    Face f = face_buffer[id.x];
    float3 a = position_buffer[f.a];
    float3 b = position_buffer[f.b];
    float3 c = position_buffer[f.c];
    
    float3 diff1 = b - a;
    float3 diff2 = b - c;
    float3 normal = normalize(cross(diff1, diff2));
    
    normal_buffer[f.a] += normal;
    normal_buffer[f.b] += normal;
    normal_buffer[f.c] += normal;
}
