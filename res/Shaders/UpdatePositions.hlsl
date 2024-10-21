RWStructuredBuffer<float3> position_buffer : register(u0);
RWStructuredBuffer<float3> previous_position : register(u1);
RWStructuredBuffer<float3> velocity : register(u2);
RWStructuredBuffer<uint3> jacobi : register(u3);

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

cbuffer Resolution : register(b4)
{
    uint resolution;
    uint z_multiplier;
    float2 padding_r;
}

float rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719))
{
    return frac(sin(value * 143758.5453));
}

[numthreads(32, 8, 1)]
void CSMain(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{   
    uint sz = z_multiplier * gid.z;
    uint sx = gid.x * 32;
    uint sy = gid.y * 8;
    uint gx = sx + tid.x;
    uint gy = sy + tid.y;
    int i = sz + gx + gy * resolution;
    
    float wind_s = rand3dTo1d(position_buffer[i]);
    wind_s *= wind_s * wind_s;

    velocity[i] += dt * dt * imass * (fg + wind_s * w_strength_multiplier * w_direction);
    previous_position[i] = position_buffer[i];    
    position_buffer[i] += dt * velocity[i];
    jacobi[i] = uint3(0.0, 0.0, 0.0);

}
