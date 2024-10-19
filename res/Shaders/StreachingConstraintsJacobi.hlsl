struct LinearConstraint
{
    int idx_a;
    int idx_b;
    float distance;
    float padding;
};

RWStructuredBuffer<float3> position_buffer : register(u0);
RWStructuredBuffer<uint3> jacobi_buffer : register(u1);
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
    float idst = rsqrt(dst_sqr + 0.000001);
    float dst = dst_sqr * idst;
    
    
    float C = dst - constraint.distance;
    float3 ga = diff * idst;
    
    float lambda = -C / (imass * (2.0) + alpha * idt);
    float m = lambda * imass;
    a += m * ga;
    b -= m * ga;
        
    uint3 ua = asuint(a);
    uint3 ub = asuint(b);
    
    InterlockedAdd(jacobi_buffer[constraint.idx_a].x, ua.x);
    InterlockedAdd(jacobi_buffer[constraint.idx_a].y, ua.y);
    InterlockedAdd(jacobi_buffer[constraint.idx_a].z, ua.z);
    InterlockedAdd(jacobi_buffer[constraint.idx_b].x, ub.x);
    InterlockedAdd(jacobi_buffer[constraint.idx_b].y, ub.y);
    InterlockedAdd(jacobi_buffer[constraint.idx_b].z, ub.z);
}
