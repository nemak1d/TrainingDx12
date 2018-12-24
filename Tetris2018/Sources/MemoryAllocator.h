//*************************************************************************
/*
 *	MemoryAllocator.h
 */
 //************************************************************************
#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

namespace N1D
{
	class Heap;
	class HeapFactory;
}
using N1D::Heap;
using N1D::HeapFactory;

void* operator new (size_t size);
void* operator new (size_t size, Heap* pHeap);
void operator delete(void* pMemory);

#define DECLEAR_HEAP \
	public: \
		static void* operator new(size_t size); \
		static void operator delete(void* p, size_t size); \
	private: \
		static Heap* s_pHeap; 

#define DEFINE_HEAP(className, heapName) \
	Heap* className::s_pHeap = nullptr; \
	void* className::operator new(size_t size) \
	{ \
		if (nullptr == s_pHeap) \
		{ \
			s_pHeap = HeapFactory::Create(heapName); \
		} \
		return ::operator new(size, s_pHeap); \
	} \
	void className::operator delete(void* p, size_t size) \
	{ \
		::operator delete(p); \
	} \

#define DEFINE_HIERARCHICALHEAP(className, heapName, parentName) \
Heap* className::s_pHeap = nullptr; \
void* className::operator new(size_t size) \
{ \
	if (nullptr == s_pHeap) \
	{ \
		s_pHeap = HeapFactory::Create(size, s_pHeap); \
	} \
	void className::operator delete(void* p, size_t size) \
	{ \
		::operator delete(p); \
	} \
} 

#endif // MEMORY_ALLOCATOR_H

