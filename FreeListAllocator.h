#ifndef MEMORYALLOCATOR_FREELISTALLOCATOR_H
#define MEMORYALLOCATOR_FREELISTALLOCATOR_H
#include <cstddef>


class FreeListAllocator
{
public:
	enum class SearchMethod
	{
		FIRST_FIT,
		BEST_FIT
	};

private:
	struct Header
	{
		size_t size;
		size_t padding;
	};

	struct FreeBlock
	{
		size_t size; // available space after FreeBlock struct
		FreeBlock* next;
	};

	struct SearchResult
	{
		FreeBlock* block{}; // block to use
		FreeBlock* prev{}; // previous block in list
		size_t totalSize{}; // (padding + header + user data)
		size_t padding{};
		bool success{};
	};

	size_t m_size;
	char* m_start;
	FreeBlock* m_freelist;
	SearchMethod m_searchMethod;

	SearchResult search(const size_t size, const size_t alignment);

	SearchResult findFirstFit(const size_t size, const size_t alignment);

	SearchResult findBestFit(const size_t size, const size_t alignment);

	std::size_t calculatePadding(std::size_t address, std::size_t alignment);

public:
	explicit FreeListAllocator(std::size_t size, SearchMethod method = SearchMethod::FIRST_FIT);

	~FreeListAllocator();

	void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

	void free(void* ptr);

	void reset();

	void printStats() const;
};


#endif //MEMORYALLOCATOR_FREELISTALLOCATOR_H
