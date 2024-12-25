#include "../include/TerrainBuilder.h"
#include <functional>
#include <chrono>   
#include <stack>

TerrainBuilder::TerrainBuilder(std::mutex* device_mutex_ptr, int num_workers, int cap)
{
	cache = Cache(cap);
	device_mutex = device_mutex_ptr;
	this->num_workers = num_workers;
}

void TerrainBuilder::Finish()
{
	finished.store(true);
	queue_cv.notify_all();
}

void TerrainBuilder::EnqueLeaves(std::vector<QuadTreeNode*>& leaves)
{
	std::stack<QuadTreeNode*> s;

	{
		std::unique_lock<std::mutex> map_lock(map_mutex);
		QuadTreeNodePtrHash hasher;
		for (auto& node : leaves)
		{
			auto hash = hasher(node);
			if (cache.get(hash)) continue;
			if (cache.staged.contains(hash)) continue;
			s.push(node);
			cache.staged.insert(hash);
		}
	}

	{
		std::unique_lock<std::mutex> que_lock(que_mutex);

		while (!s.empty())
		{
			auto node = s.top();
			s.pop();
			construction_queue.push(node);
		}

		queue_cv.notify_all();
	}
}

void TerrainBuilder::BuilderThread()
{
	auto id = std::this_thread::get_id()._Get_underlying_id();
	while (true)
	{
		QuadTreeNode* node = nullptr;

		{
			std::unique_lock<std::mutex> que_lock(que_mutex);
			queue_cv.wait(que_lock, [&]() { return !construction_queue.empty() || finished.load(); });
			
			if (finished.load() && construction_queue.empty()) break;
			node = construction_queue.top();
			construction_queue.pop();
		}
		QuadTreeNodePtrHash hasher;
		auto hash = hasher(node);

		CacheNode* cache_node = new CacheNode(hash);

		cache_node->value.x = static_cast<float>(node->x);
		cache_node->value.z = static_cast<float>(node->y);
		cache_node->value.size = static_cast<float>(node->size);
		cache_node->value.position = DirectX::XMVectorSet(cache_node->value.x, 0.0f, cache_node->value.z, 0.0f);
		cache_node->value.resolution = node->depth * 6;
		cache_node->value.dp = static_cast<float>(node->size) / (node->depth * 6);
		cache_node->value.resolution += 1;

		cache_node->value.BuildChunk();

		{
			std::lock_guard<std::mutex> device_lock(*device_mutex);
			cache_node->value.CreateBuffers();
		}

		{
			std::lock_guard<std::mutex> map_lock(map_mutex);
			cache.put(cache_node);
			std::cout << "Thread " << id << ": created: " << node->to_string() << std::endl;
		}

		if (finished.load()) break;
	}
}

void TerrainBuilder::Init()
{
	
	for (int i = 0; i < num_workers; i++)
	{
		workers.push_back(std::thread(&TerrainBuilder::BuilderThread, this));
		workers.back().detach();
	}
}

bool TerrainBuilder::GetChunk(std::size_t hash, TerrainChunk& chunk) const
{
	if (!built.contains(hash)) return false;
	chunk = built.at(hash);
	return true;
}