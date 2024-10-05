struct VertexInput
{
    float4 position : POSITION;
};

struct PixelInput
{
    float4 position : SV_POSITION;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = input.position; // No transformation, just pass the position through
    return output;
}
