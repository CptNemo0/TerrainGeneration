#pragma once

#include "QuadTree.h"
#include "TerrainChunk.h"
#include "Cache.h"
#include <queue>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
class TerrainBuilder
{
public:
	std::priority_queue<QuadTreeNode*, std::vector<QuadTreeNode*>, QuadTreeNodePtrDepthCopr> construction_queue;
	std::unordered_set<std::size_t> enqued;
	std::unordered_map<std::size_t, TerrainChunk> built;

	std::mutex que_mutex;
	std::mutex cache_mutex;
	std::mutex* device_mutex = nullptr;

	std::atomic<bool> finished = false;

	std::condition_variable queue_cv;

	std::vector<std::thread> workers;
	int num_workers;

	Cache cache;

	TerrainBuilder(std::mutex* device_mutex_ptr, int num_workers, int cap);

	void Finish();

	void EnqueLeaves(std::vector<QuadTreeNode*>& leaves);

	void BuilderThread();

	void Init();

	bool GetChunk(std::size_t node, TerrainChunk& chunk) const;
};
