#ifndef APP_H
#define APP_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>

#include <algorithm>
#include <d3d11.h>
#include <DirectXMath.h>

#include <tchar.h>
#include <wrl/client.h>
#include <WinBase.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "Shader.h"
#include "ConstantBufferStructs.h"
#include "Cloth.h"
#include "TerrainChunk.h"
#include "FPCamera.h"
#include "QuadTree.h"
#include "TerrainBuilder.h"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

class App
{
public:

	const float rectangle_vertices[24]
	{
		//position============= | normals=========|
		-150.0f, -2.5f,  150.0f, 0.0f, 1.0f, 0.0f,
		 150.0f, -2.5f,  150.0f, 0.0f, 1.0f, 0.0f,
		 150.0f, -2.5f, -150.0f, 0.0f, 1.0f, 0.0f,
		-150.0f, -2.5f, -150.0f, 0.0f, 1.0f, 0.0f,
	};

	const unsigned int rectangle_indices[6]
	{
		0, 1, 3,
		3, 1, 2
	};

	const float screen_quad_vertices[24]
	{
		//position========| normals=========|
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	const unsigned int screen_quad_indices[6]
	{
		0, 1, 3,
		3, 1, 2
	};

	const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	WNDCLASSEX wc_;
	HWND hwnd_;

	static ID3D11Device* device_;
	static ID3D11DeviceContext* context_;
	static IDXGISwapChain* swap_chain_;
	static ID3D11RenderTargetView* main_render_target_view_;
	
	ID3D11Texture2D* depth_stencil_texture_ = nullptr;
	ID3D11DepthStencilState* depth_stencil_state_ = nullptr;
	ID3D11DepthStencilView* depth_stencil_view_ = nullptr;

	D3D11_TEXTURE2D_DESC depth_stencil_texture_description_;
	D3D11_DEPTH_STENCIL_DESC depth_stencil_description_;
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_description_;

	D3D11_VIEWPORT viewport_;

	ID3D11RasterizerState* rasterizer_state_ = nullptr;
	D3D11_RASTERIZER_DESC rasterizer_description_;

	int window_width_;
	int window_height_;
	LPCSTR window_name_;

	std::mutex device_mutex;

	App(int width, int height, LPCSTR name);

	static void CreateRenderTarget();

	static void CleanupRenderTarget();

	static bool CreateDeviceD3D(HWND hWnd);

	static void CleanupDeviceD3D();

	void InitViewport();

	void InitRasterizer();

	void InitDepthStencilBuffer();

	void CreateShaders(Shader& shader, LPCWSTR path);

	void CreateCShader(CShader& shader, LPCWSTR path);

	void BindShaders(Shader& shader);

	void BindCShader(CShader& shader);

	void Init();

	void Run();

	void End();

	//void CreateLand(QuadTreeNode* node, std::unordered_map<QuadTreeNode, TerrainChunk, QuadTreeNodeHash, QuadTreeNodeEqual>* map, std::vector<TerrainChunk>* chunks);

	template<typename T>
	void SetCBuffer(ID3D11Buffer* buffer, T& data);
	template<typename T>
	void CreateCBuffer(ID3D11Buffer** buffer, T& data);

	void RenderSolid()
	{
	    rasterizer_description_.FillMode = D3D11_FILL_SOLID;
	    device_->CreateRasterizerState(&rasterizer_description_, &rasterizer_state_);
	    context_->RSSetState(rasterizer_state_);
	}
	
	void RenderWireframe()
	{
		rasterizer_description_.FillMode = D3D11_FILL_WIREFRAME;
		device_->CreateRasterizerState(&rasterizer_description_, &rasterizer_state_);
		context_->RSSetState(rasterizer_state_);
	}
};


#endif // !APP_H
