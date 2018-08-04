#pragma once

#include "tcontainer.h"
#include "particular_alloc.h"

namespace Hash
{
	template<typename T> uint32 Hash(T v) { return HashTypeOverload(v); }
	uint32 HashTypeOverload(uint32 v) { return v; }
	uint32 HashTypeOverload(int32 v) { return v; }
	uint32 HashTypeOverload(uint64 v) { return uint32(v) ^ uint32(v >> 32); }
	uint32 HashTypeOverload(int64 v) { return uint32(v) ^ uint32(uint64(v) >> 32); }
};

template<typename KeyType, typename ValueType>
class GenericHashTable
{
public:
	GenericHashTable()
		: muCapacity(0)
		, muCount(0)
		, mIterator(nullptr)
		, muInterator(0)
		, mTable(nullptr) {}
	~GenericHashTable() { Clear(); }

	uint32 GetCount() const { return muCount; }
	uint32 GetCapacity() const { return muCapacity; }

	int Clear()
	{
		for (uint32 i = 0; i < muCapacity; ++i)
		{
			_UNIT* pHeader = mTable[i];
			while (pHeader)
			{
				_UNIT* pNext = pHeader->_next;
				mAlloc.free(pHeader);
				pHeader = pNext;
			}
		}
		mAlloc.clear();
		if (mTable)
		{
			::free(mTable);
			mTable = nullptr;
		}
		muCapacity = 0;
		muCount = 0;
		return true;
	}

	ValueType* Find(const KeyType& key)
	{
		if (muCount == 0) return nullptr;
		uint32 uHash = Hash::Hash(key);
		return Find(uHash, key);
	}
	ValueType* operator[](const KeyType& key) { return Find(key); }
	ValueType* Insert(const KeyType& key)
	{
		uint32 uHash = Hash::Hash(key);
		if (muCount > 0) { ValueType* pVal = Find(uHash, key); if (pVal) return pVal; }
		if (muCount * 4 >= muCapacity * 3 && !Increase()) return nullptr;
		_UNIT* pUnit = mAlloc.alloc();
		if (!pUnit) return nullptr;
		pUnit->_hash = uHash;
		pUnit->_key = key;
		uint32 uPos = uHash & (muCapacity - 1);
		++muCount;
		pUnit->_next = mTable[uPos];
		mTable[uPos] = pUnit;
		return new(&pUnit->_value)ValueType;
	}
	int Remove(const KeyType& key)
	{
		if (muCount == 0) return false;
		uint32 uHash = Hash::Hash(key);
		uint32 uPos = uHash & (muCapacity - 1);
		_UNIT* pUnit = mTable[uPos];
		_UNIT** ptr = &mTable[uPos];
		while (pUnit)
		{
			if (pUnit->_hash == uHash && pUnit->_key == key)
			{
				*ptr = pUnit->_next;
				pUnit->_value.~ValueType();
				mAlloc.free(pUnit);
				return true;
			}
			ptr = &pUnit->_next;
			pUnit = pUnit->_next;
		}
		return false;
	}

	int Begin() { mIterator = nullptr; muInterator = 0; return Next(); }
	int End() { return false; }
	int Next()
	{
		if (mIterator && mIterator->_next)
		{
			mIterator = mIterator->_next;
			return true;
		}
		mIterator = nullptr;
		for (uint32 i = muInterator; i < muCapacity; ++i)
		{
			if (mTable[i])
			{
				muInterator = i + 1;
				mIterator = mTable[i];
				return true;
			}
		}
		return false;
	}
	KeyType* GetCurKey() { return &mIterator->_key; }
	ValueType* GetCurValue() { return &mIterator->_value; }

private:
	struct _UNIT {
		uint32 _hash;
		KeyType _key;
		ValueType _value;
		_UNIT* _next;
	};

	uint32 muCapacity;
	uint32 muCount;
	_UNIT** mTable;
	TChopped<_UNIT> mAlloc;

	_UNIT* mIterator;
	uint32 muInterator;

	ValueType* Find(uint32 uHash, const KeyType& key)
	{
		uint32 uPos = uHash & (muCapacity - 1);
		_UNIT* pHeader = mTable[uPos];
		while (pHeader)
		{
			if (pHeader->_hash == uHash && pHeader->_key == key) return &pHeader->_value;
			pHeader = pHeader->_next;
		}
		return nullptr;
	}
	int Increase()
	{
		uint32 uCapacity = muCapacity << 1;
		if (uCapacity < 128) uCapacity = 128;
		_UNIT** ptr = (_UNIT**)malloc(sizeof(_UNIT*) * uCapacity);
		if (!ptr) return false;
		memset(ptr, 0, sizeof(_UNIT*) * uCapacity);
		for (uint32 i = 0; i < muCapacity; ++i)
		{
			_UNIT* pUnit = mTable[i];
			while (pUnit)
			{
				unsigned uPos = pUnit->_hash & (uCapacity - 1);
				_UNIT* pNext = pUnit->_next;
				pUnit->_next = ptr[uPos];
				ptr[uPos] = pUnit;
				pUnit = pNext;
			}
		}
		if (mTable) ::free(mTable);
		mTable = ptr;
		muCapacity = uCapacity;
		return true;
	}
};
