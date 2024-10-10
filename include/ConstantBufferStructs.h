#ifndef CONSTANT_BUFFER_STRUCTS_H
#define CONSTANT_BUFFER_STRUCTS_H

#include "../include/global.h"
#include<glm/glm.hpp>
#include<DirectXMath.h>

struct ColorBuffer
{
	DirectX::XMFLOAT4 color;
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
	float padding1, padding2;
};

struct DirectionalLight
{
	DirectX::XMVECTOR direction;
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

#endif // !CONSTANT_BUFFER_STRUCTS_H

