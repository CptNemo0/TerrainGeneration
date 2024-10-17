struct Vertex
{
    float3 position;
    float3 normal;
};

struct LinearConstraint
{
    int idx_a;
    int idx_b;
    float distance;
    float padding;
};

RWStructuredBuffer<Vertex> output_buffer : register(u0);
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
    Vertex a = output_buffer[constraint.idx_a];
    Vertex b = output_buffer[constraint.idx_b];
    
    float dst = distance(a.position, b.position);
    dst += 0.00001;
    
    float idst = 1.0 / dst;
    float C = dst - constraint.distance;
    float3 ga = (a.position - b.position) * idst;
    float3 gb = -1.0 * ga;
    float gal = length(ga);
    float gab = gal;
    float lambda = -1.0 * C / (imass * (gal + gab) + alpha * idt);
    float m = lambda * imass;
    a.position += m * ga;
    b.position += m * gb;
    
    output_buffer[constraint.idx_a] = a;
    output_buffer[constraint.idx_b] = b;

}
