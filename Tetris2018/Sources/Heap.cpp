//************************************************************************
/*
 *	Heap.cpp
 */
 //************************************************************************
#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "Heap.h"
#include "Log.h"

using namespace N1D;

namespace N1D
{
	struct AllocHeader
	{
		static const int DefaultSignature = 0xDEADC0DE;

		int signature;
		int allocatedCount;
		int size;
		Heap* pHeap;
		AllocHeader* pNext;
		AllocHeader* pPrev;
	};

	int Heap::s_nextAllocNum = 0;

	void Heap::Initialize()
	{
		_isActive = false;
		_allocatedBytes = 0;
		_peakBytes = 0;
		_instances = 0;
		_pRoot = nullptr;

		_pParent = nullptr;
		_pFirstChild = nullptr;
		_pNextSibling = nullptr;
		_pPrevSibling = nullptr;
	}


	void* Heap::Allocate(size_t bytes)
	{
		size_t requestedBytes = bytes + sizeof(AllocHeader);
		char* pMemory = reinterpret_cast<char*>(malloc(requestedBytes));
		AllocHeader* pHeader = reinterpret_cast<AllocHeader*>(pMemory);
		pHeader->signature = AllocHeader::DefaultSignature;
		pHeader->pHeap = this;
		pHeader->size = static_cast<int>(bytes);
		pHeader->pNext = _pRoot;
		pHeader->pPrev = nullptr;
		pHeader->allocatedCount = s_nextAllocNum++;

		// リンクの更新
		if (nullptr != _pRoot)
		{
			_pRoot->pPrev = pHeader;
		}
		_pRoot = pHeader;

		// アロケート済サイズの更新
		_allocatedBytes += bytes;
		if (_allocatedBytes > _peakBytes)
		{
			_peakBytes = _allocatedBytes;
		}
		++_instances;

		void* pStartMemoryBlock = pMemory + sizeof(AllocHeader);
		return pStartMemoryBlock;
	}

	void Heap::Deallocate(void* pMemory)
	{
		AllocHeader* pHeader = reinterpret_cast<AllocHeader*>(
			reinterpret_cast<char*>(pMemory) - sizeof(AllocHeader));

		assert(AllocHeader::DefaultSignature == pHeader->signature);
		pHeader->pHeap->Deallocate(pHeader);
	}

	void Heap::Deallocate(AllocHeader* pHeader)
	{
		if (nullptr == pHeader->pPrev)
		{
			assert(_pRoot == pHeader);
			_pRoot = pHeader->pNext;
		}
		else
		{
			pHeader->pPrev->pNext = pHeader->pNext;
		}

		if (nullptr != pHeader->pNext)
		{
			pHeader->pNext->pPrev = pHeader->pPrev;
		}
		_allocatedBytes -= pHeader->size;
		--_instances;	
		free(pHeader);
	}

	void Heap::Activate(const char* name)
	{
		assert(nullptr != name);
		assert(strlen(name) < NameLength);
		strcpy_s(_name, sizeof(_name), name);

		_allocatedBytes = 0;
		_peakBytes = 0;
		_instances = 0;
		_isActive = true;
	}

	void Heap::Deactivate()
	{
		strcpy_s(_name, sizeof(_name), "");
		_allocatedBytes = 0;
		_peakBytes = 0;
		_instances = 0;
		_isActive = false;
	}

	void Heap::AttachTo(Heap* pParent)
	{
		assert(nullptr != pParent);

		if (_pParent == pParent)
		{
			return;
		}

		if (nullptr != _pPrevSibling)
		{
			_pPrevSibling->_pNextSibling = _pNextSibling;
		}
		if (nullptr != _pNextSibling)
		{
			_pNextSibling->_pPrevSibling = _pPrevSibling;
		}
		if (nullptr != _pParent
		&& _pParent->_pFirstChild == this)
		{
			_pParent->_pFirstChild = _pNextSibling;
		}

		_pNextSibling = pParent->_pFirstChild;
		_pPrevSibling = nullptr;
		_pParent = pParent;
		pParent->_pFirstChild;		
	}

	const char* Heap::GetName() const
	{
		return _name;
	}

	void Heap::PrintTreeInfo(int indentLevel) const
	{
		PrintInfo(indentLevel);
		Heap* pChild = _pFirstChild;
		while (nullptr != pChild)
		{
			pChild->PrintTreeInfo(indentLevel + 1);
			pChild = pChild->_pNextSibling;
		}
	}

	void Heap::PrintInfo(int indentLevel) const
	{
		for (int i = 0; i < indentLevel; ++i)
		{
			Log::Write(" ");
		}

		size_t totalBytes = 0;
		size_t totalPeakBytes = 0;
		int totalInstances = 0;
		GetTreeStats(totalBytes, totalPeakBytes, totalInstances);
		
		int spacing = 20 - indentLevel * 2;
		Log::Write("%-*s %6d %6d %5d  %6d %6d %5d\n",
			spacing, _name,
			static_cast<int>(_allocatedBytes), static_cast<int>(_peakBytes), static_cast<int>(_instances),
			static_cast<int>(totalBytes), static_cast<int>(totalPeakBytes), static_cast<int>(totalInstances));
	}

	void Heap::GetTreeStats(size_t& totalBytes, size_t& totalPeak,
		int& totalInstances) const
	{
		totalBytes += _allocatedBytes;
		totalPeak += _peakBytes;
		totalInstances += _instances;

		Heap* pChild = _pFirstChild;
		while (nullptr != pChild)
		{
			pChild->GetTreeStats(totalBytes, totalPeak, totalInstances);
			pChild = pChild->_pNextSibling;
		}
	}

	int Heap::GetMemoryBookmark()
	{
		return s_nextAllocNum;
	}

	int Heap::ReportMemoryLeaks(int bookmark1, int bookmark2)
	{
		int leaks = 0;

		AllocHeader* pHeader = _pRoot;
		while (nullptr != pHeader)
		{
			if (pHeader->allocatedCount >= bookmark1
				&& pHeader->allocatedCount < bookmark2)
			{
				Log::Write("Leak in %s. Size: %d, address: 0x%0Xd\n",
					_name, pHeader->size, (reinterpret_cast<char*>(pHeader) + sizeof(AllocHeader)));
				++leaks;
			}
			pHeader = pHeader->pNext;
		}
		return leaks;
	}
}