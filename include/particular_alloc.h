#pragma once

#include <assert.h>
#include <stdint.h>
#include <intrin.h>
#include <new>
#include "utility.h"
#include "tcontainer.h"

#define USM_BLOCK_COUNT			64
#define USM_CHUNK_COUNT			64
#define USM_ELEMENT_COUNT		8
#define USM_CHUNK_MASK			0x3F
#define USM_BLOCK_MASK			0xFC0
#define USM_ELEMENT_MASK		0x7000
#define USM_ELEMENT_EMPTY		0xFF
#define USM_BLOCK_EMPTY			0xFFFFFFFFFFFFFFFF
#define USM_CHUNK_EMPTY			0xFFFFFFFFFFFFFFFF
#define USM_HEADER_MASK			0xFFFF

static const uint64	USM_MASK[USM_CHUNK_COUNT] = {
	0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x100000000, 0x200000000, 0x400000000, 0x800000000,
	0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
	0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
	0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
	0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
	0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
	0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
	0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000
};

static const unsigned USM_ELEMENT_TRAVEL_MASK[USM_ELEMENT_COUNT] = {
	0x01, 0x02, 0x04, 0x08,
	0x10, 0x20, 0x40, 0x80
};

template<typename T>
class UniformSliceMemory
{
public:
	UniformSliceMemory()
		: m_uChunkMask(USM_CHUNK_EMPTY)
		, m_uCacheMask(USM_HEADER_MASK)
	{
		memset(m_pChunk, 0, sizeof(_CHUNK*) * USM_CHUNK_COUNT);
	}

	~UniformSliceMemory()
	{
		clear();
	}

	void clear()
	{
		register _CHUNK* pChunk = nullptr;
		for (unsigned i = 0; i < USM_CHUNK_COUNT; ++i)
		{
			pChunk = m_pChunk[i];
			if (pChunk)
			{
				for (unsigned j = 0; j < USM_BLOCK_COUNT; ++j)
				{
					_BLOCK& blk = pChunk->block[j];
					if (blk.ptr) ::free(blk.ptr);
				}
				::free(pChunk);
				m_pChunk[i] = nullptr;
			}
		}
		m_uChunkMask = USM_CHUNK_EMPTY;
		m_uCacheMask = USM_HEADER_MASK;
	}

	T* alloc()
	{
		if (m_uCacheMask < USM_HEADER_MASK)
			return allocFromCache();
		else if (m_uChunkMask == 0)
			return allocFromHeap();
		return allocFromChunk();
	}

	void free(T* ptr)
	{
		if (!ptr) return;
		unsigned short uHeader = *((unsigned short*)ptr - 1);
		m_uCacheMask = USM_HEADER_MASK;
		if (uHeader == USM_HEADER_MASK)
			freeFromHeap(ptr);
		else
			freeFromChunk(uHeader, ptr);
	}

private:
	struct _BLOCK
	{
		unsigned __int8 uMask;
		void* ptr;
	};
	struct _CHUNK
	{
		unsigned __int64 uMask;
		_BLOCK block[USM_BLOCK_COUNT];
		unsigned uCount;
	};

	_CHUNK* m_pChunk[USM_CHUNK_COUNT];
	unsigned __int64 m_uChunkMask;
	unsigned m_uCacheMask;

	inline T* allocFromHeap()
	{
		unsigned short* ptr = (unsigned short*)::malloc(sizeof(unsigned short) + sizeof(T));
		if (!ptr) return nullptr;
		*ptr = USM_HEADER_MASK;
		T* inst = (T*)(ptr + 1);
		//inst = new(inst) T;
		return inst;
	}

	inline void freeFromHeap(T* ptr)
	{
		//ptr->~T();
		::free((unsigned short*)ptr - 1);
	}

