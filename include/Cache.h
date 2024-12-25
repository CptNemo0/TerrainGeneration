#pragma once

#include "QuadTree.h"
#include "TerrainChunk.h"
#include <unordered_map>
#include <unordered_set>
struct CacheNode
{
	CacheNode* previous;
	CacheNode* next;

	std::size_t key;
	TerrainChunk value;

	CacheNode(const std::size_t key);
	CacheNode(const std::size_t key, TerrainChunk value);
	CacheNode(const std::size_t key, TerrainChunk& value);
};

class Cache
{
public:
	CacheNode* left = nullptr;
	CacheNode* right = nullptr;

	std::unordered_map<std::size_t, CacheNode*> map;
	std::unordered_set<std::size_t> staged;

	int capacity = 0;

	Cache() = default;
	Cache(int capacity);

	TerrainChunk* get(std::size_t key);

	void remove(CacheNode* node);
	void insert(CacheNode* node);
	void put(CacheNode* node);
};

