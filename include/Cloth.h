#ifndef CLOTH_H
#define CLOTH_H

#include "Structs.h"
#include <d3d11.h>
#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>
#include "Shader.h"

class Cloth
{
private:
    std::vector<std::pair<int, int>> offsets
    {
        {0, 0}, {0, 1}, {0, 2}, {0, 3},
        {1, 0}, {1, 1}, {1, 2}, {1, 3}
    };

public:

    int resolution_multiplier_;
    int resolution_;
    int faces_per_row_;

    std::vector<float> positions_;
    std::vector<float> normals_;
    std::vector<Face> faces_;
    std::vector<std::vector<Face>> faces_gpu_groups_;
    
    ID3D11Buffer* index_buffer_;
    D3D11_SUBRESOURCE_DATA index_srd_;
    D3D11_BUFFER_DESC index_buffer_description_;
    
    ID3D11Buffer* position_buffer_ = nullptr;
    ID3D11UnorderedAccessView* position_uav_ = nullptr;
    ID3D11ShaderResourceView* position_srv_ = nullptr;
    D3D11_BUFFER_DESC position_buffer_description_;
    
    ID3D11Buffer* normal_buffer_ = nullptr;
    ID3D11UnorderedAccessView* normal_uav_ = nullptr;
    ID3D11ShaderResourceView* normal_srv_ = nullptr;
    D3D11_BUFFER_DESC normal_buffer_description_;
   
    ID3D11Buffer* previous_positions_buffer_ = nullptr;
    ID3D11UnorderedAccessView* previous_positions_uav_ = nullptr;
    D3D11_BUFFER_DESC previous_positions_buffer_description_;
   
    ID3D11Buffer* velocity_buffer_ = nullptr;
    ID3D11UnorderedAccessView* velocity_uav_ = nullptr;
    
    ID3D11Buffer* jacobi_buffer_ = nullptr;
    ID3D11UnorderedAccessView* jacobi_uav_ = nullptr;

    std::vector<ID3D11Buffer*> faces_buffers_;
    std::vector<ID3D11ShaderResourceView*> faces_srvs_;
    D3D11_BUFFER_DESC faces_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC faces_srv_description_;

    std::vector<std::vector<LinearConstraint>> structural_constraints_;
    std::vector<ID3D11Buffer*> sc_buffers_;
    std::vector<ID3D11ShaderResourceView*> sc_srvs_;
    D3D11_BUFFER_DESC sc_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC sc_srv_description_;

    std::vector<std::vector<LinearConstraint>> bending_constraints_;
    std::vector<ID3D11Buffer*> bending_buffers_;
    std::vector<ID3D11ShaderResourceView*> bending_srvs_;
    D3D11_BUFFER_DESC bending_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC bending_srv_description_;

    std::vector<PinConstraint> pin_constraints_;
    ID3D11Buffer* pc_buffer_;
    ID3D11ShaderResourceView* pc_srvs_;
    D3D11_BUFFER_DESC pc_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC pc_srv_description_;

    ID3D11UnorderedAccessView* cleaner_uav_ = nullptr;
    ID3D11ShaderResourceView* cleaner_srv_ = nullptr;

    UINT stride_;
    UINT offset_;

    CShader* zero_normals_shader_;
    CShader* recalculate_normals_shader_;

    float mass_;
    float inverse_mass_;

    Cloth(int resolution, ID3D11Device* device);
    void Init(int resolution, ID3D11Device* device);
    void CleanUp();
    void Draw(ID3D11DeviceContext* context);
    void TangentUpdate(ID3D11DeviceContext* context);
};

#endif // !CLOTH_H