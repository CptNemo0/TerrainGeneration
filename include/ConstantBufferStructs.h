#ifndef CONSTANT_BUFFER_STRUCTS_H
#define CONSTANT_BUFFER_STRUCTS_H

#include "../include/global.h"
#include<glm/glm.hpp>
#include<DirectXMath.h>

struct ColorBuffer
{
	alignas(16)DirectX::XMFLOAT4 color;
};

struct ModelMatrixBuffer
{
	DirectX::XMMATRIX model_matrix;
};

struct ViewProjBuffer
{
	DirectX::XMMATRIX view_matrix;
	DirectX::XMMATRIX projection_matrix;
};

/*
* shader_type: 0 - vertex, 1 - pixel
*/
template<typename T>
inline void RegisterConstantBuffer(ComPtr<ID3D11Buffer> constant_buffer, T* data, UINT binding, int shader_type);

#endif // !CONSTANT_BUFFER_STRUCTS_H

template<typename T>
inline void RegisterConstantBuffer(ComPtr<ID3D11Buffer> constant_buffer, T* data, UINT binding, int shader_type)
{
    D3D11_BUFFER_DESC constant_buffer_description = { 0 };
    constant_buffer_description.ByteWidth = sizeof(T);
    constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA constant_buffer_srd;
    constant_buffer_srd.pSysMem = data;
    constant_buffer_srd.SysMemPitch = 0;
    constant_buffer_srd.SysMemSlicePitch = 0;

    device->CreateBuffer(&constant_buffer_description, &constant_buffer_srd, &constant_buffer);

    switch (shader_type)
    {
        case 0:
        {
            context->VSSetConstantBuffers(binding, 1, &constant_buffer);
            break;
        }
        case 1:
        {
            context->PSSetConstantBuffers(binding, 1, &constant_buffer);
            break;
        }
    }
}
