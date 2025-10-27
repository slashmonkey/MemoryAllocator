#ifndef MEMORYALLOCATOR_STACKALLOCATOR_H
#define MEMORYALLOCATOR_STACKALLOCATOR_H

#include <cstddef>
#include <cstdint>

class StackAllocator
{
public:
	StackAllocator(std::size_t size);

	~StackAllocator();

	void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t));

	void free(void* ptr);

	void reset();

	void printMemoryMap() const;

private:
	struct AllocationHeader
	{
		std::size_t padding;
		std::size_t size; // Optional: store size for debugging
	};

	std::size_t calculatePadding(std::size_t address, std::size_t alignment);

	char* m_start = nullptr;
	std::size_t m_offset{};
	std::size_t m_size{};
};

#endif // MEMORYALLOCATOR_STACKALLOCATOR_H
