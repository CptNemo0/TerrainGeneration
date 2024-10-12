#include "../include/global.h"

#include <d3d11.h>
#include <tchar.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <wrl/client.h>
#include <xmmintrin.h>
#include <WinBase.h>

#include <algorithm>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "../include/Shader.h"
#include "../include/ConstantBufferStructs.h"

// Main code
int main(int, char**)
{
#pragma region Initialization

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

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));

    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    device->CreateBlendState(&blendDesc, &blendState);

    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(blendState, blendFactor, sampleMask);
    
#pragma endregion     
    
#pragma region Resources Initialization

    ID3D11Buffer* triangle_vertex_buffer = nullptr;
    D3D11_BUFFER_DESC traingle_buffer_description;
    D3D11_SUBRESOURCE_DATA traingle_vertex_srd;

    ZeroMemory(&traingle_buffer_description, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&traingle_vertex_srd, sizeof(D3D11_SUBRESOURCE_DATA));

    traingle_buffer_description.ByteWidth = sizeof(float) * 3 * 6;
    traingle_buffer_description.Usage = D3D11_USAGE_DEFAULT;
    traingle_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    traingle_buffer_description.CPUAccessFlags = 0;
    traingle_buffer_description.MiscFlags = 0;
    traingle_buffer_description.StructureByteStride = sizeof(float) * 6;

    traingle_vertex_srd.pSysMem = triangle_verticies;
    traingle_vertex_srd.SysMemPitch = 0;
    traingle_vertex_srd.SysMemSlicePitch = 0;

    device->CreateBuffer(&traingle_buffer_description, &traingle_vertex_srd, &triangle_vertex_buffer);

    ID3D11Buffer* triangle_index_buffer;
    D3D11_BUFFER_DESC triangle_index_buffer_description = { 0 };
    triangle_index_buffer_description.ByteWidth = sizeof(unsigned int) * 3;
    triangle_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA triangle_index_srd = { triangle_indices, 0, 0 };
    device->CreateBuffer(&triangle_index_buffer_description, &triangle_index_srd, &triangle_index_buffer);

    // Rectangle Buffers Buffer
    ID3D11Buffer* rectangle_vertex_buffer;
    D3D11_BUFFER_DESC rectangle_buffer_description = { 0 };
    rectangle_buffer_description.ByteWidth = sizeof(float) * 4 * 6;
    rectangle_buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    rectangle_buffer_description.StructureByteStride = sizeof(float) * 6;
    D3D11_SUBRESOURCE_DATA rectangle_vertex_srd = { rectangle_verticies, 0, 0 };
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
    D3D11_SUBRESOURCE_DATA screen_quad_vertex_srd = { screen_quad_verticies, 0, 0 };
    device->CreateBuffer(&screen_quad_buffer_description, &screen_quad_vertex_srd, &screen_quad_vertex_buffer);

    ID3D11Buffer* screen_quad_index_buffer;
    D3D11_BUFFER_DESC screen_quad_index_buffer_description = { 0 };
    screen_quad_index_buffer_description.ByteWidth = sizeof(unsigned int) * 3 * 2;
    screen_quad_index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA screen_quad_index_srd = { screen_quad_indices, 0, 0 };
    device->CreateBuffer(&screen_quad_index_buffer_description, &screen_quad_index_srd, &screen_quad_index_buffer);
    // Screen Quad Buffers End

    // New texture
    ID3D11Texture2D* aux_texture;
    ID3D11RenderTargetView* aux_render_target_view;
    ID3D11ShaderResourceView* aux_shader_resource_view;
    D3D11_TEXTURE2D_DESC texture_description = {};
    texture_description.Width = WIDTH;  
    texture_description.Height = HEIGHT;
    texture_description.MipLevels = 1;
    texture_description.ArraySize = 1;
    texture_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_description.SampleDesc.Count = 1;
    texture_description.Usage = D3D11_USAGE_DEFAULT;
    texture_description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    if (device->CreateTexture2D(&texture_description, nullptr, &aux_texture))
    {
        std::cout << "Creating aux texture failed\n";
        exit(1);
    }

    if (device->CreateShaderResourceView(aux_texture, nullptr, &aux_shader_resource_view))
    {
        std::cout << "Creating aux_shader_resource_view failed\n";
        exit(1);
    }

    if (device->CreateRenderTargetView(aux_texture, nullptr, &aux_render_target_view))
    {
        std::cout << "Creating aux_shader_resource_view failed\n";
        exit(1);
    }
    //New texture end

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

    ColorBuffer color_data;
    color_data.color = DirectX::XMFLOAT4(1.0, 0.0, 1.0, 1.0);

    GridBuffer grid_data;
    grid_data.offset = 1.0f;
    grid_data.width = 0.05f;

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
    spotlight_data.position = DirectX::XMVECTOR({ 15.0f, 15.0f, 15.0f, 1.0f });
    spotlight_data.direction = DirectX::XMVector4Normalize({ -15.0f, -15.0f, -15.0f, 1.0f });
    spotlight_data.diffuse_color = DirectX::XMVECTOR({ 1.0f, 1.0f, 1.0f, 1.0f });
    spotlight_data.specular_color = DirectX::XMVECTOR({ 1.0f, 1.0f, 1.0f, 1.0f });
    spotlight_data.cut_off = 0.91f;
    spotlight_data.outer_cut_off = 0.82f;
    spotlight_data.intensity = 800.0f;
   
    ID3D11Buffer* color_constant_buffer;
    D3D11_BUFFER_DESC color_constant_buffer_description = { 0 };
    color_constant_buffer_description.ByteWidth = sizeof(ColorBuffer);
    color_constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    color_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    color_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA color_constant_buffer_srd;
    color_constant_buffer_srd.pSysMem = &color_data;
    color_constant_buffer_srd.SysMemPitch = 0;
    color_constant_buffer_srd.SysMemSlicePitch = 0;
    device->CreateBuffer(&color_constant_buffer_description, &color_constant_buffer_srd, &color_constant_buffer);
    context->PSSetConstantBuffers(0, 1, &color_constant_buffer);

    ID3D11Buffer* view_proj_constant_buffer;
    D3D11_BUFFER_DESC view_proj_constant_buffer_description = { 0 };
    view_proj_constant_buffer_description.ByteWidth = sizeof(ViewProjBuffer);
    view_proj_constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    view_proj_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    view_proj_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA view_proj_constant_buffer_srd;
    view_proj_constant_buffer_srd.pSysMem = &view_proj_data;
    view_proj_constant_buffer_srd.SysMemPitch = 0;
    view_proj_constant_buffer_srd.SysMemSlicePitch = 0;
    device->CreateBuffer(&view_proj_constant_buffer_description, &view_proj_constant_buffer_srd, &view_proj_constant_buffer);
    context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);

    ID3D11Buffer* camera_constant_buffer;
    D3D11_BUFFER_DESC camera_constant_buffer_description = { 0 };
    camera_constant_buffer_description.ByteWidth = sizeof(CameraBuffer);
    camera_constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    camera_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    camera_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA camera_constant_buffer_srd;
    camera_constant_buffer_srd.pSysMem = &camera_data;
    camera_constant_buffer_srd.SysMemPitch = 0;
    camera_constant_buffer_srd.SysMemSlicePitch = 0;
    device->CreateBuffer(&camera_constant_buffer_description, &camera_constant_buffer_srd, &camera_constant_buffer);
    context->VSSetConstantBuffers(2, 1, &camera_constant_buffer);

    ID3D11Buffer* mm_constant_buffer;
    D3D11_BUFFER_DESC mm_constant_buffer_description = { 0 };
    mm_constant_buffer_description.ByteWidth = sizeof(ModelMatrixBuffer);
    mm_constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    mm_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    mm_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA mm_constant_buffer_srd;
    mm_constant_buffer_srd.pSysMem = &mm_data;
    mm_constant_buffer_srd.SysMemPitch = 0;
    mm_constant_buffer_srd.SysMemSlicePitch = 0;
    device->CreateBuffer(&mm_constant_buffer_description, &mm_constant_buffer_srd, &mm_constant_buffer);
    context->VSSetConstantBuffers(3, 1, &mm_constant_buffer);

    ID3D11Buffer* grid_constant_buffer;
    D3D11_BUFFER_DESC grid_constant_buffer_description = { 0 };
    grid_constant_buffer_description.ByteWidth = sizeof(GridBuffer);
    grid_constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    grid_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    grid_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA grid_constant_buffer_srd;
    grid_constant_buffer_srd.pSysMem = &grid_data;
    grid_constant_buffer_srd.SysMemPitch = 0;
    grid_constant_buffer_srd.SysMemSlicePitch = 0;
    device->CreateBuffer(&grid_constant_buffer_description, &grid_constant_buffer_srd, &grid_constant_buffer);
    context->PSSetConstantBuffers(4, 1, &grid_constant_buffer);

    ID3D11Buffer* spotlight_constant_buffer;
    D3D11_BUFFER_DESC spotlight_constant_buffer_description = { 0 };
    spotlight_constant_buffer_description.ByteWidth = sizeof(SpotlightBuffer);
    spotlight_constant_buffer_description.Usage = D3D11_USAGE_DYNAMIC;
    spotlight_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    spotlight_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA spotlight_constant_buffer_srd;
    spotlight_constant_buffer_srd.pSysMem = &grid_data;
    spotlight_constant_buffer_srd.SysMemPitch = 0;
    spotlight_constant_buffer_srd.SysMemSlicePitch = 0;
    device->CreateBuffer(&spotlight_constant_buffer_description, &spotlight_constant_buffer_srd, &spotlight_constant_buffer);
    context->PSSetConstantBuffers(5, 1, &spotlight_constant_buffer);

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
    
