#include "../include/TerrainChunk.h"
#include "../include/App.h"

TerrainChunk::TerrainChunk(float x, float z, float size, int resolution)
{
    this->x = x;
    this->z = z;
    this->size = size;
	this->position = DirectX::XMVectorSet(x, 0.0f, z, 0.0f);
	this->resolution = resolution; 
    this->dp = size / resolution;
    this->resolution+=1;
}

TerrainChunk::TerrainChunk()
{
    this->x = 0;
    this->z = 0;
    this->size = (10);
    this->resolution = (16 );
    this->dp = (size) / resolution;
    resolution += 1;
}

float get_y(float local_x, float local_z)
{
    SimplexNoise noise{};
    float local_y = noise.signedFBM(local_x, local_z, 5, 0.02f, 1.0f, 0.00006f, 1.0);
    local_y *= 100.0f;
    local_y = pow(local_y, 3);
    local_y = max(local_y, 0.0f);
    return local_y;
}

void TerrainChunk::CreateVertices()
{
    
    float half_size = size * 0.5f;
    float start_x = x - half_size;
    float start_z = z - half_size;
    vertices.resize(resolution * resolution);

    
    for (int i = 0; i < resolution; i++)
    {
        for (int j = 0; j < resolution; j++)
        {
            int idx = i * resolution + j;
            float local_x = start_x + dp * i;
            float local_z = start_z + dp * j;
            
            vertices[idx] = { local_x, get_y(local_x, local_z), local_z, 0.0f, 0.0f, 0.0f};
        }
    }
}

void TerrainChunk::CreateFaces()
{
    bool up = true;
    for (int i = 0; i < resolution - 1; i++)
    {
        for (int j = 0; j < resolution - 1; j++)
        {
            int idx = resolution * i + j;

            if (up)
            {
                up = false;
                faces.emplace_back(idx + resolution, idx, idx + 1);
                faces.emplace_back(idx + 1 + resolution, idx + resolution, idx + 1);
            }
            else
            {
                up = true;
                faces.emplace_back(idx + resolution + 1, idx + resolution, idx);
                faces.emplace_back(idx + resolution + 1, idx, idx + 1);
            }
        }
    }
}

DirectX::XMVECTOR TerrainChunk::GetVertex(int x, int z)
{
    if (x > -1 && x < resolution && z > -1 && z < resolution)
    {
        int idx = resolution * x + z;
        return DirectX::XMVectorSet(vertices[idx].x, vertices[idx].y, vertices[idx].z, 0.0f);
    }
    else
    {
        SimplexNoise noise{};
        float half_size = size * 0.5f;
        float start_x = this->x - half_size;
        float start_z = this->z - half_size;
        float local_x = start_x + dp * x;
        float local_z = start_z + dp * z;
        return DirectX::XMVectorSet(local_x, get_y(local_x, local_z), local_z, 0.0f);
    }
}

void TerrainChunk::AccumulateNormal(int x, int z, DirectX::XMVECTOR& normal)
{
    if (x > -1 && x < resolution && z > -1 && z < resolution)
    {
        int idx = resolution * x + z;
        vertices[idx].nx += DirectX::XMVectorGetX(normal);
        vertices[idx].ny += DirectX::XMVectorGetY(normal);
        vertices[idx].nz += DirectX::XMVectorGetZ(normal);
    }
}

void TerrainChunk::CreateNormals()
{
    std::vector<std::pair<int, int>> offsets
    {
        {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, 1}, {-1, -1}, {-1, 0}
    };

    auto per_vertex = [this, &offsets](int vertex_i)
    {
        int az = vertex_i > 0 ? (vertex_i % resolution) : 0;
        int ax = (vertex_i - az) / resolution;
        DirectX::XMVECTOR va = GetVertex(ax, az);

        for (int i = 0; i < offsets.size(); i++)
        {
            int ii = (i + 1) % offsets.size();

            auto [bx, bz] = offsets[i];
            auto [cx, cz] = offsets[ii];

            bx += ax;
            bz += az;
            cx += ax;
            cz += az;
            
            DirectX::XMVECTOR vb = GetVertex(bx, bz);
            DirectX::XMVECTOR vc = GetVertex(cx, cz);

            auto diff1 = DirectX::XMVectorSubtract(va, vb);
            auto diff2 = DirectX::XMVectorSubtract(va, vc);
            auto normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(diff1, diff2));

            AccumulateNormal(ax, az, normal);
            AccumulateNormal(bx, bz, normal);
            AccumulateNormal(cx, cz, normal);
        }
    };

    for (int i = 0; i < vertices.size(); i++)
    {
        per_vertex(i);
    }
    
    for (auto& vertex : vertices)
    {
        DirectX::XMVECTOR normal = DirectX::XMVectorSet(vertex.nx, vertex.ny, vertex.nz, 0.0f);
        normal = DirectX::XMVector3Normalize(normal);
        vertex.nx = DirectX::XMVectorGetX(normal);
        vertex.ny = DirectX::XMVectorGetY(normal);
        vertex.nz = DirectX::XMVectorGetZ(normal);
    }
}

void TerrainChunk::CreateBuffers()
{
    if (!App::device_ ) return;
    if (!vertices.size()) return;
    if (!faces.size()) return;
    buffer_description.ByteWidth = sizeof(Vertex) * vertices.size();
    buffer_description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_description.StructureByteStride = sizeof(float) * 6;
    D3D11_SUBRESOURCE_DATA screen_quad_vertex_srd = { &(vertices[0]), 0, 0 };
    

    index_buffer_description.ByteWidth = sizeof(Face) * faces.size();
    index_buffer_description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA screen_quad_index_srd = { &(faces[0]), 0, 0 };

    App::device_->CreateBuffer(&buffer_description, &screen_quad_vertex_srd, &vertex_buffer);
    App::device_->CreateBuffer(&index_buffer_description, &screen_quad_index_srd, &index_buffer);
}

void TerrainChunk::BuildChunkAndBuffers()
{
    CreateVertices();
    CreateFaces();
    CreateNormals();
    CreateBuffers();
}

void TerrainChunk::BuildChunk()
{
    CreateVertices();
    CreateFaces();
    CreateNormals();
}