struct LinearConstraint
{
    int idx_a;
    int idx_b;
    float distance;
    float padding;
};

RWStructuredBuffer<float3> position_buffer : register(u0);
RWStructuredBuffer<float3> jacobi_buffer : register(u1);
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
    //a += m * ga;
    //b -= m * ga;
    
    //float3 ja = asfloat(jacobi_buffer[constraint.idx_a]);
    //float3 jb = asfloat(jacobi_buffer[constraint.idx_b]);
    //
    //ja += m * ga;
    //jb -= m * ga;
    
    jacobi_buffer[constraint.idx_a] += m * ga;
    jacobi_buffer[constraint.idx_b] -= m * ga;
    //int tmp;
    //
    //InterlockedExchange(jacobi_buffer[constraint.idx_a].x, asuint(ja.x), tmp);
    //InterlockedExchange(jacobi_buffer[constraint.idx_a].y, asuint(ja.y), tmp);
    //InterlockedExchange(jacobi_buffer[constraint.idx_a].z, asuint(ja.z), tmp);
    //InterlockedExchange(jacobi_buffer[constraint.idx_b].x, asuint(jb.x), tmp);
    //InterlockedExchange(jacobi_buffer[constraint.idx_b].y, asuint(jb.y), tmp);
    //InterlockedExchange(jacobi_buffer[constraint.idx_b].z, asuint(jb.z), tmp);
}
