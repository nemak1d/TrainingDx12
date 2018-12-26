//*************************************************************************
/*
 *	MemoryAllocator.cpp
 */
 //************************************************************************
#include <assert.h>
#include "MemoryAllocator.h"
#include "HeapFactory.h"
#include "Heap.h"

using N1D::HeapFactory;

void* operator new (size_t size, Heap* pHeap)
{
	assert(nullptr != pHeap);
	return pHeap->Allocate(size);
}

void* operator new (size_t size)
{
	return operator new (size, HeapFactory::GetDefaultHeap());
}

void operator delete(void* pMemory)
{
	if (nullptr != pMemory)
	{
		Heap::Deallocate(pMemory);
	}
}


