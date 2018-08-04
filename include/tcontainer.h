#pragma once

#include <assert.h>
#include "utility.h"

template<typename T, unsigned CAPACITY = 12, unsigned THRESHOLD = 0x800000>
class FArray
{
private:
	unsigned m_uCount;
	unsigned m_uCapacity;
	unsigned m_uThreshold;
	T* m_ptr;

public:
	FArray() : m_uCount(0), m_uCapacity(CAPACITY), m_uThreshold(THRESHOLD), m_ptr(0) { m_ptr = (T*)malloc(sizeof(T) * CAPACITY); assert(m_ptr); }
	~FArray() { if (m_ptr) free(m_ptr); m_ptr = 0; }

	int Resize(unsigned uCount, int bZero = true)
	{
		unsigned uL = m_uCount * sizeof(T);
		unsigned uR = uCount * sizeof(T);
		if (uCount <= m_uCapacity)
		{
			if (bZero && uCount > m_uCount) memset(&m_ptr[m_uCount], 0, uR - uL);
			m_uCount = uCount;
			return true;
		}
		T* ptr = (T*)malloc(uR);
		if (!ptr) return false;
		memcpy(ptr, m_ptr, uL);
		if (bZero && uCount > m_uCount) memset(&ptr[m_uCount], 0, uR - uL);
		free(m_ptr);
		m_ptr = ptr;
		m_uCount = uCount;
		m_uCapacity = uCount;
		return true;
	}

	inline unsigned	Count()		const { return m_uCount; }
	inline unsigned	Capacity()	const { return m_uCapacity; }
	inline unsigned	Threshold()	const { return m_uThreshold; }
	inline int		Empty()		const { return m_uCount > 0 ? 0 : 1; }

	inline void		Reset() { m_uCount = 0; }
	inline void		Clear() { m_uCount = 0; }

	const T&	operator[](unsigned uIdx) const { return m_ptr[uIdx]; }
	T&			operator[](unsigned uIdx) { return m_ptr[uIdx]; }
	//const T*	operator[](unsigned uIdx) const { return m_ptr + uIdx; }
	//T*		operator[](unsigned uIdx)		{ return m_ptr + uIdx; }
	T*			Get(unsigned uIdx) { return m_ptr + uIdx; }

private:
	inline T* push()
	{
		if (m_uCount == m_uCapacity)
		{
			unsigned uSize = (m_uCount + 1) * sizeof(T);
			if (uSize > m_uThreshold) uSize += m_uThreshold;
			else uSize <<= 1;
			T* ptr = (T*)malloc(uSize);
			if (!ptr) return 0;
			memcpy(ptr, m_ptr, m_uCount * sizeof(T));
			free(m_ptr);
			m_ptr = ptr;
			m_uCapacity = uSize / sizeof(T);
		}
		m_uCount++;
		return m_ptr + m_uCount - 1;
	}

public:
	T * Push() { return push(); }
	//T&	Push() { T* ptr = push(); if (!ptr) assert(0); return *ptr; }	not safe
	T*	Push(const T& e) { T* p = push(); if (!p) return 0; memcpy(p, &e, sizeof(T)); return p; }

	void Erase(unsigned uIdx)
	{
		if (uIdx >= m_uCount) return;
		if (uIdx < m_uCount - 1) memcpy(m_ptr + uIdx, m_ptr + uIdx + 1, (m_uCount - uIdx - 1) * sizeof(T));
		m_uCount--;
	}
	void ExchangeErase(unsigned uIdx)
	{
		if (uIdx >= m_uCount) return;
		if (uIdx < m_uCount - 1) memcpy(m_ptr + uIdx, m_ptr + m_uCount - 1, sizeof(T));
		m_uCount--;
	}
	T* EraseBack()
	{
		if (m_uCount == 0) return nullptr;
		m_uCount--;
		return m_ptr + m_uCount;
	}

	FArray(const FArray& fa)
	{
		*this = fa;
		unsigned uSize = m_uCapacity * sizeof(T);
		m_ptr = (T*)malloc(uSize);
		assert(m_ptr);
		memcpy(m_ptr, fa.m_ptr, uSize);
	}

	T* Begin() { return m_ptr; }
	T* End() { return m_ptr + m_uCount; }

	void Sort(int(*comparator)(const void*, const void*)) { qsort(m_ptr, m_uCount, sizeof(T), comparator); }
};
