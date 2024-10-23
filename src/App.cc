#include "../include/App.h"

ID3D11Device* App::device_ = nullptr;
ID3D11DeviceContext* App::context_ = nullptr;
IDXGISwapChain* App::swap_chain_ = nullptr;
ID3D11RenderTargetView* App::main_render_target_view_ = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
    case WM_SIZE:
        if (App::device_ != NULL && wParam != SIZE_MINIMIZED)
        {
            App::CleanupRenderTarget();
            App::swap_chain_->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            App::CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

App::App(int width, int height, LPCSTR name)
{
    window_width_ = width;
    window_height_ = height;
    window_name_ = name;
}

void App::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    swap_chain_->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device_->CreateRenderTargetView(pBackBuffer, NULL, &main_render_target_view_);
    pBackBuffer->Release();
}

void App::CleanupRenderTarget()
{
    if (main_render_target_view_) { main_render_target_view_->Release(); main_render_target_view_ = NULL; }
}

bool App::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 1600;
    sd.BufferDesc.Height = 900;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 999;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swap_chain_, &device_, &featureLevel, &context_) != S_OK)
        return false;

    IDXGIDevice* pDXGIDevice = nullptr;
    IDXGIAdapter* pAdapter = nullptr;
    DXGI_ADAPTER_DESC adapterDesc;

    // Query the DXGI Device from the D3D11 device_
    HRESULT hr = device_->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
    if (SUCCEEDED(hr) && pDXGIDevice)
    {
        // Get the adapter (the GPU) that the device_ is using
        hr = pDXGIDevice->GetAdapter(&pAdapter);
        if (SUCCEEDED(hr) && pAdapter)
        {
            // Get the adapter's description
            pAdapter->GetDesc(&adapterDesc);

            // Output the GPU information
            std::wcout << L"GPU: " << adapterDesc.Description << std::endl;
            std::wcout << L"Dedicated Video Memory: " << adapterDesc.DedicatedVideoMemory / (1024 * 1024) << L" MB" << std::endl;

            // Release the adapter
            pAdapter->Release();
        }

        // Release the DXGI device_
        pDXGIDevice->Release();
    }

    CreateRenderTarget();
    return true;
}

void App::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (swap_chain_) { swap_chain_->Release(); swap_chain_ = nullptr; }
    if (context_) { context_->Release(); context_ = nullptr; }
    if (device_) { device_->Release(); device_ = nullptr; }
}

void App::InitViewport()
{
    ZeroMemory(&viewport_, sizeof(D3D11_VIEWPORT));
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.Width = window_width_;
    viewport_.Height = window_height_;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
}

void App::InitRasterizer()
{
    ZeroMemory(&rasterizer_description_, sizeof(rasterizer_description_));

    rasterizer_description_.FillMode = D3D11_FILL_SOLID;
    rasterizer_description_.CullMode = D3D11_CULL_NONE;
    rasterizer_description_.FrontCounterClockwise = false;
    rasterizer_description_.DepthClipEnable = true;
    rasterizer_description_.DepthBias = 0;
    rasterizer_description_.DepthBiasClamp = 0.0f;
    rasterizer_description_.SlopeScaledDepthBias = 0.0f;
    rasterizer_description_.DepthClipEnable = false;
    rasterizer_description_.ScissorEnable = false;
    rasterizer_description_.MultisampleEnable = true;
    rasterizer_description_.AntialiasedLineEnable = true;

    device_->CreateRasterizerState(&rasterizer_description_, &rasterizer_state_);
}

