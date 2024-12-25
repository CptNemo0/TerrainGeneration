#pragma once

#include<vector>
#include<queue>
#include<string>
#include<sstream>
#include <thread>
#include <mutex>
#include <queue>
#include <math.h>
#include <cassert>

struct QuadTreeNode
{
	int depth = 0;
	int size;
	int x;
	int y;

	QuadTreeNode* tl = nullptr;
	QuadTreeNode* tr = nullptr;
	QuadTreeNode* bl = nullptr;
	QuadTreeNode* br = nullptr;

	QuadTreeNode(int x, int y, int size, int depth)
	{
		this->depth = depth;
		this->x = x;
		this->y = y;
		this->size = size;
	}

	std::string to_string()
	{
		std::stringstream ss;

		ss << "x: " << x << " y: " << y << " size: " << size << " depth: " << depth << std::endl;

		return ss.str();
	}
};

struct QuadTreeNodeHash
{
	std::size_t operator()(const QuadTreeNode& node) const
	{
		std::size_t h1 = std::hash<float>()(node.depth);
		std::size_t h2 = std::hash<float>()(node.size);
		std::size_t h3 = std::hash<float>()(node.x);
		std::size_t h4 = std::hash<float>()(node.y);

		return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

struct QuadTreeNodePtrHash
{
	std::size_t operator()(const QuadTreeNode* node) const
	{
		std::size_t h1 = std::hash<float>()(node->depth);
		std::size_t h2 = std::hash<float>()(node->size);
		std::size_t h3 = std::hash<float>()(node->x);
		std::size_t h4 = std::hash<float>()(node->y);

		return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

struct QuadTreeNodePtrDepthCopr
{
	std::size_t operator()(const QuadTreeNode* node1, const QuadTreeNode* node2) const
	{
		return node1->depth < node2->depth;
	}
};

struct QuadTree
{
public:
	int start_size;
	int start_x;
	int start_y;
	int max_depth;
	int inner_chunk_size;

	QuadTreeNode* head;

	std::vector<QuadTreeNode*> leaves;

	bool ispow2(int num)
	{
		int ctr = 0;
		for (int i = 0; i < 32; i++)
		{
			if ((num >> i) & 1) ctr++;
		}

		return (ctr == 1);
	}


	QuadTree(int size, float x, float y, int max_depth)
	{
		assert(ispow2(size));

		this->start_size = size;
		start_x = x;
		start_y = y;
		this->max_depth = max_depth;
		head = nullptr;

		inner_chunk_size = size / pow(2, max_depth);
	}

	void BuildTreeDfs(QuadTreeNode* current, const float x, const float y)
	{
		if (!current) return;
		auto new_size = current->size / 2;
		bool in = (current->x - new_size <= x &&
				   current->x + new_size >= x &&
				   current->y - new_size <= y &&
				   current->y + new_size >= y);

		
		auto half = new_size / 2;
		auto distance = ((x - current->x) * (x - current->x) + (y - current->y) * (y - current->y));
		if (distance / current->size < current->size &&
			current->size >(inner_chunk_size / 2) &&
			current->depth < max_depth)
		{
			current->tl = new QuadTreeNode(current->x - half, current->y + half, new_size, current->depth + 1);
			current->tr = new QuadTreeNode(current->x + half, current->y + half, new_size, current->depth + 1);
			current->bl = new QuadTreeNode(current->x - half, current->y - half, new_size, current->depth + 1);
			current->br = new QuadTreeNode(current->x + half, current->y - half, new_size, current->depth + 1);

			BuildTreeDfs(current->tl, x, y), current->depth;
			BuildTreeDfs(current->tr, x, y), current->depth;
			BuildTreeDfs(current->bl, x, y), current->depth;
			BuildTreeDfs(current->br, x, y), current->depth;
		}
		else
		{
			leaves.push_back(current);
		}
	}

	void BuildTree(int x, int y)
	{
		start_x = (x / inner_chunk_size) * inner_chunk_size;
		start_y = (y / inner_chunk_size) * inner_chunk_size;
		
		head = new QuadTreeNode(start_x, start_y, start_size, 0);
		
		BuildTreeDfs(head, x, y);
	}


	void CleanTreDfs(QuadTreeNode* node)
	{
		if (!node) return;

		CleanTreDfs(node->tl);
		CleanTreDfs(node->tr);
		CleanTreDfs(node->bl);
		CleanTreDfs(node->br);

		delete node;
	}

	void CleanTree()
	{
		if (head)
		{
			CleanTreDfs(head);
		}
		leaves.clear();
	}
};