	inline T* allocFromCache()
	{
		register unsigned uMask = m_uCacheMask;
		m_uCacheMask = USM_HEADER_MASK;
		register unsigned uChunkIndex = uMask & USM_CHUNK_MASK;
		register unsigned uBlockIndex = (uMask & USM_BLOCK_MASK) >> 6;
		register unsigned uElementIndex = (uMask & USM_ELEMENT_MASK) >> 12;
		_CHUNK* pChunk = m_pChunk[uChunkIndex];
		assert(pChunk);
		_BLOCK& blk = pChunk->block[uBlockIndex];
		assert(blk.ptr);
		unsigned short* pHeader = (unsigned short*)((unsigned char*)(blk.ptr) + (sizeof(T) + sizeof(unsigned short)) * uElementIndex);
		*pHeader = (unsigned short)uMask;
		T* ptr = (T*)(pHeader + 1);
		//ptr = new(ptr) T;
		blk.uMask &= ~(0x01 << uElementIndex);
		if (blk.uMask == 0)
		{
			pChunk->uMask &= ~(0x01ll << uBlockIndex);
			if (pChunk->uMask == 0)
				m_uChunkMask &= ~(0x01ll << uChunkIndex);
		}
		else
		{
			_BitScanForward((unsigned long*)&uElementIndex, (unsigned long)blk.uMask);
			m_uCacheMask = (uElementIndex << 12) | (uBlockIndex << 6) | uChunkIndex;
		}
		return ptr;
	}

	inline T* allocFromChunk()
	{
		register unsigned __int64 uMask = m_uChunkMask;
		register unsigned uChunkIndex = 0;
		register unsigned uBlockIndex = 0;
		register unsigned uElementIndex = 0;
		_BitScanForward64((unsigned long*)&uChunkIndex, uMask);
		_CHUNK* pChunk = m_pChunk[uChunkIndex];
		if (!pChunk)
		{
			pChunk = (_CHUNK*)::malloc(sizeof(_CHUNK));
			if (!pChunk)
				return allocFromHeap();
			memset(pChunk, 0, sizeof(_CHUNK));
			pChunk->uMask = USM_BLOCK_EMPTY;
			m_pChunk[uChunkIndex] = pChunk;
		}
		assert(pChunk->uMask);
		uMask = pChunk->uMask;
		_BitScanForward64((unsigned long*)&uBlockIndex, uMask);
		_BLOCK& blk = pChunk->block[uBlockIndex];
		if (!blk.ptr)
		{
			void* ptr = ::malloc((sizeof(T) + sizeof(unsigned short)) * USM_ELEMENT_COUNT);
			if (!ptr)
				return allocFromHeap();
			blk.ptr = ptr;
			blk.uMask = USM_ELEMENT_EMPTY;
			pChunk->uCount++;
		}
		assert(blk.uMask);
		_BitScanForward((unsigned long*)&uElementIndex, (unsigned long)blk.uMask);
		unsigned short* pHeader = (unsigned short*)((unsigned char*)(blk.ptr) + (sizeof(T) + sizeof(unsigned short)) * uElementIndex);
		*pHeader = (unsigned short)((uElementIndex << 12) | (uBlockIndex << 6) | uChunkIndex);
		T* ptr = (T*)(pHeader + 1);
		//ptr = new(ptr) T;
		blk.uMask &= ~(0x01 << uElementIndex);
		if (blk.uMask == 0)
		{
			pChunk->uMask &= ~(0x01ll << uBlockIndex);
			if (pChunk->uMask == 0)
				m_uChunkMask &= ~(0x01ll << uChunkIndex);
		}
		else
		{
			_BitScanForward((unsigned long*)&uElementIndex, (unsigned long)blk.uMask);
			m_uCacheMask = (uElementIndex << 12) | (uBlockIndex << 6) | uChunkIndex;
		}
		return ptr;
	}

	inline void freeFromChunk(unsigned short uHeader, T* ptr)
	{
		register unsigned uChunkIndex = uHeader & USM_CHUNK_MASK;
		register unsigned uBlockIndex = (uHeader & USM_BLOCK_MASK) >> 6;
		register unsigned uElementIndex = (uHeader & USM_ELEMENT_MASK) >> 12;
		_CHUNK* pChunk = m_pChunk[uChunkIndex];
		assert(pChunk);
		_BLOCK& blk = pChunk->block[uBlockIndex];
		assert(blk.ptr);
		//ptr->~T();
		blk.uMask |= (0x01 << uElementIndex);
		if (blk.uMask == USM_ELEMENT_EMPTY)
		{
			assert(blk.ptr);
			::free(blk.ptr);
			blk.ptr = nullptr;
			blk.uMask = 0;
			pChunk->uMask |= (0x01ll << uBlockIndex);
			pChunk->uCount--;
			if (pChunk->uCount == 0)
			{
				::free(pChunk);
				m_pChunk[uChunkIndex] = 0;
			}
			m_uChunkMask |= (0x01ll << uChunkIndex);
			return;
		}
		pChunk->uMask |= (0x01ll << uBlockIndex);
		m_uChunkMask |= (0x01ll << uChunkIndex);
		m_uCacheMask = uHeader;
	}
};

