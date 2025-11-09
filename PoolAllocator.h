#ifndef MEMORYALLOCATOR_POOLALLOCATOR_H
#define MEMORYALLOCATOR_POOLALLOCATOR_H

#include <cstddef>
#include <iostream>
#include <ostream>
#include <vector>

template<std::size_t BlockSize, std::size_t DefaultPoolSize = 64>
class PoolAllocator
{
	static_assert(BlockSize % alignof(std::max_align_t) == 0, "BlockSize must be a multiple of platform alignment");

public:
	PoolAllocator() : m_freelist(nullptr), m_poolSize(DefaultPoolSize)
	{
		allocatePool();
	}

	~PoolAllocator()
	{
		for (auto pool: m_pools)
		{
			delete[] pool;
		}
	}

	void* allocate()
	{
		if (m_freelist == nullptr)
		{
			allocatePool();
		}

		Node* node = m_freelist;
		m_freelist = node->next;

		return reinterpret_cast<void*>(node->data);
	}

	void free(void* ptr)
	{
		if (ptr == nullptr) return;
		Node* node = static_cast<Node*>(ptr);
		node->next = m_freelist;
		m_freelist = node;
	}

	void printStats() const
	{
		std::cout << "PoolAllocator: Memory Map Size: " << m_poolSize << std::endl;
		std::cout << "PoolAllocator: Memory Map Block Size: " << BlockSize << std::endl;

		size_t freeBlocks = 0;
		for (Node* node = m_freelist; node != nullptr; node = node->next)
		{
			++freeBlocks;
		}

		std::cout << "PoolAllocator:Free Blocks: " << freeBlocks << std::endl;
	}

private:
	union Node
	{
		Node* next;
		alignas(alignof(std::max_align_t)) char data[BlockSize];
	};

	void allocatePool()
	{
		Node* nodes = new Node[m_poolSize];
		m_pools.push_back(nodes);

		for (std::size_t i = 0; i < m_poolSize - 1; ++i)
		{
			nodes[i].next = &nodes[i + 1];
		}

		nodes[m_poolSize - 1].next = m_freelist;
		m_freelist = nodes;
	}

	Node* m_freelist;
	std::vector<Node*> m_pools;
	std::size_t m_poolSize;
};


#endif //MEMORYALLOCATOR_POOLALLOCATOR_H
