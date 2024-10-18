struct LinearConstraint
{
    int idx_a;
    int idx_b;
    float distance;
    float padding;
};

RWStructuredBuffer<float3> position_buffer : register(u0);
StructuredBuffer<LinearConstraint> constraints : register(t0);

cbuffer DeltaTime : register(b0)
{
    float dt;
    float idt;
    float2 padding_dt;
};

cbuffer Compliance : register(b1)
{
    float alpha;
    float3 padding_c;
};

cbuffer Mass : register(b2)
{
    float mass;
    float imass;
    float2 padding_m;
};

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    LinearConstraint constraint = constraints[id.x];
    float3 a = position_buffer[constraint.idx_a];
    float3 b = position_buffer[constraint.idx_b];
    
    float3 diff = a - b;
    float dst_sqr = dot(diff, diff);
    dst_sqr += 0.000001;
    float dst = sqrt(dst_sqr);
    
    float idst = 1.0 / dst;
    float C = dst - constraint.distance;
    float3 ga = diff * idst;
    float gal = length(ga);
    float lambda = -C / (imass * (2.0 * gal) + alpha * idt);
    float m = lambda * imass;
    a += m * ga;
    b -= m * ga;
    
    position_buffer[constraint.idx_a] = a;
    position_buffer[constraint.idx_b] = b;
}
