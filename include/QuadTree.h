#pragma once

#include<vector>
#include<queue>
#include<string>
#include<sstream>
#include <thread>
#include <mutex>
#include <queue>
#include <math.h>
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

	QuadTreeNode(int x, int y, int size)
	{
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
		std::size_t h2 = std::hash<float>()(node.size);
		std::size_t h3 = std::hash<float>()(node.x);
		std::size_t h4 = std::hash<float>()(node.y);

		return (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

struct QuadTreeNodePtrHash
{
	std::size_t operator()(const QuadTreeNode* node) const
	{
		std::size_t h2 = std::hash<float>()(node->size);
		std::size_t h3 = std::hash<float>()(node->x);
		std::size_t h4 = std::hash<float>()(node->y);

		return (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

struct QuadTree
{
public:
	int start_size;
	int start_x;
	int start_y;
	int min_size;
	int inner_chunk_size;

	float fwd_x;
	float fwd_y;

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

	bool visible(int x, int y)
	{
		DirectX::XMVECTOR dir = DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
		DirectX::XMVECTOR fwd = DirectX::XMVectorSet(fwd_x, fwd_y, 0.0f, 0.0f);

		dir = DirectX::XMVector2Normalize(dir);
		fwd = DirectX::XMVector2Normalize(fwd);

		auto dot = DirectX::XMVector2Dot(dir, fwd);
		
		return DirectX::XMVectorGetX(dot) > 0;
	}

	QuadTree(int size, float x, float y, int msize)
	{
		assert(ispow2(size));
		assert(ispow2(msize));
		this->start_size = size;
		start_x = x;
		start_y = y;
		min_size = msize;
		head = nullptr;
	}

	int BuildTreeDfs(QuadTreeNode* current, const float x, const float y)
	{
		if (!current) return 0;
		auto new_size = current->size / 2;
		auto half = new_size / 2;
		auto distance = ((current->x - x) * (current->x - x) + (current->y - y) * (current->y - y));

		current->depth = 1;

		if (distance < current->size * current->size && current->size >(min_size / 4))
		{
			current->tl = new QuadTreeNode(current->x - half, current->y + half, new_size);
			current->tr = new QuadTreeNode(current->x + half, current->y + half, new_size);
			current->bl = new QuadTreeNode(current->x - half, current->y - half, new_size);
			current->br = new QuadTreeNode(current->x + half, current->y - half, new_size);

			current->depth = max(BuildTreeDfs(current->tl, x, y), current->depth);
			current->depth = max(BuildTreeDfs(current->tr, x, y), current->depth);
			current->depth = max(BuildTreeDfs(current->bl, x, y), current->depth);
			current->depth = max(BuildTreeDfs(current->br, x, y), current->depth);

			current->depth++;
		}
		else
		{
			leaves.push_back(current);
		}

		return current->depth;
	}

	void BuildTree(int x, int y, float fwd_x, float fwd_y)
	{
		this->fwd_x = fwd_x;
		this->fwd_y = fwd_y;

		start_x = (x / min_size) * min_size;
		start_y = (y / min_size) * min_size;
		
		std::queue<QuadTreeNode*> q;
		head = new QuadTreeNode(start_x, start_y, start_size);
		
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