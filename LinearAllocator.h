#ifndef MEMORYALLOCATOR_LINEARALLOCATOR_H
#define MEMORYALLOCATOR_LINEARALLOCATOR_H

#include <cstddef>

class LinearAllocator
{
public:
	LinearAllocator(const std::size_t size);

	~LinearAllocator();

	void* allocate(const std::size_t size, const std::size_t alignment = alignof(std::max_align_t));

	void reset();

private:
	std::size_t align(const std::size_t size, const std::size_t alignment) const;

	char* m_start = nullptr;
	std::size_t m_size{};
	std::size_t m_used{};
};


#endif //MEMORYALLOCATOR_LINEARALLOCATOR_H