void App::InitDepthStencilBuffer()
{
    ZeroMemory(&depth_stencil_texture_description_, sizeof(D3D11_TEXTURE2D_DESC));
    ZeroMemory(&depth_stencil_description_, sizeof(D3D11_DEPTH_STENCIL_DESC));
    ZeroMemory(&depth_stencil_view_description_, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    depth_stencil_texture_description_.Width = window_width_;
    depth_stencil_texture_description_.Height = window_height_;
    depth_stencil_texture_description_.MipLevels = 1;
    depth_stencil_texture_description_.ArraySize = 1;
    depth_stencil_texture_description_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_texture_description_.SampleDesc.Count = 1;
    depth_stencil_texture_description_.SampleDesc.Quality = 0;
    depth_stencil_texture_description_.Usage = D3D11_USAGE_DEFAULT;
    depth_stencil_texture_description_.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    depth_stencil_texture_description_.CPUAccessFlags = 0;
    depth_stencil_texture_description_.MiscFlags = 0;

    depth_stencil_description_.DepthEnable = true;
    depth_stencil_description_.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_description_.DepthFunc = D3D11_COMPARISON_LESS;
    depth_stencil_description_.StencilEnable = true;
    depth_stencil_description_.StencilReadMask = 0xFF;
    depth_stencil_description_.StencilWriteMask = 0xFF;
    depth_stencil_description_.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description_.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depth_stencil_description_.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description_.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depth_stencil_description_.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description_.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depth_stencil_description_.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description_.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    depth_stencil_view_description_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_view_description_.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_description_.Texture2D.MipSlice = 0;

    if (static_cast<int>(device_->CreateTexture2D(&depth_stencil_texture_description_, nullptr, &depth_stencil_texture_)))
    {
        std::cout << "Depth stencil texture creation failed." << std::endl;
        exit(1);
    }

    if (static_cast<int>(device_->CreateDepthStencilState(&depth_stencil_description_, &depth_stencil_state_)))
    {
        std::cout << "Depth stencil state creation failed." << std::endl;
        exit(1);
    }

    if (static_cast<int>(device_->CreateDepthStencilView(depth_stencil_texture_, &depth_stencil_view_description_, &depth_stencil_view_)))
    {
        std::cout << "Depth stencil view creation failed." << std::endl;
        exit(1);
    }
}

template <typename T>
void App::SetCBuffer(ID3D11Buffer* buffer, T& data)
{
    if (!buffer) [[unlikely]]
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    context_->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
    T* data_ptr = (T*)mapped_resource.pData;
    *data_ptr = data;
    context_->Unmap(buffer, 0);
}

template <typename T>
void App::CreateCBuffer(ID3D11Buffer** buffer, T& data)
{
    D3D11_BUFFER_DESC description = { 0 };
    description.ByteWidth = sizeof(T);
    description.Usage = D3D11_USAGE_DYNAMIC;
    description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA srd;
    srd.pSysMem = &data;
    srd.SysMemPitch = 0;
    srd.SysMemSlicePitch = 0;
    device_->CreateBuffer(&description, &srd, buffer);
}

void App::CreateShaders(Shader& shader, LPCWSTR path)
{
    CompileShaders(&(shader.vs_blob), &(shader.ps_blob), path);
    device_->CreateVertexShader(shader.vs_blob->GetBufferPointer(), shader.vs_blob->GetBufferSize(), nullptr, &(shader.vertex_shader));
    device_->CreatePixelShader(shader.ps_blob->GetBufferPointer(), shader.ps_blob->GetBufferSize(), nullptr, &(shader.pixel_shader));
}

void App::CreateCShader(CShader& shader, LPCWSTR path)
{
    CompileShader(&(shader.cs_blob), path);
    device_->CreateComputeShader(shader.cs_blob->GetBufferPointer(), shader.cs_blob->GetBufferSize(), nullptr, &(shader.compute_shader));
}

void App::BindShaders(Shader& shader)
{
    context_->VSSetShader(shader.vertex_shader, nullptr, 0);
    context_->PSSetShader(shader.pixel_shader, nullptr, 0);
}

void App::BindCShader(CShader& shader)
{
    context_->CSSetShader(shader.compute_shader, nullptr, 0);
}

void App::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    wc_ = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, window_name_, NULL };
    ::RegisterClassEx(&wc_);
    hwnd_ = ::CreateWindow(wc_.lpszClassName, _T(window_name_), WS_OVERLAPPEDWINDOW, 100, 100, window_width_, window_height_, NULL, NULL, wc_.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd_))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc_.lpszClassName, wc_.hInstance);
        exit(1);
    }

    // Show the window
    ::ShowWindow(hwnd_, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd_);

    ImGui_ImplWin32_Init(hwnd_);  // Pass the window handle here
    ImGui_ImplDX11_Init(device_, context_);

    InitViewport();
    context_->RSSetViewports(1, &viewport_);

    InitDepthStencilBuffer();
    context_->OMSetDepthStencilState(depth_stencil_state_, 1);
    context_->OMSetRenderTargets(1, &main_render_target_view_, depth_stencil_view_);

    InitRasterizer();
    context_->RSSetState(rasterizer_state_);

    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void App::End()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupRenderTarget();
    CleanupDeviceD3D();

    ::DestroyWindow(hwnd_);
    ::UnregisterClass(wc_.lpszClassName, wc_.hInstance);
}

