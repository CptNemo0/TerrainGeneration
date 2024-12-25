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

cbuffer ModelMatrix : register(b3)
{
    float4x4 model_matrix;
    float4x4 ti_model_matrix;
};

StructuredBuffer<float3> position_buffer : register(t0);
StructuredBuffer<float3> normal_buffer : register(t1);

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
     
    float4 view_position = mul(view_matrix, input.position);
    float4 proj_position = mul(projection_matrix, view_position);

    float4 difference = camera_position - input.position;
    
    float4 view_direction = normalize(difference);
    float view_dot_product = dot(view_direction.xyz, input.normal.xyz);
    
    if (view_dot_product < 0)
    {
        input.normal.xyz *= -1.0;
    }
    
    output.projected_position = proj_position;
    output.world_position = input.position;
    output.normal = input.normal;
    
    return output;
}



PixelOutput PSMain(PixelInput input)
{
    PixelOutput output;
    output.color = float4(color.rgb, 1.0);
    output.position = input.world_position;

    output.normal = input.normal * 0.5 + 0.5;
    return output;
}
