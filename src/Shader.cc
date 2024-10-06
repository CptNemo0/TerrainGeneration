#include "../include/Shader.h"

HRESULT CompileShader(_In_ LPCWSTR src_file, _In_ LPCSTR entry_point, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
    if (!src_file || !entry_point || !profile || !blob)
        return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif

    const D3D_SHADER_MACRO defines[] =
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };

    ID3DBlob* shader_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
    HRESULT hr = D3DCompileFromFile(src_file, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry_point, profile,
        flags, 0, &shader_blob, &error_blob);
    if (FAILED(hr))
    {
        if (error_blob)
        {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }

        if (shader_blob)
            shader_blob->Release();

        return hr;
    }

    *blob = shader_blob;

    return hr;
}

int CompileShaders(ID3DBlob** vs_blob, LPCWSTR vertex_file_path, ID3DBlob** ps_blob, LPCWSTR pixel_file_pth)
{
    // Compile vertex shader
    HRESULT hr = CompileShader(vertex_file_path, "VSMain", "vs_4_0_level_9_1", vs_blob);
    if (FAILED(hr))
    {
        std::cout << "Failed compiling vertex shader %08X: " << hr << "\n";
        return -1;
    }

    hr = CompileShader(pixel_file_pth, "PSMain", "ps_4_0_level_9_1", ps_blob);
    if (FAILED(hr))
    {
        (*vs_blob)->Release();
        std::cout << "Failed compiling pixel shader %08X: "<<hr<<"\n";
        return -1;
    }

    std::cout << "Shaders Compiled\n";
    
    std::string vertex_file_input = CW2A(vertex_file_path);
    std::string pixel_file_input = CW2A(pixel_file_pth);

    std::string vertex_file_output = vertex_file_input.substr(0, vertex_file_input.length() - 4) + "cso";
    std::string pixel_file_output = pixel_file_input.substr(0, pixel_file_input.length() - 4) + "cso";

    std::ofstream vertex_file(vertex_file_output, std::ios::binary);
    if (!vertex_file.is_open())
    {
        std::cout << "Error: Unable to open file " << vertex_file_output << std::endl;
        (*vs_blob)->Release();
        (*ps_blob)->Release();
        return false;
    }
    vertex_file.write(reinterpret_cast<const char*>((*vs_blob)->GetBufferPointer()), (*vs_blob)->GetBufferSize());
    vertex_file.close();

    std::ofstream pixel_file(pixel_file_output, std::ios::binary);
    if (!pixel_file.is_open())
    {
        std::cout << "Error: Unable to open file " << pixel_file_output << std::endl;
        (*vs_blob)->Release();
        (*ps_blob)->Release();
        return false;
    }
    pixel_file.write(reinterpret_cast<const char*>((*ps_blob)->GetBufferPointer()), (*ps_blob)->GetBufferSize());
    pixel_file.close();

    return 0;
}
