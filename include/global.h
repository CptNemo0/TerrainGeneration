#ifndef GLABAL_H
#define GLABAL_H

#include <iostream>

#include <d3d11.h>
#include <wrl/client.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

using Microsoft::WRL::ComPtr;

const int WIDTH = 1600;
const int HEIGHT = 900;

static ID3D11Device* device = NULL;
static ID3D11DeviceContext* context = NULL;
static IDXGISwapChain* swap_chain = NULL;
static ID3D11RenderTargetView* main_render_target_view = NULL;

ID3D11Texture2D* depth_stencil_texture = nullptr;
ID3D11DepthStencilState* depth_stencil_state = nullptr;
ID3D11DepthStencilView* depth_stencil_view = nullptr;

D3D11_TEXTURE2D_DESC depth_stencil_texture_description;
D3D11_DEPTH_STENCIL_DESC depth_stencil_description;
D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_description;

D3D11_VIEWPORT viewport;

ID3D11RasterizerState* rasterizer_state = nullptr;
D3D11_RASTERIZER_DESC rasterizer_description;

const float triangle_verticies[]
{
    //position==========| normals=========|
      0.0f ,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      1.0f , -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};


const unsigned int triangle_indices[]
{
    0, 1, 2,
};

const float rectangle_verticies[]
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

const float screen_quad_verticies[]
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

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, NULL, &main_render_target_view);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (main_render_target_view) { main_render_target_view->Release(); main_render_target_view = NULL; }
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = WIDTH;
    sd.BufferDesc.Height = HEIGHT;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
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
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &featureLevel, &context) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (swap_chain) { swap_chain->Release(); swap_chain = NULL; }
    if (context) { context->Release(); context = NULL; }
    if (device) { device->Release(); device = NULL; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
    case WM_SIZE:
        if (device != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
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

static void InitDepthStencilBuffer()
{
    ZeroMemory(&depth_stencil_texture_description, sizeof(D3D11_TEXTURE2D_DESC));
    ZeroMemory(&depth_stencil_description, sizeof(D3D11_DEPTH_STENCIL_DESC));
    ZeroMemory(&depth_stencil_view_description, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    depth_stencil_texture_description.Width = WIDTH;
    depth_stencil_texture_description.Height = HEIGHT;
    depth_stencil_texture_description.MipLevels = 1;
    depth_stencil_texture_description.ArraySize = 1;
    depth_stencil_texture_description.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_texture_description.SampleDesc.Count = 1;
    depth_stencil_texture_description.SampleDesc.Quality = 0;
    depth_stencil_texture_description.Usage = D3D11_USAGE_DEFAULT;
    depth_stencil_texture_description.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    depth_stencil_texture_description.CPUAccessFlags = 0;
    depth_stencil_texture_description.MiscFlags = 0;

    depth_stencil_description.DepthEnable = true;
    depth_stencil_description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_description.DepthFunc = D3D11_COMPARISON_LESS;
    depth_stencil_description.StencilEnable = true;
    depth_stencil_description.StencilReadMask = 0xFF;
    depth_stencil_description.StencilWriteMask = 0xFF;
    depth_stencil_description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depth_stencil_description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depth_stencil_description.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depth_stencil_description.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_description.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    depth_stencil_view_description.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_stencil_view_description.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_description.Texture2D.MipSlice = 0;

    if (HRESULT result = device->CreateTexture2D(&depth_stencil_texture_description, nullptr, &depth_stencil_texture); result)
    {
        std::cout << "result: " << result << ". Depth stencil texture creation failed." << std::endl;
        exit(1);
    }

    if (HRESULT result = device->CreateDepthStencilState(&depth_stencil_description, &depth_stencil_state); result)
    {
        std::cout << "result: " << result << ". Depth stencil state creation failed." << std::endl;
        exit(1);
    }

    if (HRESULT result = device->CreateDepthStencilView(depth_stencil_texture, &depth_stencil_view_description, &depth_stencil_view); result)
    {
        std::cout << "result: " << result << ". Depth stencil view creation failed." << std::endl;
        exit(1);
    }
}

static void InitViewport()
{
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = WIDTH;
    viewport.Height = HEIGHT;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
}

static void InitRasterizer()
{
    ZeroMemory(&rasterizer_description, sizeof(rasterizer_description));

    rasterizer_description.FillMode = D3D11_FILL_SOLID;
    rasterizer_description.CullMode = D3D11_CULL_NONE;
    rasterizer_description.FrontCounterClockwise = false;
    rasterizer_description.DepthClipEnable = true;
    rasterizer_description.DepthBias = 0;
    rasterizer_description.DepthBiasClamp = 0.0f;
    rasterizer_description.SlopeScaledDepthBias = 0.0f;
    rasterizer_description.DepthClipEnable = false;
    rasterizer_description.ScissorEnable = false;
    rasterizer_description.MultisampleEnable = false;
    rasterizer_description.AntialiasedLineEnable = false;

    device->CreateRasterizerState(&rasterizer_description, &rasterizer_state);
}

#endif GLABAL_H