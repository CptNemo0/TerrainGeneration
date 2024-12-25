#pragma once

#include "Structs.h"
#include<DirectXMath.h>
#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>
#include "Shader.h"
#include "noise.h"

class TerrainChunk
{
public:
	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	DirectX::XMVECTOR position;// { 0.0f, 0.0f, 0.0f, 0.0f };
	int resolution;
	
	float x;
	float z;
	float size;
	float dp;

	ID3D11Buffer* vertex_buffer = nullptr;
	ID3D11Buffer* index_buffer = nullptr;
	D3D11_BUFFER_DESC buffer_description = { 0 };
	D3D11_BUFFER_DESC index_buffer_description = { 0 };
	
	TerrainChunk(float x, float z, float size, int resolution);
	
	TerrainChunk();

	void CreateVertices();

	void CreateFaces();

	DirectX::XMVECTOR GetVertex(int x, int y);

	void AccumulateNormal(int x, int z, DirectX::XMVECTOR& normal);

	void CreateNormals();

	void CreateBuffers();

	void BuildChunkAndBuffers();
	void BuildChunk();

	~TerrainChunk();
	void CleanUp();
};