void App::Run()
{
    ULONGLONG start_time = GetTickCount64();

    const float rectangle_vertices[]
    {
        //position============= | normals=========|
        -150.0f, -2.5f,  150.0f, 0.0f, 1.0f, 0.0f,
         150.0f, -2.5f,  150.0f, 0.0f, 1.0f, 0.0f,
         150.0f, -2.5f, -150.0f, 0.0f, 1.0f, 0.0f,
        -150.0f, -2.5f, -150.0f, 0.0f, 1.0f, 0.0f,
    };

    const unsigned int rectangle_indices[]
    {
        0, 1, 3,
        3, 1, 2
    };

    const float screen_quad_vertices[]
    {
        //position========| normals=========|
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    const unsigned int screen_quad_indices[]
    {
        0, 1, 3,
        3, 1, 2
    };

    const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    DirectX::XMVECTOR  camera_position_iv = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    float camera_distance = 3.0f;
    DirectX::XMVECTOR  camera_angles = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR  center_position_iv = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR  up_direction_iv = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    auto view_matrix = DirectX::XMMatrixLookAtLH(camera_position_iv, center_position_iv, up_direction_iv);
    auto projection_matrix = DirectX::XMMatrixPerspectiveFovLH(0.7864f, 16.0f / 9.0f, 0.1f, 1000.0f);

#pragma region Buffers
    ID3D11UnorderedAccessView* cleaner_uav = nullptr;
    ID3D11ShaderResourceView* cleaner_srv = nullptr;
    // Rectangle Buffers Buffer
    ID3D11Buffer* rectangle_vertex_buffer;
    D3D11_BUFFER_DESC rectangle_buffer_description = { 0 };
    rectangle_buffer_description.ByteWidth = sizeof(float) * 4 * 6;
    rectangle_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    rectangle_buffer_description.StructureByteStride = sizeof(float) * 6;
    D3D11_SUBRESOURCE_DATA rectangle_vertex_srd = { rectangle_vertices, 0, 0 };
    device_->CreateBuffer(&rectangle_buffer_description, &rectangle_vertex_srd, &rectangle_vertex_buffer);

    ID3D11Buffer* rectangle_index_buffer;
    D3D11_BUFFER_DESC rectangle_index_buffer_description = { 0 };
    rectangle_index_buffer_description.ByteWidth = sizeof(unsigned int) * 3 * 2;
    rectangle_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA rectangle_index_srd = { rectangle_indices, 0, 0 };
    device_->CreateBuffer(&rectangle_index_buffer_description, &rectangle_index_srd, &rectangle_index_buffer);
    // Rectangle Buffers End

    // Screen Quad Buffers
    ID3D11Buffer* screen_quad_vertex_buffer;
    D3D11_BUFFER_DESC screen_quad_buffer_description = { 0 };
    screen_quad_buffer_description.ByteWidth = sizeof(float) * 4 * 6;
    screen_quad_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    screen_quad_buffer_description.StructureByteStride = sizeof(float) * 6;
    D3D11_SUBRESOURCE_DATA screen_quad_vertex_srd = { screen_quad_vertices, 0, 0 };
    device_->CreateBuffer(&screen_quad_buffer_description, &screen_quad_vertex_srd, &screen_quad_vertex_buffer);

    ID3D11Buffer* screen_quad_index_buffer;
    D3D11_BUFFER_DESC screen_quad_index_buffer_description = { 0 };
    screen_quad_index_buffer_description.ByteWidth = sizeof(unsigned int) * 3 * 2;
    screen_quad_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA screen_quad_index_srd = { screen_quad_indices, 0, 0 };
    device_->CreateBuffer(&screen_quad_index_buffer_description, &screen_quad_index_srd, &screen_quad_index_buffer);
    // Screen Quad Buffers End

#pragma endregion

#pragma region Textures

    D3D11_TEXTURE2D_DESC texture_description = { 0 };
    texture_description.Width = window_width_;
    texture_description.Height = window_height_;
    texture_description.MipLevels = 1;
    texture_description.ArraySize = 1;
    texture_description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texture_description.SampleDesc.Count = 1;
    texture_description.Usage = D3D11_USAGE_DEFAULT;
    texture_description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* position_texture;
    ID3D11RenderTargetView* position_render_target_view;
    ID3D11ShaderResourceView* position_shader_resource_view;
    if (static_cast<int>(device_->CreateTexture2D(&texture_description, nullptr, &position_texture)))
    {
        std::cout << "Creating position texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateShaderResourceView(position_texture, nullptr, &position_shader_resource_view)))
    {
        std::cout << "Creating position_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateRenderTargetView(position_texture, nullptr, &position_render_target_view)))
    {
        std::cout << "Creating position_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* normal_texture;
    ID3D11RenderTargetView* normal_render_target_view;
    ID3D11ShaderResourceView* normal_shader_resource_view;
    if (static_cast<int>(device_->CreateTexture2D(&texture_description, nullptr, &normal_texture)))
    {
        std::cout << "Creating normal texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateShaderResourceView(normal_texture, nullptr, &normal_shader_resource_view)))
    {
        std::cout << "Creating normal_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateRenderTargetView(normal_texture, nullptr, &normal_render_target_view)))
    {
        std::cout << "Creating normal_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* color_texture;
    ID3D11RenderTargetView* color_render_target_view;
    ID3D11ShaderResourceView* color_shader_resource_view;
    if (static_cast<int>(device_->CreateTexture2D(&texture_description, nullptr, &color_texture)))
    {
        std::cout << "Creating color texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateShaderResourceView(color_texture, nullptr, &color_shader_resource_view)))
    {
        std::cout << "Creating color_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateRenderTargetView(color_texture, nullptr, &color_render_target_view)))
    {
        std::cout << "Creating color_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* lightspace_position_texture;
    ID3D11RenderTargetView* lightspace_position_render_target_view;
    ID3D11ShaderResourceView* lightspace_position_shader_resource_view;
    if (static_cast<int>(device_->CreateTexture2D(&texture_description, nullptr, &lightspace_position_texture)))
    {
        std::cout << "Creating lightspace_position texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateShaderResourceView(lightspace_position_texture, nullptr, &lightspace_position_shader_resource_view)))
    {
        std::cout << "Creating lightspace_position_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateRenderTargetView(lightspace_position_texture, nullptr, &lightspace_position_render_target_view)))
    {
        std::cout << "Creating lightspace_position_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* fxaa_texture;
    ID3D11RenderTargetView* fxaa_render_target_view;
    ID3D11ShaderResourceView* fxaa_shader_resource_view;
    if (static_cast<int>(device_->CreateTexture2D(&texture_description, nullptr, &fxaa_texture)))
    {
        std::cout << "Creating fxaa texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateShaderResourceView(fxaa_texture, nullptr, &fxaa_shader_resource_view)))
    {
        std::cout << "Creating fxaa_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateRenderTargetView(fxaa_texture, nullptr, &fxaa_render_target_view)))
    {
        std::cout << "Creating fxaa_shader_resource_view failed\n";
        exit(1);
    }

    texture_description = { 0 };
    texture_description.Width = window_width_;
    texture_description.Height = window_height_;
    texture_description.MipLevels = 1;
    texture_description.ArraySize = 1;
    texture_description.Format = DXGI_FORMAT_R32_FLOAT;
    texture_description.SampleDesc.Count = 1;
    texture_description.Usage = D3D11_USAGE_DEFAULT;
    texture_description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* depth_texture;
    ID3D11RenderTargetView* depth_render_target_view;
    ID3D11ShaderResourceView* depth_shader_resource_view;
    if (static_cast<int>(device_->CreateTexture2D(&texture_description, nullptr, &depth_texture)))
    {
        std::cout << "Creating depth texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateShaderResourceView(depth_texture, nullptr, &depth_shader_resource_view)))
    {
        std::cout << "Creating depth_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device_->CreateRenderTargetView(depth_texture, nullptr, &depth_render_target_view)))
    {
        std::cout << "Creating depth_shader_resource_view failed\n";
        exit(1);
    }
#pragma endregion

#pragma region Shaders
    Shader screen_shader;
    CreateShaders(screen_shader, L"../res/Shaders/ScreenQuad.hlsl");

    Shader shadowmap_shader;
    CreateShaders(shadowmap_shader, L"../res/Shaders/ShadowMap.hlsl");

    Shader gbuffer_shader;
    CreateShaders(gbuffer_shader, L"../res/Shaders/GBuffer.hlsl");

    Shader gbuffer_grid_shader;
    CreateShaders(gbuffer_grid_shader, L"../res/Shaders/GridGBuffer.hlsl");

    Shader light_shader;
    CreateShaders(light_shader, L"../res/Shaders/Lighting.hlsl");

    CShader example_shader;
    CreateCShader(example_shader, L"../res/Shaders/ExampleCS.hlsl");

    CShader zero_normal_shader;
    CreateCShader(zero_normal_shader, L"../res/Shaders/ZeroNormals.hlsl");

    CShader recalculate_normal_shader;
    CreateCShader(recalculate_normal_shader, L"../res/Shaders/RecalculateNormals.hlsl");

    CShader update_position_shader;
    CreateCShader(update_position_shader, L"../res/Shaders/UpdatePositions.hlsl");

    CShader update_velocity_shader;
    CreateCShader(update_velocity_shader, L"../res/Shaders/UpdateVelocity.hlsl");

    CShader streaching_constraints_shader;
    CreateCShader(streaching_constraints_shader, L"../res/Shaders/EnforceStructural.hlsl");

    CShader streaching_constraints_jacobi_shader;
    CreateCShader(streaching_constraints_jacobi_shader, L"../res/Shaders/StreachingConstraintsJacobi.hlsl");

    CShader enforce_pin_shader;
    CreateCShader(enforce_pin_shader, L"../res/Shaders/EnoforcePin.hlsl");
#pragma endregion

#pragma region ConstantBuffers

    ColorBuffer color_data;
    color_data.color = DirectX::XMVECTOR({ 0.8f, 0.8f, 0.9f, 1.0f });

    ColorBuffer background_color_data;
    background_color_data.color = DirectX::XMVECTOR({ 0.0f, 0.0f, 0.0f, 1.0f });

    GridBuffer grid_data;
    grid_data.offset = 2.0f;
    grid_data.width = 0.5f;
    grid_data.time = 0.0f;

    ViewProjBuffer view_proj_data;
    view_proj_data.view_matrix = view_matrix;
    view_proj_data.projection_matrix = projection_matrix;

    CameraBuffer camera_data;
    camera_data.camera_position = camera_position_iv;

    ModelMatrixBuffer model_matrix_data;
    model_matrix_data.model_matrix = DirectX::XMMatrixIdentity();
    auto matrix_determinant = DirectX::XMMatrixDeterminant(model_matrix_data.model_matrix);
    model_matrix_data.ti_model_matrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&matrix_determinant, model_matrix_data.model_matrix));

    SpotlightBuffer spotlight_data;
    spotlight_data.position = DirectX::XMVECTOR({ 5.0f,5.0f, 5.0f, 1.0f });
    spotlight_data.direction = DirectX::XMVector4Normalize({ -3.0f, -5.0f, -3.0f, 1.0f });
    spotlight_data.diffuse_color = DirectX::XMVECTOR({ 1.0f, 1.0f, 1.0f, 1.0f });
    spotlight_data.specular_color = DirectX::XMVECTOR({ 1.0f, 1.0f, 1.0f, 1.0f });
    spotlight_data.cut_off = 0.59f;
    spotlight_data.outer_cut_off = 0.52f;
    spotlight_data.intensity = 50.0f;

    ViewProjBuffer lightspace_data;
    lightspace_data.view_matrix = DirectX::XMMatrixLookAtLH(spotlight_data.position, DirectX::XMVectorAdd(spotlight_data.position, spotlight_data.direction), up_direction_iv);
    lightspace_data.projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(-8.0f, 8.0f, -4.5f, 4.5f, 0.1f, 100.0f);

    TimeBuffer time_data;
    time_data.time = 0.0;

    DeltaTimeBuffer dt_data;
    dt_data.dt = 0.003f;
    dt_data.idt = 1.0f / dt_data.dt;
    dt_data.t = 0.0f;
    dt_data.p1 = 0.0f;

    GravityBuffer gravity_data;
    gravity_data.x = 0.0f;
    gravity_data.y = -10.0f;
    gravity_data.z = 0.0f;
    gravity_data.p1 = 0.0f;

    MassBuffer mass_data;
    mass_data.mass = 0.4f;
    mass_data.imass = 1.0f / mass_data.mass;

    ComplianceBuffer structural_compliance_data;
    structural_compliance_data.alpha = 0.0001f;

    ComplianceBuffer bending_compliance_data;
    bending_compliance_data.alpha = 0.01f;

    WindBuffer wind_data;
    wind_data.strength_mul = 1.0f;
    wind_data.x = 0.0f;
    wind_data.y = 0.0f;
    wind_data.z = -1.0f;

    ResolutionBuffer resolution_data;
    resolution_data.resolution = 64;

    ID3D11Buffer* color_constant_buffer = nullptr;
    CreateCBuffer(&color_constant_buffer, color_data);

    ID3D11Buffer* view_proj_constant_buffer;
    CreateCBuffer(&view_proj_constant_buffer, view_proj_data);

    ID3D11Buffer* camera_constant_buffer;
    CreateCBuffer(&camera_constant_buffer, camera_data);

    ID3D11Buffer* model_matrix_constant_buffer;
    CreateCBuffer(&model_matrix_constant_buffer, model_matrix_data);

    ID3D11Buffer* grid_constant_buffer;
    CreateCBuffer(&grid_constant_buffer, grid_data);

    ID3D11Buffer* spotlight_constant_buffer;
    CreateCBuffer(&spotlight_constant_buffer, spotlight_data);

    ID3D11Buffer* bg_color_constant_buffer;
    CreateCBuffer(&bg_color_constant_buffer, background_color_data);

    ID3D11Buffer* time_constant_buffer;
    CreateCBuffer(&time_constant_buffer, time_data);

    ID3D11Buffer* delta_time_constant_buffer;
    CreateCBuffer(&delta_time_constant_buffer, dt_data);

    ID3D11Buffer* gravity_constant_buffer;
    CreateCBuffer(&gravity_constant_buffer, gravity_data);

    ID3D11Buffer* compliance_constant_buffer;
    CreateCBuffer(&compliance_constant_buffer, structural_compliance_data);

    ID3D11Buffer* mass_constant_buffer;
    CreateCBuffer(&mass_constant_buffer, mass_data);

    ID3D11Buffer* wind_constant_buffer;
    CreateCBuffer(&wind_constant_buffer, wind_data);

    ID3D11Buffer* resolutiom_constant_buffer;
    CreateCBuffer(&resolutiom_constant_buffer, resolution_data);
#pragma endregion

    // initialize input layout
    D3D11_INPUT_ELEMENT_DESC input_element_description[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    ID3D11InputLayout* input_layout = nullptr;
    device_->CreateInputLayout(input_element_description, 2, gbuffer_shader.vs_blob->GetBufferPointer(), gbuffer_shader.vs_blob->GetBufferSize(), &input_layout);
    context_->IASetInputLayout(input_layout);

    uint32_t stride = sizeof(float) * 6;
    uint32_t offset = 0;

    float bgc[4] =
    {
        DirectX::XMVectorGetX(background_color_data.color),
        DirectX::XMVectorGetY(background_color_data.color),
        DirectX::XMVectorGetZ(background_color_data.color),
        DirectX::XMVectorGetW(background_color_data.color)
    };

    int resolution = 2;
    Cloth cloth{ resolution, device_ };
    cloth.zero_normals_shader_ = &zero_normal_shader;
    cloth.recalculate_normals_shader_ = &recalculate_normal_shader;
    cloth.stride_ = stride;
    cloth.offset_ = offset;

    // Main loop
    bool done = false;
    bool rotate = false;
    bool render_wireframe = false;

    bool run_sim = false;
    bool step_sim = false;

    int pin_amount = 2;

    int quality_steps = 20;

    POINT prev_mouse_pos = { 0, 0 };
    while (!done)
    {

#pragma region Input handling

        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                cloth.CleanUp();
                done = true;
            }
                
            else if (msg.message == WM_MBUTTONDOWN)
            {
                rotate = true;
            }
            else if (msg.message == WM_MBUTTONUP)
            {
                rotate = false;
            }
            else if (msg.message == WM_MOUSEWHEEL)
            {
                float scroll_amount = GET_WHEEL_DELTA_WPARAM(msg.wParam) * 0.002f;
                camera_distance -= scroll_amount;

                camera_distance = fmaxf(camera_distance, 0.5f);

                float x = DirectX::XMScalarCos(DirectX::XMVectorGetByIndex(camera_angles, 0)) * DirectX::XMScalarCos(DirectX::XMVectorGetByIndex(camera_angles, 1));
                float y = DirectX::XMScalarSin(DirectX::XMVectorGetByIndex(camera_angles, 1));
                float z = -1.0 * DirectX::XMScalarSin(DirectX::XMVectorGetByIndex(camera_angles, 0)) * DirectX::XMScalarCos(DirectX::XMVectorGetByIndex(camera_angles, 1));
                camera_position_iv = DirectX::XMVectorSet(camera_distance * x, camera_distance * y, camera_distance * z, 1.0f);
                view_proj_data.view_matrix = DirectX::XMMatrixLookAtLH(camera_position_iv, center_position_iv, up_direction_iv);
            }
            else if (msg.message == WM_MOUSEMOVE)
            {
                int mouseX = GET_X_LPARAM(msg.lParam);
                int mouseY = GET_Y_LPARAM(msg.lParam);

                float deltaX = (mouseX - prev_mouse_pos.x) * 0.01f;
                float deltaY = (mouseY - prev_mouse_pos.y) * 0.01f;

                prev_mouse_pos.x = mouseX;
                prev_mouse_pos.y = mouseY;

                if (rotate)
                {
                    float a = DirectX::XMVectorGetByIndex(camera_angles, 0) + deltaX;
                    float b = fmaxf(fminf(DirectX::XMVectorGetByIndex(camera_angles, 1) + deltaY, 1.57f), -1.57f);

                    camera_angles = DirectX::XMVectorSetByIndex(camera_angles, a, 0);
                    camera_angles = DirectX::XMVectorSetByIndex(camera_angles, b, 1);

                    float x = DirectX::XMScalarCos(a) * DirectX::XMScalarCos(b);
                    float y = DirectX::XMScalarSin(b);
                    float z = -1.0 * DirectX::XMScalarSin(a) * DirectX::XMScalarCos(b);
                    camera_position_iv = DirectX::XMVectorSet(camera_distance * x, camera_distance * y, camera_distance * z, 1.0f);
                    camera_data.camera_position = camera_position_iv;
                    view_proj_data.view_matrix = DirectX::XMMatrixLookAtLH(camera_position_iv, center_position_iv, up_direction_iv);
                }
            }
        }
        if (done)
        {
            break;
        }

#pragma endregion

        ULONGLONG current_time = GetTickCount64();
        float t = (current_time - start_time) * 0.001f;
        float dt = 10.0f * fmin(ImGui::GetIO().DeltaTime, 1.0f / 60.0f) / (quality_steps);
        grid_data.time = t;
        time_data.time = t;

        dt_data.t = t;
        dt_data.dt = dt;

        dt_data.idt = 1.0f / dt;
#pragma region Logic
        if (run_sim || step_sim)
        {
            SetCBuffer(delta_time_constant_buffer, dt_data);
            SetCBuffer(gravity_constant_buffer, gravity_data);
            SetCBuffer(mass_constant_buffer, mass_data);
            SetCBuffer(wind_constant_buffer, wind_data);
            for (int it = 0; it < quality_steps; it++)
            {
                BindCShader(update_position_shader);
                context_->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context_->CSSetConstantBuffers(1, 1, &gravity_constant_buffer);
                context_->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context_->CSSetConstantBuffers(3, 1, &wind_constant_buffer);
                context_->CSSetConstantBuffers(4, 1, &resolutiom_constant_buffer);
                context_->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(1, 1, &cloth.previous_positions_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(2, 1, &cloth.velocity_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(3, 1, &cloth.jacobi_uav_, nullptr);
                resolution_data.resolution = cloth.resolution_;
                resolution_data.z_multiplier = cloth.resolution_ * cloth.resolution_ / 4;
                SetCBuffer(resolutiom_constant_buffer, resolution_data);
                context_->Dispatch(cloth.resolution_multiplier_, cloth.resolution_multiplier_, 4);

                BindCShader(enforce_pin_shader);
                context_->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context_->CSSetConstantBuffers(1, 1, &compliance_constant_buffer);
                context_->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context_->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context_->CSSetShaderResources(0, 1, &(cloth.pc_srvs_));
                context_->Dispatch(pin_amount, 1, 1);

                BindCShader(streaching_constraints_shader);
                context_->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context_->CSSetConstantBuffers(1, 1, &compliance_constant_buffer);
                context_->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context_->CSSetConstantBuffers(3, 1, &resolutiom_constant_buffer);

                context_->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(1, 1, &cloth.jacobi_uav_, nullptr);
                for (int i = 0; i < 4; i++)
                {
                    int g = static_cast<int>(cloth.structural_constraints_[i].size());
                    int ydim = 4;
                    int zdim = 4;
                    float xdim_f = static_cast<float>(g) / static_cast<float>(ydim * zdim * 128);
                    float xdim = static_cast<int>(ceil(xdim_f));
                    int z_mul = static_cast<int>(xdim_f * ydim * 128);
                    int resolution = xdim * 32;
                    resolution_data.resolution = resolution;
                    resolution_data.z_multiplier = z_mul;
                    SetCBuffer(resolutiom_constant_buffer, resolution_data);
                    context_->CSSetShaderResources(0, 1, &(cloth.sc_srvs_[i]));
                    context_->Dispatch(xdim, 4, 4);
                }

                BindCShader(streaching_constraints_jacobi_shader);
                context_->CSSetConstantBuffers(3, 1, &resolutiom_constant_buffer);

                for (int i = 4; i < 8; i++)
                {
                    int g = static_cast<int>(cloth.structural_constraints_[i].size());
                    int ydim = 4;
                    int zdim = 4;
                    float xdim_f = static_cast<float>(g) / static_cast<float>(ydim * zdim * 128);
                    float xdim = static_cast<int>(ceil(xdim_f));
                    int z_mul = static_cast<int>(xdim_f * ydim * 128);
                    int resolution = xdim * 32;
                    resolution_data.resolution = resolution;
                    resolution_data.z_multiplier = z_mul;
                    SetCBuffer(resolutiom_constant_buffer, resolution_data);
                    context_->CSSetShaderResources(0, 1, &(cloth.sc_srvs_[i]));
                    context_->Dispatch(xdim, ydim, zdim);
                }


                SetCBuffer(compliance_constant_buffer, bending_compliance_data);

                for (int i = 0; i < 4; i++)
                {
                    int g = static_cast<int>(cloth.structural_constraints_[i].size());
                    int ydim = 4;
                    int zdim = 4;
                    float xdim_f = static_cast<float>(g) / static_cast<float>(ydim * zdim * 128);
                    float xdim = static_cast<int>(ceil(xdim_f));
                    int z_mul = static_cast<int>(xdim_f * ydim * 128);
                    int resolution = xdim * 32;
                    resolution_data.resolution = resolution;
                    resolution_data.z_multiplier = z_mul;
                    SetCBuffer(resolutiom_constant_buffer, resolution_data);
                    context_->CSSetShaderResources(0, 1, &(cloth.bending_srvs_[i]));
                    context_->Dispatch(xdim, ydim, zdim);
                }

                BindCShader(update_velocity_shader);
                context_->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context_->CSSetConstantBuffers(1, 1, &gravity_constant_buffer);
                context_->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context_->CSSetConstantBuffers(3, 1, &resolutiom_constant_buffer);
                context_->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(1, 1, &cloth.previous_positions_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(2, 1, &cloth.velocity_uav_, nullptr);
                context_->CSSetUnorderedAccessViews(3, 1, &cloth.jacobi_uav_, nullptr);
                resolution_data.resolution = cloth.resolution_;
                resolution_data.z_multiplier = cloth.resolution_ * cloth.resolution_ / 4;
                SetCBuffer(resolutiom_constant_buffer, resolution_data);
                context_->Dispatch(cloth.resolution_multiplier_, cloth.resolution_multiplier_, 4);

            }

            cloth.TangentUpdate(context_);
            if (step_sim)
            {
                step_sim = false;
            }
        }
#pragma endregion

#pragma region Shadow map

        ID3D11RenderTargetView* shadowmap_render_targets[2] = { depth_render_target_view, lightspace_position_render_target_view };
        context_->ClearRenderTargetView(depth_render_target_view, white);
        context_->ClearRenderTargetView(lightspace_position_render_target_view, white);
        context_->OMSetRenderTargets(2, shadowmap_render_targets, depth_stencil_view_);
        context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        BindShaders(shadowmap_shader);

        context_->VSSetConstantBuffers(0, 1, &view_proj_constant_buffer);
        context_->VSSetConstantBuffers(1, 1, &model_matrix_constant_buffer);
        SetCBuffer(view_proj_constant_buffer, lightspace_data);
        SetCBuffer(model_matrix_constant_buffer, model_matrix_data);
        context_->VSSetShaderResources(0, 1, &cloth.position_srv_);
        cloth.Draw(context_);

#pragma endregion

#pragma region GBuffer

        ID3D11RenderTargetView* gbuffer_render_targets[3] = { position_render_target_view, normal_render_target_view, color_render_target_view };
        context_->ClearRenderTargetView(position_render_target_view, bgc);
        context_->ClearRenderTargetView(normal_render_target_view, bgc);
        context_->ClearRenderTargetView(color_render_target_view, bgc);
        context_->OMSetRenderTargets(3, gbuffer_render_targets, depth_stencil_view_);
        context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        BindShaders(gbuffer_grid_shader);

        context_->VSSetConstantBuffers(0, 1, &view_proj_constant_buffer);
        context_->VSSetConstantBuffers(1, 1, &camera_constant_buffer);
        context_->VSSetConstantBuffers(2, 1, &model_matrix_constant_buffer);
        context_->PSSetConstantBuffers(3, 1, &grid_constant_buffer);
        context_->PSSetConstantBuffers(4, 1, &bg_color_constant_buffer);
        SetCBuffer(view_proj_constant_buffer, view_proj_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(model_matrix_constant_buffer, model_matrix_data);
        SetCBuffer(grid_constant_buffer, grid_data);
        SetCBuffer(bg_color_constant_buffer, background_color_data);

        context_->IASetVertexBuffers(0, 1, &rectangle_vertex_buffer, &stride, &offset);
        context_->IASetIndexBuffer(rectangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context_->DrawIndexed(6, 0, 0);

        context_->Flush();
        BindShaders(gbuffer_shader);
        context_->PSSetConstantBuffers(0, 1, &color_constant_buffer);
        context_->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
        context_->VSSetConstantBuffers(2, 1, &camera_constant_buffer);
        context_->VSSetConstantBuffers(3, 1, &model_matrix_constant_buffer);
        context_->VSSetShaderResources(0, 1, &cloth.position_srv_);
        context_->VSSetShaderResources(1, 1, &cloth.normal_srv_);
        SetCBuffer(color_constant_buffer, color_data);
        SetCBuffer(view_proj_constant_buffer, view_proj_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(model_matrix_constant_buffer, model_matrix_data);

        if (render_wireframe)
        {
            RenderWireframe();
        }

        cloth.Draw(context_);
        RenderSolid();

#pragma endregion

#pragma region Light

        context_->OMSetRenderTargets(1, &main_render_target_view_, depth_stencil_view_);
        context_->ClearRenderTargetView(main_render_target_view_, bgc);
        context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        BindShaders(light_shader);
        context_->PSSetConstantBuffers(0, 1, &view_proj_constant_buffer);
        context_->PSSetConstantBuffers(1, 1, &camera_constant_buffer);
        context_->PSSetConstantBuffers(2, 1, &spotlight_constant_buffer);

        context_->PSSetShaderResources(0, 1, &position_shader_resource_view);
        context_->PSSetShaderResources(1, 1, &normal_shader_resource_view);
        context_->PSSetShaderResources(2, 1, &color_shader_resource_view);
        context_->PSSetShaderResources(3, 1, &depth_shader_resource_view);
        context_->PSSetShaderResources(4, 1, &lightspace_position_shader_resource_view);

        SetCBuffer(view_proj_constant_buffer, lightspace_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(spotlight_constant_buffer, spotlight_data);

        context_->IASetVertexBuffers(0, 1, &screen_quad_vertex_buffer, &stride, &offset);
        context_->IASetIndexBuffer(screen_quad_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context_->DrawIndexed(6, 0, 0);

#pragma endregion

#pragma region IMGUI

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Menu");

        ImGui::SliderInt("Resolution", &resolution, 1, 10);
        if (ImGui::Button("Apply new resolution"))
        {
            cloth.CleanUp();
            cloth.Init(resolution, device_);
        }

        ImGui::Checkbox("Render wireframe", &render_wireframe);

        ImGui::SliderInt("Quality steps", &quality_steps, 5, 80);
        ImGui::SliderFloat("Structure elasticity", &structural_compliance_data.alpha, 0.000025f, 0.5f, "%.4f");
        ImGui::SliderFloat("Flexibility", &bending_compliance_data.alpha, 0.001f, 0.5f);
        ImGui::SliderFloat("Gravity strength", &gravity_data.y, -100.0f, 0.0f);
        ImGui::SliderFloat("Wind strength", &wind_data.strength_mul, 0.0f, 100.0f);
        ImGui::SliderInt("Pin amount", &pin_amount, 0, 10);

        if (ImGui::Button("Stop simulation"))
        {
            run_sim = false;
        }

        if (ImGui::Button("Run simulation"))
        {
            run_sim = true;
        }

        if (ImGui::Button("Step through simulation"))
        {
            if (run_sim)
            {
                run_sim = false;
            }

            step_sim = true;
        }

        ImGui::SliderFloat("Offset", &grid_data.offset, 0.1f, 5.0f);
        ImGui::SliderFloat("Width", &grid_data.width, 0.001f, 0.2f);

        float fps = ImGui::GetIO().Framerate;

        ImGui::LabelText(("FPS: " + std::to_string(fps)).c_str(), "");

        ImGui::End();

        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#pragma endregion

        swap_chain_->Present(0, 0);
    }
}

