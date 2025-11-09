cmake_minimum_required(VERSION 4.0)
project(MemoryAllocator)

set(CMAKE_CXX_STANDARD 20)

add_executable(MemoryAllocator main.cpp
        LinearAllocator.cpp
        LinearAllocator.h
        StackAllocator.cpp
        StackAllocator.h
        PoolAllocator.h
        FreeListAllocator.cpp
        FreeListAllocator.h
)
