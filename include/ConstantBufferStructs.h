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

struct DirectionalLight
{
	DirectX::XMVECTOR direction;
};

struct CameraBuffer
{
	DirectX::XMVECTOR camera_position;
};

#endif // !CONSTANT_BUFFER_STRUCTS_H

