#include "../include/Cloth.h"

Cloth::Cloth(int resolution, ID3D11Device* device)
{
	resolution_multiplier_ = resolution;
	resolution_ = 32 * resolution_multiplier_;
   
	ZeroMemory(&index_buffer_description_, sizeof(D3D11_BUFFER_DESC));

    ZeroMemory(&position_buffer_description_, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&previous_positions_buffer_description_, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&faces_buffer_description_, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&faces_srv_description_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    ZeroMemory(&sc_buffer_description_, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&sc_srv_description_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    ZeroMemory(&bending_buffer_description_, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&bending_srv_description_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    ZeroMemory(&pc_buffer_description_, sizeof(D3D11_BUFFER_DESC));
    ZeroMemory(&pc_srv_description_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    float offset = 2.0f / (static_cast<float>(resolution_) - 1.0f);

    float start_x = -1.0f;
    float start_y = 1.0f;

    for (int i = 0; i < resolution_; i++)
    {
        for (int j = 0; j < resolution_; j++)
        {
            positions_.push_back(start_x + j * offset);
            positions_.push_back(start_y - i * offset);
            positions_.push_back(0.1f);

            normals_.push_back(0.0f);
            normals_.push_back(0.0f);
            normals_.push_back(1.0f);
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

    
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_description;
    ZeroMemory(&srv_description, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    srv_description.Format = DXGI_FORMAT_UNKNOWN;
    srv_description.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srv_description.Buffer.FirstElement = 0;
    srv_description.Buffer.NumElements = resolution_ * resolution_;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uav_description;
    ZeroMemory(&uav_description, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
    uav_description.Format = DXGI_FORMAT_UNKNOWN;
    uav_description.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uav_description.Buffer.FirstElement = 0;
    uav_description.Buffer.NumElements = resolution_ * resolution_;

    D3D11_SUBRESOURCE_DATA srd;
    ZeroMemory(&srd, sizeof(D3D11_SUBRESOURCE_DATA));

    position_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
    position_buffer_description_.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    position_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    position_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    position_buffer_description_.StructureByteStride = sizeof(float) * 3;
    position_buffer_description_.ByteWidth = sizeof(float) * 3 * resolution_ * resolution_;
    srd.pSysMem = &(positions_[0]);
    
    if (static_cast<int>(device->CreateBuffer(&position_buffer_description_, &srd, &position_buffer_)))
    {
        std::cout << "Buffer creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateUnorderedAccessView(position_buffer_, &uav_description, &position_uav_)))
    {
        std::cout << " UAV creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateShaderResourceView(position_buffer_, &srv_description, &position_srv_)))
    {
        std::cout << " SRV creation failed\n";
        exit(1);
    }

    normal_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
    normal_buffer_description_.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    normal_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    normal_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    normal_buffer_description_.StructureByteStride = sizeof(float) * 3;
    normal_buffer_description_.ByteWidth = sizeof(float) * 3 * resolution_ * resolution_;
    srd.pSysMem = &(normals_[0]);

    if (static_cast<int>(device->CreateBuffer(&normal_buffer_description_, &srd, &normal_buffer_)))
    {
        std::cout << "Buffer creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateUnorderedAccessView(normal_buffer_, &uav_description, &normal_uav_)))
    {
        std::cout << " UAV creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateShaderResourceView(normal_buffer_, &srv_description, &normal_srv_)))
    {
        std::cout << " SRV creation failed\n";
        exit(1);
    }

    previous_positions_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
    previous_positions_buffer_description_.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    previous_positions_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    previous_positions_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    previous_positions_buffer_description_.StructureByteStride = sizeof(float) * 3;
    previous_positions_buffer_description_.ByteWidth = sizeof(float) * 3 * resolution_ * resolution_;
    srd.pSysMem = &(positions_[0]);

    if (static_cast<int>(device->CreateBuffer(&previous_positions_buffer_description_, &srd, &previous_positions_)))
    {
        std::cout << "Buffer creation failed\n";
        exit(1);
    }
    if (static_cast<int>(device->CreateUnorderedAccessView(previous_positions_, &uav_description, &previous_positions_uav_)))
    {
        std::cout << " UAV creation failed\n";
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

        srd.pSysMem = &(faces_gpu_groups_[i][0]);
        
        faces_srv_description_.Format = DXGI_FORMAT_UNKNOWN;
        faces_srv_description_.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        faces_srv_description_.Buffer.FirstElement = 0;
        faces_srv_description_.Buffer.NumElements = faces_gpu_groups_[i].size();

        if (static_cast<int>(device->CreateBuffer(&faces_buffer_description_, &srd, &faces_buffers_[i])))
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


    float edge_len = 2.0f / static_cast<float>(resolution_ -1 );
    float d_len = edge_len * sqrtf(2.0f);

    for (int i = 0; i < 8 ; i++)
    {
        structural_constraints_.push_back({});
    }
    
    //RED
    for (int i = 0; i < resolution_; i++)
    {
        for (int j = 0; j < resolution_ - 1; j+=2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + 1;
            c.distance = edge_len;
            structural_constraints_[0].push_back(c);
        }
    }

    //YELLOW
    for (int i = 0; i < resolution_; i++)
    {
        for (int j = 1; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + 1;
            c.distance = edge_len;
            structural_constraints_[1].push_back(c);
        }
    }

    //GREEN
    for (int i = 0; i < resolution_ - 1; i+=2)
    {
        for (int j = 0; j < resolution_; j++)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + resolution_;
            c.distance = edge_len;
            structural_constraints_[2].push_back(c);
        }
    }

    //PALE GREEN
    for (int i = 1; i < resolution_ - 1; i += 2)
    {
        for (int j = 0; j < resolution_; j++)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + resolution_;
            c.distance = edge_len;
            structural_constraints_[3].push_back(c);
        }
    }

    //DEEP BLUE
    for (int i = 1; i < resolution_; i += 2)
    {
        for (int j = 0; j < resolution_; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx - resolution_ + 1;
            c.distance = d_len;
            structural_constraints_[4].push_back(c);
        }
    }

    //BLUE
    for (int i = 0; i < resolution_ - 1; i += 2)
    {
        for (int j = 1; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + resolution_ + 1;
            c.distance = d_len;
            structural_constraints_[5].push_back(c);
        }
    }

    //PALE BLUE
    for (int i = 1; i < resolution_ - 1; i += 2)
    {
        for (int j = 0; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + resolution_ + 1;
            c.distance = d_len;
            structural_constraints_[6].push_back(c);
        }
    }

    //PURPLE
    for (int i = 2; i < resolution_ - 1; i += 2)
    {
        for (int j = 2; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx - resolution_ + 1;
            c.distance = d_len;
            structural_constraints_[7].push_back(c);
        }
    }

    for (int i = 0; i < 8; i++)
    {
        sc_buffers_.push_back(nullptr);
        sc_srvs_.push_back(nullptr);

        sc_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
        sc_buffer_description_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        sc_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        sc_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        sc_buffer_description_.StructureByteStride = sizeof(LinearConstraint);
        sc_buffer_description_.ByteWidth = sizeof(LinearConstraint) * structural_constraints_[i].size();

        srd.pSysMem = &(structural_constraints_[i][0]);

        sc_srv_description_.Format = DXGI_FORMAT_UNKNOWN;
        sc_srv_description_.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        sc_srv_description_.Buffer.FirstElement = 0;
        sc_srv_description_.Buffer.NumElements = structural_constraints_[i].size();

        if (static_cast<int>(device->CreateBuffer(&sc_buffer_description_, &srd, &sc_buffers_[i])))
        {
            std::cout << "Buffer creation failed\n";
            exit(1);
        }

        if (static_cast<int>(device->CreateShaderResourceView(sc_buffers_[i], &sc_srv_description_, &sc_srvs_[i])))
        {
            std::cout << "SRV creation failed\n";
            exit(1);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        bending_constraints_.push_back({});
    }

    for (int i = 0; i < resolution_; i += 2)
    {
        for (int j = 0; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + 1 + resolution_;
            c.distance = d_len;
            bending_constraints_[0].push_back(c);
        }
    }

    for (int i = 1; i < resolution_; i += 2)
    {
        for (int j = 1; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + 1 - resolution_;
            c.distance = d_len;
            bending_constraints_[1].push_back(c);
        }
    }

    for (int i = 2; i < resolution_; i += 2)
    {
        for (int j = 0; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + 1 - resolution_;
            c.distance = d_len;
            bending_constraints_[2].push_back(c);
        }
    }
    
    for (int i = 1; i < resolution_ - 1; i += 2)
    {
        for (int j = 1; j < resolution_ - 1; j += 2)
        {
            int idx = i * resolution_ + j;
            LinearConstraint c;
            c.idx_a = idx;
            c.idx_b = idx + 1 + resolution_;
            c.distance = d_len;
            bending_constraints_[3].push_back(c);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        bending_buffers_.push_back(nullptr);
        bending_srvs_.push_back(nullptr);

        bending_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
        bending_buffer_description_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bending_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bending_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bending_buffer_description_.StructureByteStride = sizeof(LinearConstraint);
        bending_buffer_description_.ByteWidth = sizeof(LinearConstraint) * bending_constraints_[i].size();

        srd.pSysMem = &(bending_constraints_[i][0]);

        bending_srv_description_.Format = DXGI_FORMAT_UNKNOWN;
        bending_srv_description_.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        bending_srv_description_.Buffer.FirstElement = 0;
        bending_srv_description_.Buffer.NumElements = bending_constraints_[i].size();

        if (static_cast<int>(device->CreateBuffer(&bending_buffer_description_, &srd, &bending_buffers_[i])))
        {
            std::cout << "Buffer creation failed\n";
            exit(1);
        }

        if (static_cast<int>(device->CreateShaderResourceView(bending_buffers_[i], &bending_srv_description_, &bending_srvs_[i])))
        {
            std::cout << "SRV creation failed\n";
            exit(1);
        }
    }

    PinConstraint pin1;
    pin1.idx = 0;
    pin1.x = positions_[0];
    pin1.y = positions_[1];
    pin1.z = positions_[2];

    PinConstraint pin2;
    pin2.idx = resolution_ - 1;
    pin2.x = positions_[(resolution_ - 1) * 3];
    pin2.y = positions_[(resolution_ - 1) * 3 + 1];
    pin2.z = positions_[(resolution_ - 1) * 3 + 2];
    
    pin_constraints_.push_back(pin1);
    pin_constraints_.push_back(pin2);

    pc_buffer_description_.Usage = D3D11_USAGE_DEFAULT;
    pc_buffer_description_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    pc_buffer_description_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pc_buffer_description_.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    pc_buffer_description_.StructureByteStride = sizeof(PinConstraint);
    pc_buffer_description_.ByteWidth = sizeof(PinConstraint) * pin_constraints_.size();

    srd.pSysMem = &(pin_constraints_[0]);

    pc_srv_description_.Format = DXGI_FORMAT_UNKNOWN;
    pc_srv_description_.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    pc_srv_description_.Buffer.FirstElement = 0;
    pc_srv_description_.Buffer.NumElements = pin_constraints_.size();

    if (static_cast<int>(device->CreateBuffer(&pc_buffer_description_, &srd, &pc_buffer_)))
    {
        std::cout << "Buffer creation failed\n";
        exit(1);
    }

    if (static_cast<int>(device->CreateShaderResourceView(pc_buffer_, &pc_srv_description_, &pc_srvs_)))
    {
        std::cout << "SRV creation failed\n";
        exit(1);
    }

    mass_ = 1.0f;
    inverse_mass_ = 1.0f;
}

void Cloth::Update(float dt)
{
}

void Cloth::Draw(ID3D11DeviceContext* context)
{
    context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(3 * faces_.size(), 0, 0);
    context->VSSetShaderResources(0, 1, &cleaner_srv_);
}

void Cloth::TangentUpdate(ID3D11DeviceContext* context)
{
    context->CSSetShader(zero_normals_shader_->compute_shader, nullptr, 0);
    context->CSSetUnorderedAccessViews(0, 1, &normal_uav_, nullptr);
    context->Dispatch(resolution_multiplier_ * resolution_multiplier_, 1, 1);
    context->CSSetUnorderedAccessViews(0, 1, &cleaner_uav_, nullptr);

    context->CSSetShader(recalculate_normals_shader_->compute_shader, nullptr, 0);
    context->CSSetUnorderedAccessViews(0, 1, &position_uav_, nullptr);
    context->CSSetUnorderedAccessViews(1, 1, &normal_uav_, nullptr);
    for (int i = 0; i < 8; i++)
    {
        context->CSSetShaderResources(0, 1, &(faces_srvs_[i]));
        context->Dispatch(resolution_multiplier_ * resolution_multiplier_, 1, 1);
        context->CSSetShaderResources(0, 1, &cleaner_srv_);
    }
    context->CSSetUnorderedAccessViews(0, 1, &cleaner_uav_, nullptr);
    context->CSSetUnorderedAccessViews(1, 1, &cleaner_uav_, nullptr);
}
