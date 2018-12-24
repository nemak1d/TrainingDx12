//************************************************************************
/*
 *	Heap.h
 */
 //************************************************************************
#ifndef HEAP_H
#define HEAP_H

namespace N1D
{
	struct AllocHeader;

	class Heap
	{
	public:
		void Initialize();

		void* Allocate(size_t bytes);
		static void Deallocate(void* pMemory);

		void Activate(const char* name);
		void Deactivate();

		void AttachTo(Heap* pParent);

		bool IsActive() const { return _isActive; }
		const char* GetName() const;

		void PrintTreeInfo(int indentLevel = 0) const;
		void PrintInfo(int indentLevel = 0) const;

		int ReportMemoryLeaks(int bookmark1, int bookmark2);

		static int GetMemoryBookmark();

	private:
		void Deallocate(AllocHeader* pHeader);
		void GetTreeStats(size_t& totalBytes, size_t& totalPeak, 
			int& totalInstances) const;

		static const unsigned int NameLength = 64;

		static int s_nextAllocNum;

		bool _isActive;
		char _name[NameLength];
		size_t _allocatedBytes;
		size_t _peakBytes;
		int _instances;
		AllocHeader* _pRoot;

		Heap* _pParent;
		Heap* _pFirstChild;
		Heap* _pNextSibling;
		Heap* _pPrevSibling;
	};

}
#endif // HEAP_H

