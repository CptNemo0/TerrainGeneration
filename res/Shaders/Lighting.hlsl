#pragma vs VSMain

struct VertexInput
{
    float4 position : POSITION;
    float4 normal : NORMAL;
};

struct PixelInput
{
    float4 projected_position : SV_POSITION;
    float2 uv : TEXCOORD;
};


cbuffer CameraBuffer : register(b2)
{
    float4 camera_position;
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

    output.projected_position = input.position;
    
    float4 p = (input.position + 1.0) * 0.5;
    p.y = 1.0 - p.y;
    float4 uv = p;
  
    output.uv = uv;
    
    return output;
}

SamplerState texture_sampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture2D position_texture : register(t0);
Texture2D normal_texture : register(t1);
Texture2D color_texture : register(t2);

float4 PSMain(PixelInput input) : SV_Target
{
    float4 ambient_light = float4(0.2, 0.2, 0.2, 1.0);
    float4 world_position = float4(position_texture.Sample(texture_sampler, input.uv).rgb, 1.0);
    float4 normal = float4(normal_texture.Sample(texture_sampler, input.uv).rgb * 2.0 - 1.0, 1.0);
    float4 color = float4(color_texture.Sample(texture_sampler, input.uv).rgb, 1.0);
    
    float4 light_position = sl_position;
    float4 diffuse_color = sl_diffuse_color;
    float4 specular_color = sl_specular_color;
    float intensity = sl_intensity;
    float shinieness = 600.0;
    float gamma = 2.2;
    float inverse_gamma = 1.0 / 2.2;
    
    float4 pos_m_light = light_position - world_position;
    float distance2light = length(pos_m_light);
    float4 light_direction = normalize(pos_m_light);
    float attenuation = 1.0 / (distance2light * distance2light);
    
    float diffuse = 0.0;
    float spec = 0.0;
    
    float theta = dot(light_direction, normalize(-sl_direction));
    
    if (theta > sl_cut_off)
    {
        float4 view_direction = normalize(camera_position - world_position);
        float4 halfway_direction = normalize(light_direction + view_direction);
    
        diffuse = max(dot(light_direction, normal), 0.0);
        spec = pow(max(dot(view_direction, halfway_direction), 0.0), shinieness);
        
        float epsilon = sl_cut_off - sl_outer_cut_off;
        float sl_intensity = clamp((theta - sl_outer_cut_off) / epsilon, 0.0, 1.0);
        
        diffuse *= sl_intensity;

    }
    
    float4 after_light = (ambient_light + (diffuse * sl_diffuse_color + spec * sl_specular_color) * attenuation * intensity) * color;
   
    return after_light;
}