#define B_USM_COUNT		64
#define B_USM_EMPTY		0xFFFFFFFFFFFFFFFF
#define B_USM_NULL		0xFF

typedef unsigned char		b_usm_mask;

template<typename T>
class BigUniformSliceMemory
{
private:
	typedef UniformSliceMemory<T>	USM;
	struct _UNIT
	{
		USM* pUSM;
		unsigned uCount;
	};
	_UNIT m_unit[B_USM_COUNT];

	unsigned __int64 m_uMask;
	Spin m_Lock;

public:
	BigUniformSliceMemory()
		: m_uMask(B_USM_EMPTY)
	{
		memset(m_unit, 0, sizeof(_UNIT) * B_USM_COUNT);
	}
	~BigUniformSliceMemory()
	{
		for (int i = 0; i < B_USM_COUNT; ++i)
		{
			_UNIT& unit = m_unit[i];
			if (unit.pUSM) delete unit.pUSM;
		}
	}

	T* alloc(b_usm_mask& id)
	{
		register unsigned __int64 uMask = 0;
		register unsigned uIndex = 0;
		register T* ptr = nullptr;
		m_Lock.lock();
		uMask = m_uMask;
		if (uMask == 0)
		{
			m_Lock.unlock();
			ptr = new(std::nothrow) T();
			id = B_USM_NULL;
			return ptr;
		}
		_BitScanForward64((unsigned long*)&uIndex, uMask);
		ptr = alloc(uIndex, id);
		m_Lock.unlock();
		ptr = new(ptr) T;
		return ptr;
	}

	void free(T* ptr, const b_usm_mask& id)
	{
		if (!ptr) return;
		if (id == B_USM_NULL) { delete ptr; return; }
		ptr->~T();
		m_Lock.lock();
		free((unsigned)id, ptr);
		m_Lock.unlock();
	}

private:
	T * alloc(unsigned uIndex, b_usm_mask& id)
	{
		_UNIT& unit = m_unit[uIndex];
		register USM* pUSM = unit.pUSM;
		if (pUSM == nullptr)
		{
			pUSM = new(std::nothrow) USM();
			if (pUSM == nullptr) return nullptr;
			m_unit[uIndex].pUSM = pUSM;
		}
		T* ptr = pUSM->alloc();
		if (ptr == nullptr) return nullptr;
		unit.uCount++;
		id = b_usm_mask(uIndex);
		if (unit.uCount >= USM_BLOCK_COUNT * USM_CHUNK_COUNT * USM_ELEMENT_COUNT)
		{
			m_uMask &= ~(0x01ll << uIndex);
		}
		return ptr;
	}

	void free(unsigned uIndex, T* ptr)
	{
		_UNIT& unit = m_unit[uIndex];
		unit.pUSM->free(ptr);
		--unit.uCount;
		m_uMask |= 0x01ll << uIndex;
	}
};

template<typename T>
class CUniformSliceMemory
{
	UniformSliceMemory<T> m_USM;

public:
	CUniformSliceMemory() {}
	~CUniformSliceMemory() {}

	T* alloc() { register T* ptr = m_USM.alloc(); if (ptr) ptr = new(ptr) T; return ptr; }
	void free(T* ptr) { if (ptr) { ptr->~T(); m_USM.free(ptr); } }
};

template<typename T>
class CTUniformSliceMemory
{
	UniformSliceMemory<T> m_USM;
	Spin m_Lock;

public:
	CTUniformSliceMemory() {}
	~CTUniformSliceMemory() {}

