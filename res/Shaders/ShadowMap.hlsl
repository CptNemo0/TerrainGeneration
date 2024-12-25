#pragma vs VSMain

struct VertexInput
{
    float4 position : POSITION;
    float4 normal : NORMAL;
};

struct PixelInput
{
    float4 projected_position : SV_POSITION;
    float dst : DEPTH0;
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

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    float4 view_position = mul(view_matrix, input.position);
    float4 proj_position = mul(projection_matrix, view_position);
    output.projected_position = proj_position;
    
    float a = float4(0.0, 1000.0, 0.0, 0.0);
    output.dst = distance(input.position, a);
    
    return output;
}

PixelOutput PSMain(PixelInput input) : SV_TARGET
{
    PixelOutput output;
    float z = input.projected_position.z;
    z = log2(0.25 * z + 0.25) + 2;
    output.depth = float4(input.dst, 1.0, 1.0, 1.0);
    output.position = input.projected_position;
    return output;
}
