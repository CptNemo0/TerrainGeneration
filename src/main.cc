#include "../include/global.h"

#include <d3d11.h>
#include <tchar.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <wrl/client.h>
#include <xmmintrin.h>
#include <WinBase.h>

#include <algorithm>
#include <vector>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "../include/Shader.h"
#include "../include/ConstantBufferStructs.h"
#include "../include/Structs.h"


// Main code
int main(int, char**)
{
#pragma region Initialization

    ULONGLONG start_time = GetTickCount64();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Cloth Simulation"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("ClothSimulation"), WS_OVERLAPPEDWINDOW, 100, 100, WIDTH, HEIGHT, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    ImGui_ImplWin32_Init(hwnd);  // Pass the window handle here
    ImGui_ImplDX11_Init(device, context);

    //===============================

    InitViewport();
    context->RSSetViewports(1, &viewport);

    InitDepthStencilBuffer();
    context->OMSetDepthStencilState(depth_stencil_state, 1);
    context->OMSetRenderTargets(1, &main_render_target_view, depth_stencil_view);
    
    InitRasterizer();
    context->RSSetState(rasterizer_state);
    
    DirectX::XMVECTOR  camera_position_iv = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    float camera_distance = 3.0f;
    DirectX::XMVECTOR  camera_angles = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR  center_position_iv = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR  up_direction_iv =    DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    auto view_matrix = DirectX::XMMatrixLookAtLH(camera_position_iv, center_position_iv, up_direction_iv);
    auto projection_matrix = DirectX::XMMatrixPerspectiveFovLH(0.7864f, 16.0f/9.0f, 0.1f, 1000.0f);
    
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_BLEND_DESC blend_desciption;
    ZeroMemory(&blend_desciption, sizeof(blend_desciption));

    blend_desciption.RenderTarget[0].BlendEnable = TRUE;
    blend_desciption.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desciption.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desciption.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desciption.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desciption.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desciption.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desciption.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blend_state;
    device->CreateBlendState(&blend_desciption, &blend_state);

    float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(blend_state, blend_factor, sampleMask);
    
#pragma endregion     
    
#pragma region Resources Initialization

    std::vector<Vertex> triangle_vertices
    {
        {0.0f ,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f , -1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        {-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f}
    };

    int resolution = 32;
    // Generate vertices 
    {
        triangle_vertices.clear();

        
        float offset = 2.0f / (static_cast<float>(resolution) - 1.0f);

        float start_x = -1.0f;
        float start_y = 1.0f;

        for (int i = 0; i < resolution; i++)
        {
            for (int j = 0; j < resolution; j++)
            {
                triangle_vertices.emplace_back(start_x + j * offset,
                                               start_y - i * offset,
                                               0.0f,
                                               0.0f,
                                               0.0f,
                                               1.0f);
            }
        }
    }
    //

    std::vector<Face> triangle_faces
    {
        { 0, 1, 2 }
    };

    //Generate faces
    {
        triangle_faces.clear();

        bool up = true;

        for (int i = 0; i < resolution - 1; i++)
        {
            for (int j = 0; j < resolution - 1; j++)
            {
                int idx = resolution * i + j;
                
                if (up)
                {
                    up = false;
                    triangle_faces.emplace_back(idx, idx + 1, idx + resolution);
                    triangle_faces.emplace_back(idx + 1, idx + 1 + resolution, idx + 1 + resolution - 1);
                }
                else
                {
                    up = true;
                    triangle_faces.emplace_back(idx, idx + resolution + 1, idx + resolution);
                    triangle_faces.emplace_back(idx, idx + 1, idx + resolution + 1);
                }
            }
        }
    }
    //
    
    ID3D11Buffer* triangle_vertex_buffer = nullptr;
    D3D11_BUFFER_DESC traingle_buffer_description;
    D3D11_SUBRESOURCE_DATA traingle_vertex_srd;

    ZeroMemory(&traingle_buffer_description, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&traingle_vertex_srd, sizeof(D3D11_SUBRESOURCE_DATA));

    traingle_buffer_description.ByteWidth = sizeof(Vertex) * triangle_vertices.size();
    traingle_buffer_description.Usage = D3D11_USAGE_DEFAULT;
    traingle_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    traingle_buffer_description.CPUAccessFlags = 0;
    traingle_buffer_description.MiscFlags = 0;
    traingle_buffer_description.StructureByteStride = sizeof(Vertex);

    traingle_vertex_srd.pSysMem = &(triangle_vertices[0]);
    traingle_vertex_srd.SysMemPitch = 0;
    traingle_vertex_srd.SysMemSlicePitch = 0;

    device->CreateBuffer(&traingle_buffer_description, &traingle_vertex_srd, &triangle_vertex_buffer);

    ID3D11Buffer* triangle_index_buffer;
    D3D11_BUFFER_DESC triangle_index_buffer_description = { 0 };
    triangle_index_buffer_description.ByteWidth = sizeof(Face) * triangle_faces.size();
    triangle_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA triangle_index_srd = { &(triangle_faces[0]), 0, 0 };
    device->CreateBuffer(&triangle_index_buffer_description, &triangle_index_srd, &triangle_index_buffer);

    // Rectangle Buffers Buffer
    ID3D11Buffer* rectangle_vertex_buffer;
    D3D11_BUFFER_DESC rectangle_buffer_description = { 0 };
    rectangle_buffer_description.ByteWidth = sizeof(float) * 4 * 6;
    rectangle_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    rectangle_buffer_description.StructureByteStride = sizeof(float) * 6;
    D3D11_SUBRESOURCE_DATA rectangle_vertex_srd = { rectangle_vertices, 0, 0 };
    device->CreateBuffer(&rectangle_buffer_description, &rectangle_vertex_srd, &rectangle_vertex_buffer);

    ID3D11Buffer* rectangle_index_buffer;
    D3D11_BUFFER_DESC rectangle_index_buffer_description = { 0 };
    rectangle_index_buffer_description.ByteWidth = sizeof(unsigned int) * 3 * 2;
    rectangle_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA rectangle_index_srd = { rectangle_indices, 0, 0 };
    device->CreateBuffer(&rectangle_index_buffer_description, &rectangle_index_srd, &rectangle_index_buffer);
    // Rectangle Buffers End

    // Screen Quad Buffers
    ID3D11Buffer* screen_quad_vertex_buffer;
    D3D11_BUFFER_DESC screen_quad_buffer_description = { 0 };
    screen_quad_buffer_description.ByteWidth = sizeof(float) * 4 * 6;
    screen_quad_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    screen_quad_buffer_description.StructureByteStride = sizeof(float) * 6;
    D3D11_SUBRESOURCE_DATA screen_quad_vertex_srd = { screen_quad_vertices, 0, 0 };
    device->CreateBuffer(&screen_quad_buffer_description, &screen_quad_vertex_srd, &screen_quad_vertex_buffer);

    ID3D11Buffer* screen_quad_index_buffer;
    D3D11_BUFFER_DESC screen_quad_index_buffer_description = { 0 };
    screen_quad_index_buffer_description.ByteWidth = sizeof(unsigned int) * 3 * 2;
    screen_quad_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA screen_quad_index_srd = { screen_quad_indices, 0, 0 };
    device->CreateBuffer(&screen_quad_index_buffer_description, &screen_quad_index_srd, &screen_quad_index_buffer);
    // Screen Quad Buffers End
#pragma endregion

#pragma region Textures

    D3D11_TEXTURE2D_DESC texture_description = {0};
    texture_description.Width = WIDTH;
    texture_description.Height = HEIGHT;
    texture_description.MipLevels = 1;
    texture_description.ArraySize = 1;
    texture_description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texture_description.SampleDesc.Count = 1;
    texture_description.Usage = D3D11_USAGE_DEFAULT;
    texture_description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* position_texture;
    ID3D11RenderTargetView* position_render_target_view;
    ID3D11ShaderResourceView* position_shader_resource_view;
    if (device->CreateTexture2D(&texture_description, nullptr, &position_texture))
    {
        std::cout << "Creating position texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(position_texture, nullptr, &position_shader_resource_view))
    {
        std::cout << "Creating position_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(position_texture, nullptr, &position_render_target_view))
    {
        std::cout << "Creating position_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* normal_texture;
    ID3D11RenderTargetView* normal_render_target_view;
    ID3D11ShaderResourceView* normal_shader_resource_view;
    if (device->CreateTexture2D(&texture_description, nullptr, &normal_texture))
    {
        std::cout << "Creating normal texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(normal_texture, nullptr, &normal_shader_resource_view))
    {
        std::cout << "Creating normal_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(normal_texture, nullptr, &normal_render_target_view))
    {
        std::cout << "Creating normal_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* color_texture;
    ID3D11RenderTargetView* color_render_target_view;
    ID3D11ShaderResourceView* color_shader_resource_view;
    if (device->CreateTexture2D(&texture_description, nullptr, &color_texture))
    {
        std::cout << "Creating color texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(color_texture, nullptr, &color_shader_resource_view))
    {
        std::cout << "Creating color_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(color_texture, nullptr, &color_render_target_view))
    {
        std::cout << "Creating color_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* lightspace_position_texture;
    ID3D11RenderTargetView* lightspace_position_render_target_view;
    ID3D11ShaderResourceView* lightspace_position_shader_resource_view;
    if (device->CreateTexture2D(&texture_description, nullptr, &lightspace_position_texture))
    {
        std::cout << "Creating lightspace_position texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(lightspace_position_texture, nullptr, &lightspace_position_shader_resource_view))
    {
        std::cout << "Creating lightspace_position_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(lightspace_position_texture, nullptr, &lightspace_position_render_target_view))
    {
        std::cout << "Creating lightspace_position_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* fxaa_texture;
    ID3D11RenderTargetView* fxaa_render_target_view;
    ID3D11ShaderResourceView* fxaa_shader_resource_view;
    if (device->CreateTexture2D(&texture_description, nullptr, &fxaa_texture))
    {
        std::cout << "Creating fxaa texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(fxaa_texture, nullptr, &fxaa_shader_resource_view))
    {
        std::cout << "Creating fxaa_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(fxaa_texture, nullptr, &fxaa_render_target_view))
    {
        std::cout << "Creating fxaa_shader_resource_view failed\n";
        exit(1);
    }

    texture_description = {0};
    texture_description.Width = WIDTH;
    texture_description.Height = HEIGHT;
    texture_description.MipLevels = 1;
    texture_description.ArraySize = 1;
    texture_description.Format = DXGI_FORMAT_R32_FLOAT;
    texture_description.SampleDesc.Count = 1;
    texture_description.Usage = D3D11_USAGE_DEFAULT;
    texture_description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* depth_texture;
    ID3D11RenderTargetView* depth_render_target_view;
    ID3D11ShaderResourceView* depth_shader_resource_view;
    if (device->CreateTexture2D(&texture_description, nullptr, &depth_texture))
    {
        std::cout << "Creating depth texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(depth_texture, nullptr, &depth_shader_resource_view))
    {
        std::cout << "Creating depth_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(depth_texture, nullptr, &depth_render_target_view))
    {
        std::cout << "Creating depth_shader_resource_view failed\n";
        exit(1);
    }    
#pragma endregion

#pragma region Shaders

    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader* pixel_shader;
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    CompileShaders(&vs_blob, &ps_blob, L"../res/Shaders/Shader.hlsl");
    device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vertex_shader);
    device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &pixel_shader);

    ID3D11VertexShader* grid_vertex_shader;
    ID3D11PixelShader* grid_pixel_shader;
    ID3DBlob* gvs_blob = nullptr;
    ID3DBlob* gps_blob = nullptr;
    CompileShaders(&gvs_blob, &gps_blob, L"../res/Shaders/GridShader.hlsl");
    device->CreateVertexShader(gvs_blob->GetBufferPointer(), gvs_blob->GetBufferSize(), nullptr, &grid_vertex_shader);
    device->CreatePixelShader(gps_blob->GetBufferPointer(), gps_blob->GetBufferSize(), nullptr, &grid_pixel_shader);

    ID3D11VertexShader* screen_vertex_shader;
    ID3D11PixelShader* screen_pixel_shader;
    ID3DBlob* svs_blob = nullptr;
    ID3DBlob* sps_blob = nullptr;
    CompileShaders(&svs_blob, &sps_blob, L"../res/Shaders/ScreenQuad.hlsl");
    device->CreateVertexShader(svs_blob->GetBufferPointer(), svs_blob->GetBufferSize(), nullptr, &screen_vertex_shader);
    device->CreatePixelShader(sps_blob->GetBufferPointer(), sps_blob->GetBufferSize(), nullptr, &screen_pixel_shader);

    ID3D11VertexShader* sm_vertex_shader;
    ID3D11PixelShader* sm_pixel_shader;
    ID3DBlob* smvs_blob = nullptr;
    ID3DBlob* smps_blob = nullptr;
    CompileShaders(&smvs_blob, &smps_blob, L"../res/Shaders/ShadowMap.hlsl");
    device->CreateVertexShader(smvs_blob->GetBufferPointer(), smvs_blob->GetBufferSize(), nullptr, &sm_vertex_shader);
    device->CreatePixelShader(smps_blob->GetBufferPointer(), smps_blob->GetBufferSize(), nullptr, &sm_pixel_shader);

    ID3D11VertexShader* g_vertex_shader;
    ID3D11PixelShader* g_pixel_shader;
    ID3DBlob* gv_blob = nullptr;
    ID3DBlob* gp_blob = nullptr;
    CompileShaders(&gv_blob, &gp_blob, L"../res/Shaders/GBuffer.hlsl");
    device->CreateVertexShader(gv_blob->GetBufferPointer(), gv_blob->GetBufferSize(), nullptr, &g_vertex_shader);
    device->CreatePixelShader(gp_blob->GetBufferPointer(), gp_blob->GetBufferSize(), nullptr, &g_pixel_shader);

    ID3D11VertexShader* g_grid_vertex_shader;
    ID3D11PixelShader* g_grid_pixel_shader;
    ID3DBlob* ggv_blob = nullptr;
    ID3DBlob* ggp_blob = nullptr;
    CompileShaders(&ggv_blob, &ggp_blob, L"../res/Shaders/GridGBuffer.hlsl");
    device->CreateVertexShader(ggv_blob->GetBufferPointer(), ggv_blob->GetBufferSize(), nullptr, &g_grid_vertex_shader);
    device->CreatePixelShader(ggp_blob->GetBufferPointer(), ggp_blob->GetBufferSize(), nullptr, &g_grid_pixel_shader);

    ID3D11VertexShader* light_vertex_shader;
    ID3D11PixelShader* light_pixel_shader;
    ID3DBlob* lv_blob = nullptr;
    ID3DBlob* lf_blob = nullptr;
    CompileShaders(&lv_blob, &lf_blob, L"../res/Shaders/Lighting.hlsl");
    device->CreateVertexShader(lv_blob->GetBufferPointer(), lv_blob->GetBufferSize(), nullptr, &light_vertex_shader);
    device->CreatePixelShader(lf_blob->GetBufferPointer(), lf_blob->GetBufferSize(), nullptr, &light_pixel_shader);

    ID3D11VertexShader* fxaa_vertex_shader;
    ID3D11PixelShader* fxaa_pixel_shader;
    ID3DBlob* fxaav_blob = nullptr;
    ID3DBlob* fxaaf_blob = nullptr;
    CompileShaders(&fxaav_blob, &fxaaf_blob, L"../res/Shaders/FXAA.hlsl");
    device->CreateVertexShader(fxaav_blob->GetBufferPointer(), fxaav_blob->GetBufferSize(), nullptr, &fxaa_vertex_shader);
    device->CreatePixelShader(fxaaf_blob->GetBufferPointer(), fxaaf_blob->GetBufferSize(), nullptr, &fxaa_pixel_shader);

    ColorBuffer color_data;
    color_data.color = DirectX::XMVECTOR({ 1.0f, 0.0f, 1.0f, 1.0f });

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

    ModelMatrixBuffer mm_data;
    mm_data.model_matrix = DirectX::XMMatrixIdentity();
    auto matrix_determinant = DirectX::XMMatrixDeterminant(mm_data.model_matrix);
    mm_data.ti_model_matrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&matrix_determinant, mm_data.model_matrix));

    SpotlightBuffer spotlight_data;
    spotlight_data.position = DirectX::XMVECTOR({ 5.0f,5.0f, 5.0f, 1.0f });
    spotlight_data.direction = DirectX::XMVector4Normalize({ -3.0f, -5.0f, -3.0f, 1.0f });
    spotlight_data.diffuse_color = DirectX::XMVECTOR({ 1.0f, 1.0f, 1.0f, 1.0f });
    spotlight_data.specular_color = DirectX::XMVECTOR({ 1.0f, 1.0f, 1.0f, 1.0f });
    spotlight_data.cut_off = 0.91f;
    spotlight_data.outer_cut_off = 0.82f;
    spotlight_data.intensity = 100.0f;
   
    ViewProjBuffer lightspace_data;
    lightspace_data.view_matrix = DirectX::XMMatrixLookAtLH(spotlight_data.position, DirectX::XMVectorAdd(spotlight_data.position, spotlight_data.direction), up_direction_iv);
    lightspace_data.projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(-8.0f, 8.0f, -4.5f, 4.5f, 0.1f, 100.0f);

    
   
    ID3D11Buffer* color_constant_buffer = nullptr;
    CreateCBuffer(&color_constant_buffer, color_data);
    context->PSSetConstantBuffers(0, 1, &color_constant_buffer);

    ID3D11Buffer* view_proj_constant_buffer;
    CreateCBuffer(&view_proj_constant_buffer, view_proj_data); 
    context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
    context->PSSetConstantBuffers(1, 1, &view_proj_constant_buffer);

    ID3D11Buffer* camera_constant_buffer;
    CreateCBuffer(&camera_constant_buffer, camera_data);
    context->VSSetConstantBuffers(2, 1, &camera_constant_buffer);
    context->PSSetConstantBuffers(2, 1, &camera_constant_buffer);

    ID3D11Buffer* mm_constant_buffer;
    CreateCBuffer(&mm_constant_buffer, mm_data);
    context->VSSetConstantBuffers(3, 1, &mm_constant_buffer);

    ID3D11Buffer* grid_constant_buffer;
    CreateCBuffer(&grid_constant_buffer, grid_data);
    context->PSSetConstantBuffers(4, 1, &grid_constant_buffer);

    ID3D11Buffer* spotlight_constant_buffer;
    CreateCBuffer(&spotlight_constant_buffer, spotlight_data);
    context->PSSetConstantBuffers(5, 1, &spotlight_constant_buffer);

    ID3D11Buffer* bg_color_constant_buffer;
    CreateCBuffer(&bg_color_constant_buffer, bg_color_constant_buffer);
    context->PSSetConstantBuffers(6, 1, &bg_color_constant_buffer);

#pragma endregion
    
    // initialize input layout
    D3D11_INPUT_ELEMENT_DESC input_element_description[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    ComPtr<ID3D11InputLayout> input_layout = nullptr;
    device->CreateInputLayout(input_element_description, 2, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout);
    context->IASetInputLayout(input_layout.Get());

    uint32_t stride = sizeof(float) * 6;
    uint32_t offset = 0;
    

 
    // Main loop
    bool done = false;
    bool rotate = false;
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
                done = true;
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
            else if (msg.message = WM_MOUSEMOVE)
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

        grid_data.time = t;
        
        //======================== Logic
        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        
        SetCBuffer(color_constant_buffer, color_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(mm_constant_buffer, mm_data);
        SetCBuffer(grid_constant_buffer, grid_data);
        SetCBuffer(bg_color_constant_buffer, background_color_data);
        SetCBuffer(spotlight_constant_buffer, spotlight_data);

#pragma region Render Depth For Shadow Map
        {
            float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            ID3D11RenderTargetView* render_targets[2] = { depth_render_target_view, lightspace_position_render_target_view};
            context->ClearRenderTargetView(depth_render_target_view, white);
            context->ClearRenderTargetView(lightspace_position_render_target_view, white);
            context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
            context->OMSetRenderTargets(2, render_targets, depth_stencil_view);

            context->VSSetShader(sm_vertex_shader, nullptr, 0);
            context->PSSetShader(sm_pixel_shader, nullptr, 0);

            SetCBuffer(view_proj_constant_buffer, lightspace_data);

            context->IASetVertexBuffers(0, 1, &rectangle_vertex_buffer, &stride, &offset);
            context->IASetIndexBuffer(rectangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
            context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
            context->DrawIndexed(6, 0, 0);

            context->IASetVertexBuffers(0, 1, &triangle_vertex_buffer, &stride, &offset);
            context->IASetIndexBuffer(triangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
            context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
            context->DrawIndexed(3 * triangle_faces.size(), 0, 0);

            SetCBuffer(view_proj_constant_buffer, view_proj_data);
        }
#pragma endregion

        float bgcx = DirectX::XMVectorGetX(background_color_data.color);
        float bgcy = DirectX::XMVectorGetY(background_color_data.color);
        float bgcz = DirectX::XMVectorGetZ(background_color_data.color);
        float bgcw = DirectX::XMVectorGetW(background_color_data.color);

        float bgc[4] = { bgcx , bgcy, bgcz, bgcw};

        ID3D11RenderTargetView* render_targets[3] = { position_render_target_view, normal_render_target_view, color_render_target_view };
        context->OMSetRenderTargets(3, render_targets, depth_stencil_view);
        context->ClearRenderTargetView(position_render_target_view, bgc);
        context->ClearRenderTargetView(normal_render_target_view, bgc);
        context->ClearRenderTargetView(color_render_target_view, bgc);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        context->VSSetShader(g_grid_vertex_shader, nullptr, 0);
        context->PSSetShader(g_grid_pixel_shader, nullptr, 0);
        
        context->IASetVertexBuffers(0, 1, &rectangle_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(rectangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(6, 0, 0);

        context->VSSetShader(g_vertex_shader, nullptr, 0);
        context->PSSetShader(g_pixel_shader, nullptr, 0);

        context->IASetVertexBuffers(0, 1, &triangle_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(triangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(3 * triangle_faces.size(), 0, 0);
        
        // Render quad
        context->OMSetRenderTargets(1, &fxaa_render_target_view, depth_stencil_view);
        context->ClearRenderTargetView(fxaa_render_target_view, bgc);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
        
        SetCBuffer(view_proj_constant_buffer, lightspace_data);

        context->VSSetShader(light_vertex_shader, nullptr, 0);
        context->PSSetShader(light_pixel_shader, nullptr, 0);
        context->PSSetShaderResources(0, 1, &position_shader_resource_view);
        context->PSSetShaderResources(1, 1, &normal_shader_resource_view);
        context->PSSetShaderResources(2, 1, &color_shader_resource_view);
        context->PSSetShaderResources(3, 1, &depth_shader_resource_view);
        context->PSSetShaderResources(4, 1, &lightspace_position_shader_resource_view);

        context->IASetVertexBuffers(0, 1, &screen_quad_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(screen_quad_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(6, 0, 0);

        // postprocessing

        context->OMSetRenderTargets(1, &main_render_target_view, depth_stencil_view);
        context->ClearRenderTargetView(main_render_target_view, bgc);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        context->VSSetShader(fxaa_vertex_shader, nullptr, 0);
        context->PSSetShader(fxaa_pixel_shader, nullptr, 0);
        context->PSSetShaderResources(0, 1, &fxaa_shader_resource_view);

        context->IASetVertexBuffers(0, 1, &screen_quad_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(screen_quad_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(6, 0, 0);

        //======================== End Logic

#pragma region IMGUI

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Color");
        
        if (ImGui::ColorPicker4("Pick a color", bgc))
        {
            background_color_data.color = DirectX::XMVECTOR({ bgc[0], bgc[1], bgc[2], 1.0f });
        }

        ImGui::SliderFloat("Offset", &grid_data.offset, 0.1f, 5.0f);
        ImGui::SliderFloat("Width", &grid_data.width, 0.01f, 0.2f);

        float pos[3] = {
            DirectX::XMVectorGetByIndex(spotlight_data.direction, 0),
            DirectX::XMVectorGetByIndex(spotlight_data.direction, 1),
            DirectX::XMVectorGetByIndex(spotlight_data.direction, 2)
        };
        if (ImGui::DragFloat3("Spotlight", pos, 0.1f))
        {
            spotlight_data.direction = DirectX::XMVectorSet(pos[0], pos[1], pos[2], 1.0f);
            lightspace_data.view_matrix = DirectX::XMMatrixLookAtLH(spotlight_data.position, DirectX::XMVectorAdd(spotlight_data.position, spotlight_data.direction), up_direction_iv);
        }

        ImGui::End();

        ImGui::Render();
        
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#pragma endregion

        swap_chain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);    

    return 0;
}