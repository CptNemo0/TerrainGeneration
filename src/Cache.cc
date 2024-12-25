#include "../include/Cache.h"

CacheNode::CacheNode(const std::size_t key, TerrainChunk& value)
{
	this->key = key;
	this->value = value;
	this->next = nullptr;
	this->previous = nullptr;
}

CacheNode::CacheNode(const std::size_t key)
{
	this->key = key;
	this->value = TerrainChunk();
}

CacheNode::CacheNode(const std::size_t key, TerrainChunk value)
{
	this->key = key;
	this->value = value;
	this->next = nullptr;
	this->previous = nullptr;
}

Cache::Cache(int capacity)
{
	this->capacity = capacity;
	left = new CacheNode(0, {});
	right = new CacheNode(0, {});

	left->next = right;
	right->previous = left;
}

TerrainChunk* Cache::get(std::size_t key)
{
	if (map.contains(key))
	{
		remove(map[key]);
		insert(map[key]);
		return &(map[key]->value);
	}
	return nullptr;
}

void Cache::remove(CacheNode* node)
{
	//std::cout << "map.size(): " << map.size() << std::endl;
	auto prev = node->previous;
	auto nxt = node->next;

	prev->next = nxt;
	nxt->previous = prev;
}

void Cache::insert(CacheNode* node)
{
	right->previous->next = node;
	node->previous = right->previous;
	node->next = right;
	right->previous = node;
}

void Cache::put(CacheNode* node)
{
	map[node->key] = node;
	insert(node);

	if (map.size() >= capacity)
	{
		std::cout << "Evicting\n";
		auto lru = left->next;
		remove(lru);
		map.erase(map.find(lru->key));
		staged.erase(staged.find(lru->key));
		//lru->value.~TerrainChunk();
		delete lru;
	}
}
