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
    float t;
    float padding_dt;
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

cbuffer Wind : register(b3)
{
    float w_strength_multiplier;
    float3 w_direction;
}

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
    float wind_s = rand3dTo1d(output_buffer[id.x].position + t);
   
    velocity[id.x] += dt * dt * imass * (fg + w_direction * w_strength_multiplier * wind_s);
    velocity[id.x] *= 0.999;
    output_buffer[id.x].position += dt * velocity[id.x];
}