#pragma endregion
 
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
                    view_proj_data.view_matrix = DirectX::XMMatrixLookAtLH(camera_position_iv, center_position_iv, up_direction_iv);
                }
            }
        }
        if (done)
        {
            break;
        }

#pragma endregion

        const float clear_color_with_alpha[4] = {
            0.65f * 0.15f,
            0.65f * 0.15f,
            0.75f * 0.15f,
            1.0f
        };
        
        //======================== Logic
        D3D11_MAPPED_SUBRESOURCE mapped_resource;
        if (color_constant_buffer)
        {
            context->Map(color_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            ColorBuffer* data_ptr = (ColorBuffer*)mapped_resource.pData;
            *data_ptr = color_data;  // Update the data
            context->Unmap(color_constant_buffer, 0);
        }
        
        if (view_proj_constant_buffer)
        {
            context->Map(view_proj_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            ViewProjBuffer* data_ptr = (ViewProjBuffer*)mapped_resource.pData;
            *data_ptr = view_proj_data;
            context->Unmap(view_proj_constant_buffer, 0);
        }

        if (camera_constant_buffer)
        {
            context->Map(camera_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            CameraBuffer* data_ptr = (CameraBuffer*)mapped_resource.pData;
            camera_data.camera_position = camera_position_iv;
            *data_ptr = camera_data;
            context->Unmap(camera_constant_buffer, 0);
        }

        if (mm_constant_buffer)
        {
            context->Map(mm_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            ModelMatrixBuffer* data_ptr = (ModelMatrixBuffer*)mapped_resource.pData;
            *data_ptr = mm_data;
            context->Unmap(mm_constant_buffer, 0);
        }

        if (grid_constant_buffer)
        {
            context->Map(grid_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            GridBuffer* data_ptr = (GridBuffer*)mapped_resource.pData;
            *data_ptr = grid_data;
            context->Unmap(grid_constant_buffer, 0);
        }

        if (spotlight_constant_buffer)
        {
            context->Map(spotlight_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
            SpotlightBuffer* data_ptr = (SpotlightBuffer*)mapped_resource.pData;
            *data_ptr = spotlight_data;
            context->Unmap(spotlight_constant_buffer, 0);
        }

        context->OMSetRenderTargets(1, &aux_render_target_view, depth_stencil_view);
        context->ClearRenderTargetView(aux_render_target_view, clear_color_with_alpha);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        context->VSSetShader(grid_vertex_shader, nullptr, 0);
        context->PSSetShader(grid_pixel_shader, nullptr, 0);
        
        context->IASetVertexBuffers(0, 1, &rectangle_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(rectangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
        context->DrawIndexed(6, 0, 0);
        
        context->VSSetShader(vertex_shader, nullptr, 0);
        context->PSSetShader(pixel_shader, nullptr, 0);

        context->IASetVertexBuffers(0, 1, &triangle_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(triangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
        context->DrawIndexed(3, 0, 0);

        // Render quad
        context->OMSetRenderTargets(1, &main_render_target_view, depth_stencil_view);
        context->ClearRenderTargetView(main_render_target_view, clear_color_with_alpha);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        context->VSSetShader(screen_vertex_shader, nullptr, 0);
        context->PSSetShader(screen_pixel_shader, nullptr, 0);
        
        context->IASetVertexBuffers(0, 1, &screen_quad_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(screen_quad_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->PSSetShaderResources(0, 1, &aux_shader_resource_view);
        
        context->DrawIndexed(6, 0, 0);

        //======================== End Logic

#pragma region IMGUI

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Color");
        float color_picker[4] = { color_data.color.x, color_data.color.y, color_data.color.z, color_data.color.w };
        if (ImGui::ColorPicker4("Pick a color", color_picker))
        {
            color_data.color = DirectX::XMFLOAT4(color_picker[0], color_picker[1], color_picker[2], color_picker[3]);
        }

        ImGui::SliderFloat("Offset", &grid_data.offset, 0.1f, 5.0f);
        ImGui::SliderFloat("Width", &grid_data.width, 0.01f, 0.2f);

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