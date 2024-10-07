struct VertexInput
{
    float4 position : POSITION;
};

struct PixelInput
{
    float4 position : SV_POSITION;
};

cbuffer ViewProjBuffer : register(b1)
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    float4 view_position = mul(view_matrix, input.position);
    float4 proj_position = mul(projection_matrix, view_position);

    output.position = view_position; // No transformation, just pass the position through
    return output;
}
