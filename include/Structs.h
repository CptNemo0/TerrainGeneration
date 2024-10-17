#ifndef STRUCTS_H
#define STRUCTS_H
struct Vertex
{
    float x;
    float y;
    float z;
    
    float nx;
    float ny;
    float nz;
};

struct Face
{
    int a;
    int b;
    int c;
};

struct LinearConstraint
{
    int idx_a;
    int idx_b;
    float distance;
    float padding;
};

struct PinConstraint
{
    float x, y, z;
    int idx;
};
#endif