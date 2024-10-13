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

cbuffer ColorBuffer : register(b0)
{
    float4 color;
};

cbuffer ViewProjBuffer : register(b1)
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

cbuffer CameraBuffer : register(b2)
{
    float4 camera_position;
};

cbuffer ModelMatrixBuffer : register(b3)
{
    float4x4 model_matrix;
    float4x4 ti_model_matrix;
};

cbuffer GridBuffer : register(b4)
{
    float offset;
    float width;
    float2 padding;
};

cbuffer SpotlightBuffer : register(b5)
{
    float4 sl_position;
    float4 sl_direction;
    float4 sl_diffuse_color;
    float4 sl_specular_color;
    float sl_cut_off;
    float sl_outer_cut_off;
    float sl_intensity;
    float sl_padding;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    float4 world_position = mul(model_matrix, input.position);
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
