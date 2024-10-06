#ifndef CONSTANT_BUFFER_STRUCTS_H
#define CONSTANT_BUFFER_STRUCTS_H

#include<glm/glm.hpp>
#include<DirectXMath.h>

struct ColorBuffer
{
	alignas(16)DirectX::XMFLOAT4 color;
};

#endif // !CONSTANT_BUFFER_STRUCTS_H
