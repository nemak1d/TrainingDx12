//*************************************************************************
/*
 *	HeapFactory.cpp
 */
 //************************************************************************

#include <string.h>
#include <stdio.h>
#include "HeapFactory.h"
#include "Heap.h"

namespace N1D
{
	Heap* HeapFactory::s_pRoot = nullptr;
	Heap* HeapFactory::s_pDefault = nullptr;
	Heap HeapFactory::s_heaps[];

	Heap* HeapFactory::Create(const char* name)
	{
		return Create(name, "Root");
	}

	Heap* HeapFactory::Create(const char* name, const char* parent)
	{
		if (nullptr == s_pRoot)
		{
			Initialize();
		}

		Heap* pParent = FindHeap(parent);
		if (nullptr == pParent)
		{
			pParent = CreateNew(parent);
			pParent->AttachTo(s_pRoot);
		}

		Heap* pHeap = FindHeap(name);
		if (nullptr == pHeap)
		{
			pHeap = CreateNew(name);
		}

		pHeap->AttachTo(pParent);
		return pHeap;
	}

	Heap* HeapFactory::CreateNew(const char* name)
	{
		for (auto&& heap : s_heaps)
		{
			if (!heap.IsActive())
			{
				heap.Activate(name);
				return &heap;
			}
		}
		return nullptr;
	}

	Heap* HeapFactory::GetDefaultHeap()
	{
		if (nullptr == s_pDefault)
		{
			Initialize();
		}
		return s_pDefault;
	}

	Heap* HeapFactory::GetRootHeap()
	{
		if (nullptr == s_pRoot)
		{
			Initialize();
		}
		return s_pRoot;
	}

	void HeapFactory::PrintInfo()
	{
		printf_s("Memory Infomation\n");
		printf_s("                                  Local                   Total\n");
		printf_s("Name                      Memory  Peak  Inst      Memory  Peak  Inst\n");
		GetRootHeap()->PrintTreeInfo(0);
		printf_s("\n");
	}

	int HeapFactory::GetMemoryBookmark()
	{
		return Heap::GetMemoryBookmark();
	}

	void HeapFactory::ReportMemoryLeaks(int bookmark)
	{
		ReportMemoryLeaks(bookmark, GetMemoryBookmark());
	}

	void HeapFactory::ReportMemoryLeaks(int bookmark1, int bookmark2)
	{
		int leaks = 0;
		for (auto&& heap : s_heaps)
		{
			if (heap.IsActive())
			{
				leaks += heap.ReportMemoryLeaks(bookmark1, bookmark2);
			}
		}

		if (0 < leaks)
		{
			printf_s("%d memory leaks found\n", leaks);
		}
		else
		{
			printf_s("No memory leaks detected.\n");
		}
	}

	void HeapFactory::Initialize()
	{
#ifdef false
		for (auto&& heap : s_heaps)
		{
			heap.Initialize();
		}
#else
		for (int i = 0; i < MaxHeapCount; ++i)
		{
			s_heaps[i].Initialize();
		}

#endif
		s_pRoot = CreateNew("Root");
		s_pDefault = Create("Default");
	}

	Heap* HeapFactory::FindHeap(const char* name)
	{
		for (auto&& heap : s_heaps)
		{
			if (heap.IsActive() && !_stricmp(name, heap.GetName()))
			{
				return &heap;
			}
		}
		return nullptr;
	}
}
