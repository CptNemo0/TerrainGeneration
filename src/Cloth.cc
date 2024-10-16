#include "../include/Cloth.h"

Cloth::Cloth(int resolution, ID3D11Device* device)
{
	resolution_multiplier_ = resolution;
	resolution_ = 32 * resolution_multiplier_;

	ZeroMemory(&vertex_buffer_description_, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&index_buffer_description_, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&output_buffer_description_, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&output_uav_description_, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	ZeroMemory(&output_srv_description_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	ZeroMemory(&faces_buffer_description_, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&faces_srv_description_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    float offset = 2.0f / (static_cast<float>(resolution_) - 1.0f);

    float start_x = -1.0f;
    float start_y = 1.0f;

    for (int i = 0; i < resolution_; i++)
    {
        for (int j = 0; j < resolution_; j++)
        {
            vertices_.emplace_back(start_x + j * offset,
                                   start_y - i * offset,
                                   0.0f,
                                   0.0f,
                                   0.0f,
                                   1.0f);
        }
    }

    bool up = true;

    for (int i = 0; i < resolution_ - 1; i++)
    {
        for (int j = 0; j < resolution_ - 1; j++)
        {
            int idx = resolution_ * i + j;

            if (up)
            {
                up = false;
                faces_.emplace_back(idx + resolution_, idx, idx + 1);
                faces_.emplace_back(idx + 1 + resolution_, idx + resolution_, idx + 1);
            }
            else
            {
                up = true;
                faces_.emplace_back(idx + resolution_ + 1, idx + resolution_, idx);
                faces_.emplace_back(idx + resolution_ + 1, idx, idx + 1);
            }
        }
    }

    faces_per_row_ = (resolution_ - 1) * 2;

    for (int h = 0; h < offsets.size(); h++)
    {
        faces_gpu_groups_.push_back({});

        for (int i = offsets[h].first; i < resolution_ - 1; i += 2)
        {
            for (int j = offsets[h].second; j < faces_per_row_; j += 4)
            {
                int idx = i * faces_per_row_ + j;
                faces_gpu_groups_[h].push_back(faces_[idx]);
            }
        }
    }

    vertex_buffer_description_.ByteWidth = sizeof(Vertex) * vertices_.size();
    vertex_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
    vertex_buffer_description_.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_description_.CPUAccessFlags = 0;
    vertex_buffer_description_.MiscFlags = 0;
    vertex_buffer_description_.StructureByteStride = sizeof(Vertex);

    vertex_srd_.pSysMem = &(vertices_[0]);
    vertex_srd_.SysMemPitch = 0;
    vertex_srd_.SysMemSlicePitch = 0;

    if (static_cast<int>(device->CreateBuffer(&vertex_buffer_description_, &vertex_srd_, &vertex_buffer_)))
    {
        std::cout << "Vertex buffer creation failed\n";
        exit(1);
    }
    
    index_buffer_description_.ByteWidth = sizeof(Face) * faces_.size();
    index_buffer_description_.BindFlags = D3D11_BIND_INDEX_BUFFER;
    
    index_srd_.pSysMem = &(faces_[0]);
    index_srd_.SysMemPitch = 0;
    index_srd_.SysMemSlicePitch = 0;

    if (static_cast<int>(device->CreateBuffer(&index_buffer_description_, &index_srd_, &index_buffer_)))
    {
        std::cout << "Index buffer creation failed\n";
        exit(1);
    }

    output_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
    output_buffer_description_.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    output_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    output_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    output_buffer_description_.StructureByteStride = sizeof(Vertex);
    output_buffer_description_.ByteWidth = sizeof(Vertex) * resolution_ * resolution_;

    output_srd_.pSysMem = &(vertices_[0]);
    output_srd_.SysMemPitch = 0;
    output_srd_.SysMemSlicePitch = 0;

    output_uav_description_.Format = DXGI_FORMAT_UNKNOWN;
    output_uav_description_.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    output_uav_description_.Buffer.FirstElement = 0;
    output_uav_description_.Buffer.NumElements = resolution_ * resolution_;
    output_srv_description_.Format = DXGI_FORMAT_UNKNOWN;
    output_srv_description_.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    output_srv_description_.Buffer.FirstElement = 0;
    output_srv_description_.Buffer.NumElements = resolution_ * resolution_;

    if (static_cast<int>(device->CreateBuffer(&output_buffer_description_, &output_srd_, &output_buffer_)))
    {
        std::cout << "Buffer creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateUnorderedAccessView(output_buffer_, &output_uav_description_, &output_uav_)))
    {
        std::cout << " UAV creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateShaderResourceView(output_buffer_, &output_srv_description_, &output_srv_)))
    {
        std::cout << "SRV creation failed\n";
        exit(1);
    }

    for (int i = 0; i < 8; i++)
    {
        faces_buffers_.push_back(nullptr);
        faces_srvs_.push_back(nullptr);

        faces_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
        faces_buffer_description_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        faces_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        faces_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        faces_buffer_description_.StructureByteStride = sizeof(Face);
        faces_buffer_description_.ByteWidth = sizeof(Face) * faces_gpu_groups_[i].size();

        faces_srd_.pSysMem = &(faces_gpu_groups_[i][0]);
        faces_srd_.SysMemPitch = 0;
        faces_srd_.SysMemSlicePitch = 0;

        faces_srv_description_.Format = DXGI_FORMAT_UNKNOWN;
        faces_srv_description_.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        faces_srv_description_.Buffer.FirstElement = 0;
        faces_srv_description_.Buffer.NumElements = faces_gpu_groups_[i].size();

        if (static_cast<int>(device->CreateBuffer(&faces_buffer_description_, &faces_srd_, &faces_buffers_[i])))
        {
            std::cout << "Buffer creation failed\n";
            exit(1);
        }

        if (static_cast<int>(device->CreateShaderResourceView(faces_buffers_[i], &faces_srv_description_, &faces_srvs_[i])))
        {
            std::cout << "SRV creation failed\n";
            exit(1);
        }
    }

    mass_ = 1.0f;
    inverse_mass_ = 1.0f;
}

void Cloth::Update(float dt)
{
}

void Cloth::Draw(ID3D11DeviceContext* context)
{
    context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride_, &offset_);
    context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(3 * faces_.size(), 0, 0);
    context->VSSetShaderResources(0, 1, &cleaner_srv_);
}

void Cloth::TangentUpdate(ID3D11DeviceContext* context)
{
    context->CSSetShader(zero_normals_shader_->compute_shader, nullptr, 0);
    context->CSSetUnorderedAccessViews(0, 1, &output_uav_, nullptr);
    context->Dispatch(resolution_multiplier_ * resolution_multiplier_, 1, 1);
    context->CSSetUnorderedAccessViews(0, 1, &cleaner_uav_, nullptr);

    context->CSSetShader(recalculate_normals_shader_->compute_shader, nullptr, 0);
    context->CSSetUnorderedAccessViews(0, 1, &output_uav_, nullptr);
    for (int i = 0; i < 8; i++)
    {
        context->CSSetShaderResources(0, 1, &(faces_srvs_[i]));
        context->Dispatch(resolution_multiplier_ * resolution_multiplier_, 1, 1);
        context->CSSetShaderResources(0, 1, &cleaner_srv_);
    }
    context->CSSetUnorderedAccessViews(0, 1, &cleaner_uav_, nullptr);
}
