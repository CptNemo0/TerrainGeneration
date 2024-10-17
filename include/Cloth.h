#ifndef CLOTH_H
#define CLOTH_H

#include "Structs.h"
#include <d3d11.h>
#include <vector>
#include <iostream>
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

    std::vector<Vertex> vertices_;
    std::vector<Face> faces_;
    std::vector<std::vector<Face>> faces_gpu_groups_;
    
    ID3D11Buffer* vertex_buffer_ = nullptr;
    D3D11_SUBRESOURCE_DATA vertex_srd_;
    D3D11_BUFFER_DESC vertex_buffer_description_;

    ID3D11Buffer* index_buffer_;
    D3D11_SUBRESOURCE_DATA index_srd_;
    D3D11_BUFFER_DESC index_buffer_description_;
    
    ID3D11Buffer* output_buffer_ = nullptr;
    ID3D11UnorderedAccessView* output_uav_ = nullptr;
    ID3D11ShaderResourceView* output_srv_ = nullptr;
    D3D11_SUBRESOURCE_DATA output_srd_;
    D3D11_BUFFER_DESC output_buffer_description_;
    D3D11_UNORDERED_ACCESS_VIEW_DESC output_uav_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC output_srv_description_;

    ID3D11Buffer* previous_positions_ = nullptr;
    ID3D11UnorderedAccessView* previous_positions_uav_ = nullptr;
    D3D11_SUBRESOURCE_DATA previous_positions_srd_;
    D3D11_BUFFER_DESC previous_positions_buffer_description_;
    D3D11_UNORDERED_ACCESS_VIEW_DESC previous_positions_uav_description_;

    ID3D11Buffer* velocity_buffer_ = nullptr;
    ID3D11UnorderedAccessView* velocity_uav_ = nullptr;
    D3D11_SUBRESOURCE_DATA velocity_srd_;
    D3D11_BUFFER_DESC velocity_buffer_description_;
    D3D11_UNORDERED_ACCESS_VIEW_DESC velocity_uav_description_;

    std::vector<ID3D11Buffer*> faces_buffers_;
    std::vector<ID3D11ShaderResourceView*> faces_srvs_;
    D3D11_SUBRESOURCE_DATA faces_srd_;
    D3D11_BUFFER_DESC faces_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC faces_srv_description_;

    std::vector<std::vector<LinearConstraint>> structural_constraints_;
    std::vector<ID3D11Buffer*> sc_buffers_;
    std::vector<ID3D11ShaderResourceView*> sc_srvs_;
    D3D11_SUBRESOURCE_DATA sc_srd_;
    D3D11_BUFFER_DESC sc_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC sc_srv_description_;

    std::vector<std::vector<LinearConstraint>> bending_constraints_;
    std::vector<ID3D11Buffer*> bending_buffers_;
    std::vector<ID3D11ShaderResourceView*> bending_srvs_;
    D3D11_SUBRESOURCE_DATA bending_srd_;
    D3D11_BUFFER_DESC bending_buffer_description_;
    D3D11_SHADER_RESOURCE_VIEW_DESC bending_srv_description_;

    std::vector<PinConstraint> pin_constraints_;
    ID3D11Buffer* pc_buffer_;
    ID3D11ShaderResourceView* pc_srvs_;
    D3D11_SUBRESOURCE_DATA pc_srd_;
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
    void Update(float dt);
    void Draw(ID3D11DeviceContext* context);
    void TangentUpdate(ID3D11DeviceContext* context);
};

#endif // !CLOTH_H