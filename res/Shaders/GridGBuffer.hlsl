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

cbuffer ViewProjBuffer : register(b0)
{
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

cbuffer CameraBuffer : register(b1)
{
    float4 camera_position;
};

cbuffer ModelMatrixBuffer : register(b2)
{
    float4x4 model_matrix;
    float4x4 ti_model_matrix;
};

cbuffer GridBuffer : register(b3)
{
    float offset;
    float width;
    float time;
    float padding;
};

cbuffer BackgroundColorBuffer : register(b4)
{
    float4 bgcolor;
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
    float4 const_color = float4(0.65, 0.65, 0.75, 1.0);

    float wpx = input.world_position.x + sin(input.world_position.z + time * 0.5 * 0.241378);
    float wpz = input.world_position.z + sin(input.world_position.x + time * 0.5 * 0.345786);
    
    
    
    
    
    //float dist = length(input.world_position.xz);
    //
    //float radius = 50.0;
    //float softness = 25.0;
    //float vignette = smoothstep(radius, radius - softness, dist);
    //
    //float t = (1.0 - vignette);
    //
    //const_color = lerp(const_color, bgcolor, t);
    
    PixelOutput output;
    output.color = const_color;
    output.position = input.world_position;
    output.normal = input.normal * 0.5 + 0.5;
   
    return output;
}
