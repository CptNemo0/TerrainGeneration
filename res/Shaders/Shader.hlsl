#pragma vs VSMain

struct VertexInput
{
    float4 position : POSITION;
    float4 normal   : NORMAL;
    
};

struct PixelInput
{
    float4 projected_position : SV_POSITION;
    float4 world_position : POSITION1;
    float4 normal : NORMAL;
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


PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    float4 view_position = mul(view_matrix, input.position);
    float4 proj_position = mul(projection_matrix, view_position);
 
    output.projected_position = proj_position;
    output.world_position = input.position;
    
    float4 difference = camera_position - input.position;
    
    float4 view_direction = normalize(difference);
    float view_dot_product = dot(view_direction.xyz, input.normal.xyz);
    
    float4 N = input.normal;
    N = normalize(N);
    
    if (view_dot_product < 0)
    {
        N = -N;

    }
    
    output.normal = N;
    
    return output;
}

float4 PSMain(PixelInput input) : SV_TARGET
{
    float4 ambient_light = float4(0.1, 0.1, 0.1, 1.0);
    float4 light_position = float4(0.0, 5.0, 5.0, 1.0);
    float4 diffuse_color = float4(1.0, 1.0, 1.0, 1.0);
    
    float4 pos_m_light = light_position - input.world_position;
    float distance2light = length(pos_m_light);
    float4 light_direction = normalize(pos_m_light);
    float attenuation = 1.0 / (distance2light * distance2light);
    
    float diffuse = max(dot(light_direction, input.normal), 0.0);
    
    return ambient_light + diffuse * attenuation * diffuse_color * 25.0;

}
