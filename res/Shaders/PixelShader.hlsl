cbuffer ColorBuffer : register(b0)
{
    float4 color;  
};

float4 PSMain() : SV_TARGET
{
    return color;
}
