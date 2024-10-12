#pragma vs VSMain

struct VertexInput
{
    float4 position : POSITION;
    float4 normal : NORMAL;
};

struct PixelInput
{
    float4 projected_position : SV_POSITION;
    float4 world_position : POSITION1;
    float4 normal : NORMAL;
    
};

struct PixelOutput
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 color : SV_Target2;
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

cbuffer CameraBuffer : register(b3)
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
    float4 view_position = mul(view_matrix, world_position);
    float4 proj_position = mul(projection_matrix, view_position);

    float4 difference = camera_position - input.position;
    
    float4 view_direction = normalize(difference);
    float view_dot_product = dot(view_direction.xyz, input.normal.xyz);
    
    float4 N = mul(ti_model_matrix, input.normal);
    N = normalize(N);
    
    if (view_dot_product < 0)
    {
        N = -N;
    }
    
    output.projected_position = proj_position;
    output.world_position = world_position;
    output.normal = N;
    
    return output;
}

PixelOutput PSMain(PixelInput input)
{
    PixelOutput output;
    output.color = float4(color.rgb, 1.0);
    output.position = input.world_position;
    output.normal = input.normal * 0.5 + 0.5;
    output.normal.a = 1.0f;
    return output;
}
