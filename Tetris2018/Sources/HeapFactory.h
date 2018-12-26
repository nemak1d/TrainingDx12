//*************************************************************************
/*
 *	HeapFactory.h
 */
 //************************************************************************
#ifndef HEAP_FACTORY_H
#define HEAP_FACTORY_H

namespace N1D
{
	class Heap;

	class HeapFactory
	{
	public:
		static Heap* Create(const char* name);
		static Heap* Create(const char* name, const char* parent);
		static void Destroy(Heap* pHeap);

		static Heap* GetDefaultHeap();

		static void PrintInfo();

		static int GetMemoryBookmark();
		static void ReportMemoryLeaks(int bookmark);
		static void ReportMemoryLeaks(int bookmark1, int bookmark2);

	private:
		static void Initialize();

		static Heap* GetRootHeap();
		static Heap* FindHeap(const char* name);
		static Heap* CreateNew(const char* name);

		static const int MaxHeapCount = 512;

		static Heap* s_pRoot;
		static Heap* s_pDefault;
		static Heap s_heaps[MaxHeapCount];
	};
}
#endif // HEAP_FACTORY_H

