#include "FreeListAllocator.h"

#include <iostream>


FreeListAllocator::FreeListAllocator(std::size_t size, FreeListAllocator::SearchMethod method) : m_size(size),
	m_searchMethod(method)
{
	m_start = new char[m_size];
	m_freelist = reinterpret_cast<FreeBlock*>(m_start);
	m_freelist->next = nullptr;
	// ?
	m_freelist->size = m_size - sizeof(FreeBlock);
}

FreeListAllocator::~FreeListAllocator()
{
	delete[] m_start;
	m_freelist = nullptr;
}

// memory layout
// padding| header | data|
// ↑	  ↑		   ↑
// data address + padding + header (aligned)
void* FreeListAllocator::allocate(size_t size, size_t alignment)
{
	if (size == 0) return nullptr;
	if (alignment == 0) alignment = alignof(std::max_align_t);

	std::cout << "Allocate(" << size << " bytes, align=" << alignment << ")\n";

	SearchResult result = search(size, alignment);
	if (!result.success)
	{
		std::cout << "Failed to allocate " << size << " bytes\n";
		return nullptr;
	}

	FreeBlock* nextFreeBlock = result.block;
	FreeBlock* prev = result.prev;
	const size_t padding = result.padding;
	const size_t totalSizeForNewData = result.totalSize;

	uintptr_t blockAddress = reinterpret_cast<uintptr_t>(nextFreeBlock);
	const size_t freeBlockTotalSize = nextFreeBlock->size + sizeof(FreeBlock);

	std::cout << "  Padding: " << padding << " bytes\n";
	std::cout << "  Total needed: " << totalSizeForNewData << " bytes\n";

	const size_t remainingSize = freeBlockTotalSize - totalSizeForNewData;
	// minimal needed space for next allocation
	size_t splitSize = std::max(alignment, sizeof(FreeBlock));
	// ensure the remainder space is enough for  free block + data
	if (remainingSize >= sizeof(FreeBlock) + splitSize)
	{
		const uintptr_t newBlockAddress = blockAddress + totalSizeForNewData;
		FreeBlock* newFreeBlock = reinterpret_cast<FreeBlock*>(newBlockAddress);
		newFreeBlock->size = remainingSize - splitSize;
		newFreeBlock->next = nextFreeBlock->next;

		if (prev)
		{
			prev->next = newFreeBlock;
		}
		else
		{
			m_freelist = newFreeBlock;
		}
	}
	else
	{
		if (prev)
		{
			prev->next = nextFreeBlock->next;
		}
		else
		{
			m_freelist = nextFreeBlock->next;
		}
	}

	const uintptr_t headerAddress = blockAddress + padding;
	Header* header = reinterpret_cast<Header*>(headerAddress);
	header->size = size;
	header->padding = padding;

	const uintptr_t userDataAddress = headerAddress + sizeof(Header);
	return reinterpret_cast<void*>(userDataAddress);
}

FreeListAllocator::SearchResult FreeListAllocator::search(const size_t size, const size_t alignment)
{
	switch (m_searchMethod)
	{
	case SearchMethod::FIRST_FIT:
		return findFirstFit(size, alignment);
		break;
	case SearchMethod::BEST_FIT:
		return findBestFit(size, alignment);
		break;
	default:
		return SearchResult();
	}
}

FreeListAllocator::SearchResult FreeListAllocator::findFirstFit(const size_t size, const size_t alignment)
{
	SearchResult result;

	FreeBlock* prev = nullptr;
	FreeBlock* curr = m_freelist;

	while (curr)
	{
		const uintptr_t currentAddress = reinterpret_cast<uintptr_t>(curr);
		uintptr_t dataAddress = currentAddress + sizeof(Header);
		size_t padding = calculatePadding(dataAddress, alignment);
		size_t totalSizeNeeded = padding + sizeof(Header) + size;

		if (curr->size + sizeof(FreeBlock) >= totalSizeNeeded)
		{
			result.block = curr;
			result.prev = prev;
			result.padding = padding;
			result.totalSize = totalSizeNeeded;
			result.success = true;
			return result;
		}

		prev = curr;
		curr = curr->next;
	}

	return result;
}

