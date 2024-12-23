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

void TerrainChunk::CreateVertices()
{
    
    float half_size = size * 0.5f;
    float start_x = x - half_size;
    float start_z = z - half_size;
    vertices.resize(resolution * resolution);

    SimplexNoise noise{};
    for (int i = 0; i < resolution; i++)
    {
        for (int j = 0; j < resolution; j++)
        {
            int idx = i * resolution + j;
            float local_x = start_x + dp * i;
            float local_z = start_z + dp * j;
            float local_y = noise.signedFBM(local_x, local_z, 5, 1.5f, 10.0f, 0.0001);
            local_y *= 100.0f;
            vertices[idx] = { local_x, local_y, local_z, 0.0f, 0.0f, 0.0f };
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

void TerrainChunk::CreateNormals()
{
    for (const auto& [a, b, c] : faces)
    {
        DirectX::XMVECTOR va = DirectX::XMVectorSet(vertices[a].x, vertices[a].y, vertices[a].z, 0.0f);
        DirectX::XMVECTOR vb = DirectX::XMVectorSet(vertices[b].x, vertices[b].y, vertices[b].z, 0.0f);
        DirectX::XMVECTOR vc = DirectX::XMVectorSet(vertices[c].x, vertices[c].y, vertices[c].z, 0.0f);

        auto diff1 = DirectX::XMVectorSubtract(vb, va);
        auto diff2 = DirectX::XMVectorSubtract(vb, vc);
        auto normal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(diff1, diff2));

        auto x = DirectX::XMVectorGetX(normal);
        auto y = DirectX::XMVectorGetY(normal);
        auto z = DirectX::XMVectorGetZ(normal);

        vertices[a].nx += x;
        vertices[b].nx += x;
        vertices[c].nx += x;

        vertices[a].ny += y;
        vertices[b].ny += y;
        vertices[c].ny += y;

        vertices[a].nz += z;
        vertices[b].nz += z;
        vertices[c].nz += z;
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

void TerrainChunk::BuildChunk()
{
    CreateVertices();
    CreateFaces();
    CreateNormals();
    CreateBuffers();
}
