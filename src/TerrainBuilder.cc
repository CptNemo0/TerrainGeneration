#include "../include/TerrainBuilder.h"
#include <functional>
#include <chrono>   
#include <stack>

TerrainBuilder::TerrainBuilder(std::mutex* device_mutex_ptr, int num_workers)
{
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
	//std::cout << "\tEnqueLeaves\n";	

	std::stack<std::pair<QuadTreeNode*, std::size_t>> s;	

	{
		std::unique_lock<std::mutex> map_lock(map_mutex);
		QuadTreeNodePtrHash hasher;
		//std::cout << "\t\tMAP_mutex LOcked by MAIN\n";
		for (auto& node : leaves)
		{
			auto hash = hasher(node);
			if (built.contains(hash)) continue;
			if (enqued.contains(hash)) continue;
			s.push({ node, hash });
		}
		//std::cout << "\t\tMAP_mutex UNlocked by MAIN\n";
	}

	{
		std::unique_lock<std::mutex> que_lock(que_mutex);
		//std::cout << "\t\tQUE_mutex LOcked by MAIN\n";
		while (!s.empty())
		{
			auto [node, hash] = s.top();
			s.pop();
			construction_queue.push(node);
			enqued.insert(hash);
		}

		queue_cv.notify_all();
		//std::cout << "\t\tQUE_mutex UNlocked by MAIN\n";	
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
			//std::cout << "\tThread " << id << ": locked que\n";
			node = construction_queue.top();
			construction_queue.pop();
			//std::cout << "\tThread " << id << ": unlocked que\n";
		}

		//std::cout << "\tThread " << id << ": started building\n";
		TerrainChunk chunk{ static_cast<float>(node->x),
							static_cast<float>(node->y),
							static_cast<float>(node->size),
							node->depth * 6 };

		chunk.BuildChunk();
		//std::cout << "\tThread " << id << ": finished building\n";

		{
			std::lock_guard<std::mutex> device_lock(*device_mutex);
			//std::cout << "\tThread " << id << ": locked device\n";
			chunk.CreateBuffers();
			//std::cout << "\tThread " << id << ": unlocked device\n";
		}

		{
			QuadTreeNodePtrHash hasher;
			auto hash = hasher(node);
			std::lock_guard<std::mutex> map_lock(map_mutex);
			//std::cout << "\tThread " << id << ": locked map\n";
			built[hash] = chunk;
			//std::cout << "\tThread " << id << ": unlocked map\n";
			std::cout << "Thread " << id << ": created: " << node->to_string() << std::endl;
		}

		//std::this_thread::sleep_for(std::chrono::milliseconds(2));

		
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