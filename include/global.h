#ifndef GLABAL_H
#define GLABAL_H
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace
{
    static ID3D11Device* device = NULL;
    static ID3D11DeviceContext* context = NULL;
    static IDXGISwapChain* swap_chain = NULL;
    static ID3D11RenderTargetView* main_render_target_view = NULL;
}

#endif GLABAL_H