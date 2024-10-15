struct Vertex
{
    float3 position;
    float3 normal;
};

StructuredBuffer<Vertex> input_buffer : register(t0);
RWStructuredBuffer<Vertex> output_buffer : register(u0);

float rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719))
{
	//make value smaller to avoid artefacts
    float3 smallValue = sin(value);
	//get scalar value from 3d vector
    float random = dot(smallValue, dotDir);
	//make value more random by making it bigger and then taking the factional part
    random = frac(sin(random) * 143758.5453);
    return random;
}


[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{   
    if (id.x >= 1024)
    {
        return;
    }
    float3 pos = output_buffer[id.x].position;

    output_buffer[id.x].position.z += rand3dTo1d(pos) * 0.001f;
}
