//
// Created by YongHou on 2025-10-24.
//

#include "LinearAllocator.h"

LinearAllocator::LinearAllocator(const std::size_t size) : m_size(size), m_used(0)
{
	m_start = new char[m_size];
}

LinearAllocator::~LinearAllocator()
{
	delete[] m_start;
}

// calculate the adjustment
std::size_t LinearAllocator::align(const std::size_t size, const std::size_t alignment) const
{
	// alignment must be a power of 2
	size_t adjustment = alignment - (size & (alignment - 1));
	// A more general way
	//size_t adjustment = alignment - (size % alignment));

	// if the adjustment is 0, we are already aligned
	if (adjustment == alignment) adjustment = 0;

	return adjustment;
}

void* LinearAllocator::allocate(const std::size_t size, const std::size_t alignment)
{
	size_t current = reinterpret_cast<size_t>(m_start) + m_used;
	const std::size_t adjustment = align(size, alignment);
	if (m_used + adjustment + size > m_size)
		return nullptr; // out of memory

	size_t aligned = current + adjustment;
	m_used += adjustment + size;
	return reinterpret_cast<void*>(aligned);
}

void LinearAllocator::reset()
{
	m_used = 0;
}