FreeListAllocator::SearchResult FreeListAllocator::findBestFit(const size_t size, const size_t alignment)
{
	SearchResult result;
	FreeBlock* bestFit = nullptr;
	size_t bestWastedSize = std::numeric_limits<size_t>::max();

	FreeBlock* prev = nullptr;
	FreeBlock* curr = m_freelist;

	while (curr)
	{
		const uintptr_t currentAddress = reinterpret_cast<uintptr_t>(curr);
		uintptr_t dataAddress = currentAddress + sizeof(Header);
		size_t padding = calculatePadding(dataAddress, alignment);
		size_t totalSizeNeeded = padding + sizeof(Header) + size;
		size_t blockTotalSize = curr->size + sizeof(FreeBlock);

		if (blockTotalSize >= totalSizeNeeded)
		{
			size_t wastedSize = blockTotalSize - totalSizeNeeded;
			if (wastedSize < bestWastedSize)
			{
				bestWastedSize = wastedSize;
				result.block = curr;
				result.prev = prev;
				result.padding = padding;
				result.totalSize = totalSizeNeeded;
				result.success = true;

				// perfect match
				if (wastedSize == 0)
				{
					break;
				}
			}
		}

		prev = curr;
		curr = curr->next;
	}

	return result;
}

std::size_t FreeListAllocator::calculatePadding(std::size_t address, std::size_t alignment)
{
	size_t adjustment = alignment - (address % alignment);
	if (adjustment == alignment) adjustment = 0;

	return adjustment;
}

void FreeListAllocator::reset()
{
	m_freelist = reinterpret_cast<FreeBlock*>(m_start);
	m_freelist->next = nullptr;
	m_freelist->size = m_size - sizeof(FreeBlock);
	std::cout << "Reset\n";
}

void FreeListAllocator::free(void* ptr)
{
	if (ptr == nullptr) return;

	Header* header = reinterpret_cast<Header*>(reinterpret_cast<uintptr_t>(ptr) - sizeof(Header));
	std::cout << "Free: Size: " << header->size << " bytes, Padding: " << header->padding << "\n";

	uintptr_t toFreeBlockAddress = reinterpret_cast<uintptr_t>(header) - header->padding;
	size_t totalSize = header->padding + sizeof(Header) + header->size;

	FreeBlock* freeBlock = reinterpret_cast<FreeBlock*>(toFreeBlockAddress);
	freeBlock->size = totalSize - sizeof(FreeBlock);

	std::cout << "  Freeing " << totalSize << "\n";

	FreeBlock* prev = nullptr;
	FreeBlock* curr = m_freelist;
	while (curr && curr < freeBlock)
	{
		prev = curr;
		curr = curr->next;
	}

	// Try to coalesce with next block
	const uintptr_t blockEnd = toFreeBlockAddress + sizeof(FreeBlock) + freeBlock->size;
	if (curr && reinterpret_cast<uintptr_t>(curr) == blockEnd)
	{
		std::cout << "  Coalescing with next block\n";
		freeBlock->size += curr->size + sizeof(FreeBlock);
		freeBlock->next = curr->next;
	}
	else
	{
		freeBlock->next = curr;
	}

	// Try to coalesce with previous block
	if (prev)
	{
		const uintptr_t prevEnd = reinterpret_cast<uintptr_t>(prev) + prev->size + sizeof(FreeBlock);
		if (toFreeBlockAddress == prevEnd)
		{
			std::cout << "  Coalescing with previous block\n";
			prev->size += freeBlock->size + sizeof(FreeBlock);
			prev->next = freeBlock->next;
		}
		else
		{
			prev->next = freeBlock;
		}
	}
	else
	{
		m_freelist = freeBlock;
	}
}

void FreeListAllocator::printStats() const
{
	std::cout << "Stats: available size for each block:";
	FreeBlock* curr = m_freelist;
	size_t count = 0;

	while (curr)
	{
		std::cout << " " << curr->size;
		curr = curr->next;
		++count;
	}
	std::cout << "\n";
	std::cout << "  Total free blocks count: " << count << "\n";
}
