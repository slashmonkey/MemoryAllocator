#include <iostream>

#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"

struct TinyObject {
	char data;
};

struct SmallObject {
	bool flag;
	int value;
};

struct LargeObject {
	char data[1024];
};

int main()
{
	std::cout << "======= LinearAllocator =======" << std::endl;
	// LinearAllocator
	LinearAllocator allocator(2048);

	int* a = static_cast<int*>(allocator.allocate(sizeof(int)));
	*a = 3;
	std::cout << *a << std::endl;

	SmallObject* data = static_cast<SmallObject*>(allocator.allocate(sizeof(SmallObject)));
	data->value = 11;
	std::cout << data->value << std::endl;

	allocator.reset();

	std::cout << "======= StackAllocator =======" << std::endl;
	// StackAllocator
	StackAllocator stackAllocator(2048);
	void* tinyObj = stackAllocator.allocate(sizeof(TinyObject));
	stackAllocator.printMemoryMap();
	void* largeObj = stackAllocator.allocate(sizeof(LargeObject));
	stackAllocator.printMemoryMap();
	void* smallObj = stackAllocator.allocate(sizeof(SmallObject));
	stackAllocator.printMemoryMap();

	stackAllocator.free(smallObj);
	stackAllocator.printMemoryMap();

	stackAllocator.free(largeObj);
	stackAllocator.printMemoryMap();

	std::cout << "======= PoolAllocator =======" << std::endl;
	// PoolAllocator
	PoolAllocator<64> poolAllocator;
	TinyObject* tinyObjForPool = static_cast<TinyObject*>(poolAllocator.allocate());
	tinyObjForPool->data = 5;
	poolAllocator.printMemoryMap();

	SmallObject* smallObjForPool = static_cast<SmallObject*>(poolAllocator.allocate());
	smallObjForPool->value = 11;
	smallObjForPool->flag = true;
	poolAllocator.printMemoryMap();
	poolAllocator.free(tinyObjForPool);
	poolAllocator.printMemoryMap();
	return 0;
}
