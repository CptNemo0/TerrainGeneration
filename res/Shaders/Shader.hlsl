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

float4 PSMain(PixelInput input) : SV_TARGET
{
    float4 ambient_light = float4(0.2, 0.2, 0.2, 1.0);
    
    float4 light_position = sl_position;
    float4 diffuse_color = sl_diffuse_color;
    float4 specular_color = sl_specular_color;
    float intensity = sl_intensity;
    float shinieness = 100.0;
    float gamma = 2.2;
    float inverse_gamma = 1.0 / 2.2;
    
    float4 pos_m_light = light_position - input.world_position;
    float distance2light = length(pos_m_light);
    float4 light_direction = normalize(pos_m_light);
    float attenuation = 1.0 / (distance2light * distance2light);

    float diffuse = 0.0;
    float spec = 0.0;
    
    float theta = dot(light_direction, normalize(-sl_direction));
    
    if (theta > sl_cut_off)
    {
        float4 view_direction = normalize(camera_position - input.world_position);
        float4 halfway_direction = normalize(light_direction + view_direction);
    
        diffuse = max(dot(light_direction, input.normal), 0.0);
        spec = pow(max(dot(view_direction, halfway_direction), 0.0), shinieness);
        
        float epsilon = sl_cut_off - sl_outer_cut_off;
    }

    float4 after_light = (ambient_light + (diffuse * sl_diffuse_color + spec * sl_specular_color) * attenuation * intensity) * color;

    // fog

    float dist = length(input.world_position.xz);

    float radius = 50.0;
    float softness = 25.0;
    float vignette = smoothstep(radius, radius - softness, dist);

    after_light.a -= (1.0 - vignette);
   
    return after_light;
}
