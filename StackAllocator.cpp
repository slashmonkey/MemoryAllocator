#include "StackAllocator.h"

#include <iostream>

StackAllocator::StackAllocator(std::size_t size) : m_size(size), m_offset(0)
{
	m_start = new char[size];
}

StackAllocator::~StackAllocator()
{
	delete[] m_start;
}

// memory layout
//						  offset
//						 ↓		(next allocation)
// padding| header | data|(padding| header | data)
// ↑	  ↑		   ↑
// offset + padding + header (aligned)
void* StackAllocator::allocate(std::size_t size, std::size_t alignment)
{
	if (size == 0) return nullptr;
	if (alignment == 0) alignment = alignof(std::max_align_t);

	constexpr std::size_t headerSize = sizeof(AllocationHeader);
	const std::size_t currentAddress = reinterpret_cast<std::size_t>(m_start) + m_offset;

	//data address should be aligned
	const size_t dataAddress = currentAddress + headerSize;
	const std::size_t padding = calculatePadding(dataAddress, alignment);
	const std::size_t totalSize = padding + headerSize + size;

	if (m_offset + totalSize > m_size)
	{
		return nullptr;
	}

	char* headerPtr = m_start + m_offset + padding;
	AllocationHeader* header = reinterpret_cast<AllocationHeader*>(headerPtr);
	header->padding = padding;
	header->size = totalSize;

	void* userDataPtr = headerPtr + headerSize;
	m_offset += totalSize;

	std::cout << "Layout: [Padding:" << padding << "][Header][Data at offset "
				  << (static_cast<char*>(userDataPtr) - m_start) << "]\n";

	return userDataPtr;
}

void StackAllocator::free(void* ptr)
{
	if (ptr == nullptr)
		return;

	size_t headerAddress = reinterpret_cast<size_t>(ptr) - sizeof(AllocationHeader);
	AllocationHeader* header = reinterpret_cast<AllocationHeader*>(headerAddress);

	m_offset = reinterpret_cast<size_t>(ptr) - header->padding - sizeof(AllocationHeader) - reinterpret_cast<size_t>(
		           m_start);
}

void StackAllocator::reset()
{
	m_offset = 0;
}

void StackAllocator::printStats() const
{
	std::cout << "Stack: " << m_offset << "/" << m_size
			<< " bytes used (" << (100.0 * m_offset / m_size) << "%)\n";
}

std::size_t StackAllocator::calculatePadding(std::size_t address, std::size_t alignment)
{
	size_t adjustment = alignment - (address % alignment);
	if (adjustment == alignment) adjustment = 0;

	return adjustment;
}
