#pragma once

#include "include/utility.h"
#include <map>

#ifdef USE_CSTR_CHECK
#include <set>
#endif

namespace Schema
{
	typedef const char*		c_str;

	class atstring
	{
	public:
		atstring(unsigned bucket);
		~atstring();

		c_str string(const char* origin);
		c_str ustring(unsigned value);
		c_str u64string(uint64_t value);
		c_str istring(int value);
		c_str i64string(int64_t value);
		c_str tolower(const char* origin);
		c_str path(const char* origin);
		c_str find(const char* pattern);

		c_str string(const char* origin, unsigned len);
		c_str tolower(const char* origin, unsigned len);
		c_str path(const char* origin, unsigned len);

		c_str cache(const char* origin);

	private:
		struct _mpool_unit
		{
			_mpool_unit* next;
			unsigned capacity;
			unsigned offset;
			unsigned char* buffer;
		};

		_mpool_unit* mheader;
		void* alloc(unsigned bytes);

	private:
#pragma pack(push, 1)
		struct _unit
		{
			_unit* next;
			unsigned len;
			char* str;
		};
#pragma pack(pop)

		struct _cstr_pool
		{
			unsigned bucket;
			_unit** pool;
		};

		_cstr_pool pool;

	private:
		volatile _declspec (align(64)) long _occupied0;
		volatile LONG lock;
		volatile _declspec (align(64)) long _occupied1;

		unsigned hash(const char* origin, unsigned& len);
		unsigned hashl(const char* origin, unsigned len);
		c_str make_string(const char* pending);
		c_str make_string(const char* pending, unsigned len);

	private:
		char* local;
		unsigned local_size;
		int adapt_local(unsigned bytes);

		volatile _declspec (align(64)) long _occupied2;
		volatile LONG cache_lock;
		volatile _declspec (align(64)) long _occupied3;
		std::map<void*, c_str>*	cache_cstr;

#ifdef USE_CSTR_CHECK
	private:
		std::set<unsigned long> pool_addresses;
		int in_memery_pool(const char* str);
#endif
	};

	extern atstring* _strpool;
}
