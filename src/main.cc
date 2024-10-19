#include "../include/global.h"
#include "../include/Shader.h"
#include "../include/ConstantBufferStructs.h"
#include "../include/Structs.h"
#include "../include/Cloth.h"

#include <d3d11.h>
#include <tchar.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <wrl/client.h>
#include <xmmintrin.h>
#include <WinBase.h>

#include <algorithm>
#include <vector>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

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
    if (static_cast<int>(device->CreateTexture2D(&texture_description, nullptr, &position_texture)))
    {
        std::cout << "Creating position texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(position_texture, nullptr, &position_shader_resource_view)))
    {
        std::cout << "Creating position_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateRenderTargetView(position_texture, nullptr, &position_render_target_view)))
    {
        std::cout << "Creating position_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* normal_texture;
    ID3D11RenderTargetView* normal_render_target_view;
    ID3D11ShaderResourceView* normal_shader_resource_view;
    if (static_cast<int>(device->CreateTexture2D(&texture_description, nullptr, &normal_texture)))
    {
        std::cout << "Creating normal texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(normal_texture, nullptr, &normal_shader_resource_view)))
    {
        std::cout << "Creating normal_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateRenderTargetView(normal_texture, nullptr, &normal_render_target_view)))
    {
        std::cout << "Creating normal_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* color_texture;
    ID3D11RenderTargetView* color_render_target_view;
    ID3D11ShaderResourceView* color_shader_resource_view;
    if (static_cast<int>(device->CreateTexture2D(&texture_description, nullptr, &color_texture)))
    {
        std::cout << "Creating color texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(color_texture, nullptr, &color_shader_resource_view)))
    {
        std::cout << "Creating color_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateRenderTargetView(color_texture, nullptr, &color_render_target_view)))
    {
        std::cout << "Creating color_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* lightspace_position_texture;
    ID3D11RenderTargetView* lightspace_position_render_target_view;
    ID3D11ShaderResourceView* lightspace_position_shader_resource_view;
    if (static_cast<int>(device->CreateTexture2D(&texture_description, nullptr, &lightspace_position_texture)))
    {
        std::cout << "Creating lightspace_position texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(lightspace_position_texture, nullptr, &lightspace_position_shader_resource_view)))
    {
        std::cout << "Creating lightspace_position_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateRenderTargetView(lightspace_position_texture, nullptr, &lightspace_position_render_target_view)))
    {
        std::cout << "Creating lightspace_position_shader_resource_view failed\n";
        exit(1);
    }

    ID3D11Texture2D* fxaa_texture;
    ID3D11RenderTargetView* fxaa_render_target_view;
    ID3D11ShaderResourceView* fxaa_shader_resource_view;
    if (static_cast<int>(device->CreateTexture2D(&texture_description, nullptr, &fxaa_texture)))
    {
        std::cout << "Creating fxaa texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(fxaa_texture, nullptr, &fxaa_shader_resource_view)))
    {
        std::cout << "Creating fxaa_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateRenderTargetView(fxaa_texture, nullptr, &fxaa_render_target_view)))
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
    if (static_cast<int>(device->CreateTexture2D(&texture_description, nullptr, &depth_texture)))
    {
        std::cout << "Creating depth texture failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(depth_texture, nullptr, &depth_shader_resource_view)))
    {
        std::cout << "Creating depth_shader_resource_view failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateRenderTargetView(depth_texture, nullptr, &depth_render_target_view)))
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
    dt_data.dt = 0.0055f;
    dt_data.idt = 1.0f / dt_data.dt;
    dt_data.t = 0.0f;
    dt_data.p1 = 0.0f;

    GravityBuffer gravity_data;
    gravity_data.x = 0.0f;
    gravity_data.y = -10.0f;
    gravity_data.z = 0.0f;
    gravity_data.p1 = 0.0f;

    MassBuffer mass_data;
    mass_data.mass = 0.3f;
    mass_data.imass = 1.0f/ mass_data.mass;

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
    ComPtr<ID3D11InputLayout> input_layout = nullptr;
    device->CreateInputLayout(input_element_description, 2, gbuffer_shader.vs_blob->GetBufferPointer(), gbuffer_shader.vs_blob->GetBufferSize(), &input_layout);
    context->IASetInputLayout(input_layout.Get());

    uint32_t stride = sizeof(float) * 6;
    uint32_t offset = 0;
    
    float bgc[4] = 
    { 
        DirectX::XMVectorGetX(background_color_data.color),
        DirectX::XMVectorGetY(background_color_data.color),
        DirectX::XMVectorGetZ(background_color_data.color),
        DirectX::XMVectorGetW(background_color_data.color) 
    };

    Cloth cloth{ 4, device };
    cloth.zero_normals_shader_ = &zero_normal_shader;
    cloth.recalculate_normals_shader_ = &recalculate_normal_shader;
    cloth.stride_ = stride;
    cloth.offset_ = offset;

    // Main loop
    bool done = false;
    bool rotate = false;
    bool render_wireframe = false;

    bool run_sim = true;
    bool step_sim = false;

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
        grid_data.time = t;
        time_data.time = t;
        dt_data.t = t;

        if (run_sim || step_sim)
        {
            SetCBuffer(delta_time_constant_buffer, dt_data);
            SetCBuffer(gravity_constant_buffer, gravity_data);
            SetCBuffer(mass_constant_buffer, mass_data);
            SetCBuffer(wind_constant_buffer, wind_data);
            for (int it = 0; it < quality_steps; it++)
            {
                BindCShader(update_position_shader);
                context->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context->CSSetConstantBuffers(1, 1, &gravity_constant_buffer);
                context->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context->CSSetConstantBuffers(3, 1, &wind_constant_buffer);
                context->CSSetConstantBuffers(4, 1, &resolutiom_constant_buffer);
                resolution_data.resolution = cloth.resolution_;
                resolution_data.z_multiplier = cloth.resolution_ * cloth.resolution_ / 4;
                SetCBuffer(resolutiom_constant_buffer, resolution_data);
                context->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context->CSSetUnorderedAccessViews(1, 1, &cloth.previous_positions_uav_, nullptr);
                context->CSSetUnorderedAccessViews(2, 1, &cloth.velocity_uav_, nullptr);
                context->CSSetUnorderedAccessViews(3, 1, &cloth.jacobi_uav_, nullptr);
                context->Dispatch(cloth.resolution_multiplier_, cloth.resolution_multiplier_, 4);
                
                BindCShader(enforce_pin_shader);
                context->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context->CSSetConstantBuffers(1, 1, &compliance_constant_buffer);
                context->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context->CSSetShaderResources(0, 1, &(cloth.pc_srvs_));
                context->Dispatch(cloth.pin_constraints_.size(), 1, 1);
                context->CSSetShaderResources(0, 1, &cloth.cleaner_srv_);
                context->CSSetUnorderedAccessViews(0, 1, &cleaner_uav, nullptr);

                BindCShader(streaching_constraints_shader);
                context->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context->CSSetConstantBuffers(1, 1, &compliance_constant_buffer);
                context->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                
                context->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context->CSSetUnorderedAccessViews(1, 1, &cloth.jacobi_uav_, nullptr);
                for (int i = 0; i < 4; i++)
                {
                    int x_dim = static_cast<int>(ceil(cloth.structural_constraints_[i].size() / 1024.0f));
                    context->CSSetShaderResources(0, 1, &(cloth.sc_srvs_[i]));
                    context->Dispatch(x_dim, 1, 1);
                    context->CSSetShaderResources(0, 1, &cloth.cleaner_srv_);
                }

                BindCShader(streaching_constraints_jacobi_shader);

                for (int i = 4; i < 8; i++)
                {
                    int x_dim = static_cast<int>(ceil(cloth.structural_constraints_[i].size() / 1024.0f));
                    context->CSSetShaderResources(0, 1, &(cloth.sc_srvs_[i]));
                    context->Dispatch(x_dim, 1, 1);
                    context->CSSetShaderResources(0, 1, &cloth.cleaner_srv_);
                }
                
                
                SetCBuffer(compliance_constant_buffer, bending_compliance_data);

                for (int i = 0; i < 4; i++)
                {
                    int x_dim = static_cast<int>(ceil(cloth.bending_constraints_[i].size() / 1024.0f));
                    context->CSSetShaderResources(0, 1, &(cloth.bending_srvs_[i]));
                    context->Dispatch(x_dim, 1, 1);
                    context->CSSetShaderResources(0, 1, &cloth.cleaner_srv_);
                }
                context->CSSetUnorderedAccessViews(0, 1, &cleaner_uav, nullptr);

                BindCShader(update_velocity_shader);
                context->CSSetConstantBuffers(0, 1, &delta_time_constant_buffer);
                context->CSSetConstantBuffers(1, 1, &gravity_constant_buffer);
                context->CSSetConstantBuffers(2, 1, &mass_constant_buffer);
                context->CSSetConstantBuffers(3, 1, &resolutiom_constant_buffer);
                resolution_data.resolution = cloth.resolution_;
                resolution_data.z_multiplier = cloth.resolution_ * cloth.resolution_ / 4;
                SetCBuffer(resolutiom_constant_buffer, resolution_data);
                context->CSSetUnorderedAccessViews(0, 1, &cloth.position_uav_, nullptr);
                context->CSSetUnorderedAccessViews(1, 1, &cloth.previous_positions_uav_, nullptr);
                context->CSSetUnorderedAccessViews(2, 1, &cloth.velocity_uav_, nullptr);
                context->CSSetUnorderedAccessViews(3, 1, &cloth.jacobi_uav_, nullptr);
                context->Dispatch(cloth.resolution_multiplier_, cloth.resolution_multiplier_, 4);
                
            }
            
            cloth.TangentUpdate(context);
            if (step_sim)
            {
                step_sim = false;
            }            
        }
        
#pragma region Shadow map

        ID3D11RenderTargetView* shadowmap_render_targets[2] = { depth_render_target_view, lightspace_position_render_target_view };
        context->ClearRenderTargetView(depth_render_target_view, white);
        context->ClearRenderTargetView(lightspace_position_render_target_view, white);
        context->OMSetRenderTargets(2, shadowmap_render_targets, depth_stencil_view);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        BindShaders(shadowmap_shader);

        context->VSSetConstantBuffers(0, 1, &view_proj_constant_buffer);
        context->VSSetConstantBuffers(1, 1, &model_matrix_constant_buffer);
        SetCBuffer(view_proj_constant_buffer, lightspace_data);
        SetCBuffer(model_matrix_constant_buffer, model_matrix_data);
        context->VSSetShaderResources(0, 1, &cloth.position_srv_);
        cloth.Draw(context);

#pragma endregion
        
#pragma region GBuffer

        ID3D11RenderTargetView* gbuffer_render_targets[3] = { position_render_target_view, normal_render_target_view, color_render_target_view };
        context->ClearRenderTargetView(position_render_target_view, bgc);
        context->ClearRenderTargetView(normal_render_target_view, bgc);
        context->ClearRenderTargetView(color_render_target_view, bgc);
        context->OMSetRenderTargets(3, gbuffer_render_targets, depth_stencil_view);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

        BindShaders(gbuffer_grid_shader);

        context->VSSetConstantBuffers(0, 1, &view_proj_constant_buffer);
        context->VSSetConstantBuffers(1, 1, &camera_constant_buffer);
        context->VSSetConstantBuffers(2, 1, &model_matrix_constant_buffer);
        context->PSSetConstantBuffers(3, 1, &grid_constant_buffer);
        context->PSSetConstantBuffers(4, 1, &bg_color_constant_buffer);
        SetCBuffer(view_proj_constant_buffer, view_proj_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(model_matrix_constant_buffer, model_matrix_data);
        SetCBuffer(grid_constant_buffer, grid_data);
        SetCBuffer(bg_color_constant_buffer, background_color_data);

        context->IASetVertexBuffers(0, 1, &rectangle_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(rectangle_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(6, 0, 0);
        
        context->Flush();
        BindShaders(gbuffer_shader);
        context->PSSetConstantBuffers(0, 1, &color_constant_buffer);
        context->VSSetConstantBuffers(1, 1, &view_proj_constant_buffer);
        context->VSSetConstantBuffers(2, 1, &camera_constant_buffer);
        context->VSSetConstantBuffers(3, 1, &model_matrix_constant_buffer);
        context->VSSetShaderResources(0, 1, &cloth.position_srv_);
        context->VSSetShaderResources(1, 1, &cloth.normal_srv_);
        SetCBuffer(color_constant_buffer, color_data);
        SetCBuffer(view_proj_constant_buffer, view_proj_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(model_matrix_constant_buffer, model_matrix_data);
        
        if (render_wireframe)
        {
            RenderWireframe();
        }
        
        cloth.Draw(context);
        RenderSolid();

#pragma endregion
        
#pragma region Light

        context->OMSetRenderTargets(1, &main_render_target_view, depth_stencil_view);
        context->ClearRenderTargetView(main_render_target_view, bgc);
        context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
        
        BindShaders(light_shader);
        context->PSSetConstantBuffers(0, 1, &view_proj_constant_buffer);
        context->PSSetConstantBuffers(1, 1, &camera_constant_buffer);
        context->PSSetConstantBuffers(2, 1, &spotlight_constant_buffer);

        context->PSSetShaderResources(0, 1, &position_shader_resource_view);
        context->PSSetShaderResources(1, 1, &normal_shader_resource_view);
        context->PSSetShaderResources(2, 1, &color_shader_resource_view);
        context->PSSetShaderResources(3, 1, &depth_shader_resource_view);
        context->PSSetShaderResources(4, 1, &lightspace_position_shader_resource_view);

        SetCBuffer(view_proj_constant_buffer, lightspace_data);
        SetCBuffer(camera_constant_buffer, camera_data);
        SetCBuffer(spotlight_constant_buffer, spotlight_data);

        context->IASetVertexBuffers(0, 1, &screen_quad_vertex_buffer, &stride, &offset);
        context->IASetIndexBuffer(screen_quad_index_buffer, DXGI_FORMAT_R32_UINT, 0);
        context->DrawIndexed(6, 0, 0);

#pragma endregion

#pragma region IMGUI

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Menu");
        
        ImGui::SliderFloat("Offset", &grid_data.offset, 0.1f, 5.0f);
        ImGui::SliderFloat("Width", &grid_data.width, 0.001f, 0.2f);

        float pos[3] = {
            DirectX::XMVectorGetByIndex(spotlight_data.direction, 0),
            DirectX::XMVectorGetByIndex(spotlight_data.direction, 1),
            DirectX::XMVectorGetByIndex(spotlight_data.direction, 2)
        };
        
        ImGui::Checkbox("Render wireframe", &render_wireframe);
        
        ImGui::SliderInt("Quality steps", &quality_steps, 1, 80);
        ImGui::SliderFloat("Structure elasticity", &structural_compliance_data.alpha, 0.00005f, 0.1f, "%.4f");
        ImGui::SliderFloat("Flexibility", &bending_compliance_data.alpha, 0.001f, 0.2f);
        ImGui::SliderFloat("Gravity strength", &gravity_data.y, -100.0f, 0.0f);
        ImGui::SliderFloat("Wind strength", &wind_data.strength_mul, 0.0f, 100.0f);

        if(ImGui::Button("Stop simulation"))
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

        float fps = ImGui::GetIO().Framerate;

        ImGui::LabelText(("FPS: " + std::to_string(fps)).c_str(), "");

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