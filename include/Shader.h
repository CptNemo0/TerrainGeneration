#ifndef SHADER_H
#define SHADER_H

#define _WIN32_WINNT 0x600
#include <stdio.h>
#include <fstream>
#include <atlstr.h>
#include <iostream>
#include <d3dcompiler.h>
#include <d3d11.h>

#pragma comment(lib,"d3dcompiler.lib")

struct Shader
{
    ID3D11VertexShader* vertex_shader = nullptr;
    ID3D11PixelShader*  pixel_shader = nullptr;
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
};

HRESULT CompileShader(_In_ LPCWSTR src_file, _In_ LPCSTR entry_point, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);

int CompileShaders(ID3DBlob** vs_blob, ID3DBlob** ps_blob, LPCWSTR shader_path);
int CompileShader(ID3DBlob** cs_blob, LPCWSTR shader_path);
#endif // !SHADER_H