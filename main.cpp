#include <iostream>

#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"

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
	LinearAllocator linearAllocator(2048);

	int* a = static_cast<int*>(linearAllocator.allocate(sizeof(int)));
	*a = 3;
	std::cout << *a << std::endl;

	SmallObject* data = static_cast<SmallObject*>(linearAllocator.allocate(sizeof(SmallObject)));
	data->value = 11;
	std::cout << data->value << std::endl;

	linearAllocator.reset();

	std::cout << "======= StackAllocator =======" << std::endl;
	// StackAllocator
	StackAllocator stackAllocator(2048);
	void* tinyObj = stackAllocator.allocate(sizeof(TinyObject));
	stackAllocator.printStats();
	void* largeObj = stackAllocator.allocate(sizeof(LargeObject));
	stackAllocator.printStats();
	void* smallObj = stackAllocator.allocate(sizeof(SmallObject));
	stackAllocator.printStats();

	stackAllocator.free(smallObj);
	stackAllocator.printStats();

	stackAllocator.free(largeObj);
	stackAllocator.printStats();

	stackAllocator.free(tinyObj);
	stackAllocator.printStats();

	std::cout << "======= PoolAllocator =======" << std::endl;
	// PoolAllocator
	PoolAllocator<64> poolAllocator;
	TinyObject* tinyObjForPool = static_cast<TinyObject*>(poolAllocator.allocate());
	tinyObjForPool->data = 5;
	poolAllocator.printStats();

	SmallObject* smallObjForPool = static_cast<SmallObject*>(poolAllocator.allocate());
	smallObjForPool->value = 11;
	smallObjForPool->flag = true;
	poolAllocator.printStats();
	poolAllocator.free(tinyObjForPool);
	poolAllocator.printStats();

	std::cout << "======= FreeListAllocator =======" << std::endl;
	FreeListAllocator alloc(512, FreeListAllocator::SearchMethod::FIRST_FIT);

	// Create fragmentation pattern
	void* p1 = alloc.allocate(16, 8);
	void* p2 = alloc.allocate(64, 8);
	void* p3 = alloc.allocate(32, 8);
	void* p4 = alloc.allocate(128, 8);

	alloc.printStats();

	// Free to create holes
	alloc.free(p2);  // 64-byte hole
	alloc.free(p3);  // 32-byte hole

	alloc.printStats();

	void* p5 = alloc.allocate(1, 8);
	alloc.printStats();
	alloc.free(p5);

	return 0;
}
