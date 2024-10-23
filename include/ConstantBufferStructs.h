#ifndef CONSTANT_BUFFER_STRUCTS_H
#define CONSTANT_BUFFER_STRUCTS_H

#include<DirectXMath.h>

struct ColorBuffer
{
	DirectX::XMVECTOR color;
};

struct ModelMatrixBuffer
{
	DirectX::XMMATRIX model_matrix;
	DirectX::XMMATRIX ti_model_matrix;
};

struct ViewProjBuffer
{
	DirectX::XMMATRIX view_matrix;
	DirectX::XMMATRIX projection_matrix;
};

struct GridBuffer
{
	float offset;
	float width;
	float time;
	float padding2;
};

struct CameraBuffer
{
	DirectX::XMVECTOR camera_position;
};

struct SpotlightBuffer
{
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR direction;
	DirectX::XMVECTOR diffuse_color;
	DirectX::XMVECTOR specular_color;
	float cut_off;
	float outer_cut_off;
	float intensity;
	float padding;
};

struct LightSpaceBuffer
{
	DirectX::XMMATRIX view_matrix;
	DirectX::XMMATRIX projection_matrix;
};

struct TimeBuffer
{
	float time;
	float p1, p2, p3;
};

struct DeltaTimeBuffer
{
	float dt;
	float idt;
	float t;
	float p1;
};

struct GravityBuffer
{
	float x;
	float y;
	float z;
	float p1;
};

struct ComplianceBuffer
{
	float alpha;
	float p1, p2, p3;
};

struct MassBuffer
{
	float mass;
	float imass;
	float p1, p2;
};

struct WindBuffer
{
	float strength_mul;
	float x;
	float y;
	float z;
};

struct ResolutionBuffer
{
	int resolution;
	int z_multiplier;
	int p2, p3;
};

struct PinBitmaskBuffer
{
	int mask;
	int p1, p2, p3;
};

#endif // !CONSTANT_BUFFER_STRUCTS_H

