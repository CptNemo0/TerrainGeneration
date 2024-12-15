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
	float dp;

	ID3D11Buffer* vertex_buffer;
	ID3D11Buffer* index_buffer;
	D3D11_BUFFER_DESC buffer_description = { 0 };
	D3D11_BUFFER_DESC index_buffer_description = { 0 };
	
	TerrainChunk(float x, float z, int resolution);

	void CreateVertices();

	void CreateFaces();

	void CreateNormals();

	void CreateBuffers();

	void BuildChunk();
};