	T* alloc()
	{
		register T* ptr = nullptr;
		m_Lock.lock();
		ptr = m_USM.alloc();
		m_Lock.unlock();
		if (ptr) ptr = new(ptr) T;
		return ptr;
	}

	void free(T* ptr)
	{
		if (!ptr) return;
		ptr->~T();
		m_Lock.lock();
		m_USM.free(ptr);
		m_Lock.unlock();
	}
};

template<typename T>
class TChopped
{
	const static unsigned DEFAULE_LINE_COUNT = 256;
public:
	TChopped()
		: muLineCount(DEFAULE_LINE_COUNT)
		, mBlocks(nullptr) {}
	TChopped(unsigned uLineCount)
		: muLineCount(uLineCount)
		, mBlocks(nullptr) {}
	~TChopped() {}

	T* alloc()
	{
		if (marrAvailable.Count() == 0 && !increase()) return nullptr;
		T* ptr = *marrAvailable.EraseBack();
		ptr = new(ptr)T;
		return ptr;
	}
	void free(T* ptr) { ptr->~T(); marrAvailable.Push(ptr); }
	void clear()
	{
		_BLOCK* pHeader = mBlocks;
		while (pHeader)
		{
			_BLOCK* pNext = pHeader->_next;
			::free(pHeader);
			pHeader = pNext;
		}
		mBlocks = nullptr;
		marrAvailable.Clear();
	}

private:
	struct _BLOCK {
		_BLOCK* _next;
		T* _addr;
	};
	_BLOCK* mBlocks;
	FArray<T*> marrAvailable;
	unsigned muLineCount;

	int increase()
	{
		_BLOCK* pBlock = (_BLOCK*)malloc(sizeof(_BLOCK) + sizeof(T) * muLineCount);
		if (!pBlock) return false;
		pBlock->_next = mBlocks;
		pBlock->_addr = (T*)(pBlock + 1);
		mBlocks = pBlock;
		for (unsigned i = muLineCount; i > 0; --i) marrAvailable.Push(pBlock->_addr + i - 1);
		return true;
	}
};

class MemoryIncrement
{
public:
	inline MemoryIncrement() : mBlocks(nullptr) {}
	inline ~MemoryIncrement() { clear(); }

	inline void* alloc(unsigned uSize)
	{
		if (!mBlocks || uSize + mBlocks->_used > mBlocks->_capacity)
		{
			unsigned uCapacity = uSize + sizeof(_BLOCK);
			if (uCapacity < PAGE_SIZE) uCapacity = PAGE_SIZE;
			_BLOCK* pBlock = (_BLOCK*)malloc(uCapacity);
			if (!pBlock) return nullptr;
			pBlock->_addr = (byte*)(pBlock + 1);
			pBlock->_capacity = uCapacity - sizeof(_BLOCK);
			pBlock->_used = 0;
			pBlock->_next = mBlocks;
			mBlocks = pBlock;
		}
		byte* ptr = mBlocks->_addr + mBlocks->_used;
		mBlocks->_used += uSize;
		return ptr;
	}
	inline const char* set(const char* str, unsigned len) { char* addr = (char*)alloc(len); if (!addr) return nullptr; memcpy(addr, str, len); return addr; }
	inline int set(void* buff, unsigned len) { void* addr = alloc(len); if (!addr) return false; memcpy(addr, buff, len); return true; }
	inline void clear() { _BLOCK* pBlock = mBlocks; while (pBlock) { _BLOCK* pTmp = pBlock->_next; free(pBlock); pBlock = pTmp; } }
	inline void* travel(void* it) { if (it == nullptr) return mBlocks; _BLOCK* pBlock = (_BLOCK*)it; return pBlock->_next; }
	inline unsigned getSize(void* it) { _BLOCK* pBlock = (_BLOCK*)it; return pBlock->_used; }
	inline const char* getPtr(void* it) { _BLOCK* pBlock = (_BLOCK*)it; return (const char*)pBlock->_addr; }

private:
	struct _BLOCK {
		_BLOCK* _next;
		byte* _addr;
		unsigned _capacity;
		unsigned _used;
	};
	_BLOCK* mBlocks;
};
