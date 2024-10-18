#pragma vs VSMain

struct VertexInput
{
    float4 position : POSITION;
    float4 normal : NORMAL;
};

struct PixelInput
{
    float4 projected_position : SV_POSITION;
};

struct PixelOutput
{
    float4  depth: SV_Target0;
    float4 position : SV_Target1;
};

cbuffer ViewProjBuffer : register(b0)
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

cbuffer ModelMatrixBuffer : register(b1)
{
    float4x4 model_matrix;
    float4x4 ti_model_matrix;
};

StructuredBuffer<float3> position_buffer : register(t0);

PixelInput VSMain(VertexInput input, uint id : SV_VertexID)
{
    PixelInput output;
    float4 position = float4(position_buffer[id], 1.0);
    float4 world_position = mul(model_matrix, position);
    float4x4 light_space_matrix = mul(projection_matrix, view_matrix);
    output.projected_position = mul(light_space_matrix, world_position);
    return output;
}

PixelOutput PSMain(PixelInput input) : SV_TARGET
{
    PixelOutput output;
    output.depth    = float4(input.projected_position.z, input.projected_position.z, input.projected_position.z, 1.0);
    output.position = input.projected_position;
    return output;
}
