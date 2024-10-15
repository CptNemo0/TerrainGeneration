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

#endif // !CONSTANT_BUFFER_STRUCTS_H

