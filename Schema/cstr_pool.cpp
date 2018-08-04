#include "cstr_pool.h"
#include "_g.h"

namespace Schema
{
	extern atstring* _strpool = 0;
#define _mpool_capacity		0x100000
#define _normalize_path		256

	atstring::atstring(unsigned bucket)
		: mheader(0)
		, lock(0)
		, local(0)
		, local_size(0)
		, cache_cstr(0)
		, cache_lock(0)
	{
		memset(&pool, 0, sizeof(_cstr_pool));

		spin_init(lock);
		spin_init(cache_lock);

		pool.bucket = bucket;
		pool.pool = (_unit**)malloc(sizeof(void*) * bucket);
		assert(pool.pool);
		memset(pool.pool, 0, sizeof(void*) * bucket);
	}

	atstring::~atstring()
	{
		if (pool.pool)
		{
			free(pool.pool);
			pool.pool = 0;
		}

		while (mheader)
		{
			_mpool_unit* unit = mheader->next;
			free(mheader);
			mheader = unit;
		}
	}

	void* atstring::alloc(unsigned bytes)
	{
		if (!mheader || mheader->capacity - mheader->offset < bytes)
		{
			_mpool_unit* unit = (_mpool_unit*)malloc(sizeof(_mpool_unit) + _mpool_capacity);
			if (!unit)
				return 0;
#ifdef USE_CSTR_CHECK
			pool_addresses.insert((unsigned long)unit);
#endif
			unit->capacity = _mpool_capacity;
			unit->next = mheader;
			unit->offset = 0;
			unit->buffer = (unsigned char*)(unit + 1);
			mheader = unit;
		}
		void* ret = mheader->buffer + mheader->offset;
		mheader->offset += bytes;
		return ret;
	}

	c_str atstring::make_string(const char* pending)
	{
		unsigned len = 0;
		unsigned idx = hash(pending, len);
		idx %= pool.bucket;
		_unit* unit = pool.pool[idx];
		c_str str = 0;
		while (unit)
		{
			if (len == unit->len)
			{
				str = (c_str)&unit->str;
				if (0 == memcmp(pending, str, len * sizeof(char)))
					return str;
			}
			unit = unit->next;
		}

		unit = (_unit*)alloc(sizeof(_unit) + (len + 1) * sizeof(char) - sizeof(c_str));
		if (!unit)
			return 0;
		unit->len = len;
		unit->next = pool.pool[idx];
		pool.pool[idx] = unit;
		str = (c_str)&unit->str;
		memcpy((void*)str, pending, (len + 1) * sizeof(char));
		return str;
	}

	c_str atstring::make_string(const char* pending, unsigned len)
	{
		unsigned idx = hashl(pending, len);
		idx %= pool.bucket;
		_unit* unit = pool.pool[idx];
		c_str str = 0;
		while (unit)
		{
			if (len == unit->len)
			{
				str = (c_str)&unit->str;
				if (0 == memcmp(pending, str, len * sizeof(char)))
					return str;
			}
			unit = unit->next;
		}

		unit = (_unit*)alloc(sizeof(_unit) + (len + 1) * sizeof(char) - sizeof(c_str));
		if (!unit)
			return 0;
		unit->len = len;
		unit->next = pool.pool[idx];
		pool.pool[idx] = unit;
		str = (c_str)&unit->str;
		memcpy((void*)str, pending, len * sizeof(char));
		((char*)(&unit->str))[len] = 0;
		return str;
	}

	c_str atstring::cache(const char* origin)
	{
		c_str str = 0;
		if (!origin) return str;
		str = string(origin);
		if (str)
		{
			spin_lock(cache_lock);
			if (!cache_cstr) cache_cstr = new(std::nothrow) std::map<void *, c_str>();
			if (cache_cstr) cache_cstr->insert(std::make_pair((void*)origin, str));
			spin_unlock(cache_lock);
		}
		return str;
	}

	c_str atstring::string(const char* origin)
	{
		c_str str = 0;
		if (!origin) return str;
		if (cache_cstr)
		{
			spin_lock(cache_lock);
			std::map<void*, c_str>::iterator ptr = cache_cstr->find((void*)origin);
			if (ptr != cache_cstr->end()) str = ptr->second;
			spin_unlock(cache_lock);
			if (str) return str;
		}

		spin_lock(lock);
#ifdef USE_CSTR_CHECK
		if (in_memery_pool(origin))
			str = origin;
		else
#endif
			str = make_string(origin);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::string(const char* origin, unsigned len)
	{
		c_str str = 0;
		if (!origin) return str;
		spin_lock(lock);
#ifdef USE_CSTR_CHECK
		if (in_memery_pool(origin))
			str = origin;
		else
#endif
			str = make_string(origin, len);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::ustring(unsigned value)
	{
		c_str str = 0;
		char t[12];
		_ultoa_s(value, t, 10);
		spin_lock(lock);
#ifdef USE_CSTR_CHECK
		if (in_memery_pool(t))
			str = t;
		else
#endif
			str = make_string(t);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::u64string(uint64_t value)
	{
		c_str str = 0;
		char t[24];
		_ui64toa_s(value, t, _countof(t), 10);
		spin_lock(lock);
#ifdef USE_CSTR_CHECK
		if (in_memery_pool(t))
			str = t;
		else
#endif
			str = make_string(t);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::istring(int value)
	{
		c_str str = 0;
		char t[12];
		_ltoa_s(value, t, 10);
		spin_lock(lock);
#ifdef USE_CSTR_CHECK
		if (in_memery_pool(t))
			str = t;
		else
#endif
			str = make_string(t);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::i64string(int64_t value)
	{
		c_str str = 0;
		char t[24];
		_i64toa_s(value, t, _countof(t), 10);
		spin_lock(lock);
#ifdef USE_CSTR_CHECK
		if (in_memery_pool(t))
			str = t;
		else
#endif
			str = make_string(t);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::tolower(const char* origin)
	{
		c_str str = 0;
		if (!origin) return str;
		unsigned len = (unsigned)strlen(origin);
		spin_lock(lock);
		if (adapt_local((len + 1) * sizeof(char)))
		{
			s2l(local, origin, len);
			/*for (unsigned i = 0; i < len; ++i)
			local[i] = char_tolower(origin[i]);*/
			str = make_string(local);
		}
		spin_unlock(lock);
		return str;
	}

	c_str atstring::tolower(const char* origin, unsigned len)
	{
		c_str str = 0;
		if (!origin) return str;
		spin_lock(lock);
		if (adapt_local((len + 1) * sizeof(char)))
		{
			s2l(local, origin, len);
			/*for (unsigned i = 0; i < len; ++i)
			local[i] = char_tolower(origin[i]);*/
			str = make_string(local, len);
		}
		spin_unlock(lock);
		return str;
	}

	int atstring::adapt_local(unsigned bytes)
	{
		if (bytes > local_size)
		{
			if (local)
			{
				free(local);
				local = 0;
				local_size = 0;
			}

			local = (char*)malloc(bytes);
			if (!local)
				return false;
			local_size = bytes;
		}
		return true;
	}

#ifdef USE_CSTR_CHECK
	int atstring::in_memery_pool(const char* str)
	{
		static const unsigned long pool_size = sizeof(_mpool_unit) + _mpool_capacity;
		unsigned long address = (unsigned long)str;
		std::set<unsigned long>::iterator iter = pool_addresses.upper_bound(address);
		if (iter != pool_addresses.begin() && address < *(--iter) + pool_size)
			return true;
		return false;
	}
#endif

	c_str atstring::path(const char* origin)
	{
		c_str str = 0;
		if (!origin) return str;

		char nor[_normalize_path];
		normalize_path(nor, NELEMS(nor), origin);

		spin_lock(lock);
		str = make_string(nor);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::path(const char* origin, unsigned len)
	{
		c_str str = 0;
		if (!origin) return str;
		char nor[_normalize_path];
		len = normalize_path(nor, len, origin);

		spin_lock(lock);
		str = make_string(nor, len);
		spin_unlock(lock);
		return str;
	}

	c_str atstring::find(const char* pattern)
	{
		if (!pattern)
			return 0;
		unsigned len = 0;
		unsigned idx = hash(pattern, len);
		c_str ret = 0;
		spin_lock(lock);
		idx %= pool.bucket;
		_unit* unit = pool.pool[idx];
		while (unit)
		{
			if (len == unit->len)
			{
				c_str str = (c_str)&unit->str;
				if (0 == memcmp(pattern, str, len * sizeof(char)))
				{
					ret = str;
					break;
				}
			}
			unit = unit->next;
		}
		spin_unlock(lock);
		return ret;
	}

	static unsigned seed = 131;
	unsigned atstring::hash(const char* origin, unsigned& len)
	{
		unsigned hval = 0;
		len = 0;
		while (*origin)
		{
			hval = hval * seed + (*origin);
			origin++;
			len++;
		}
		return hval & 0x7fffffff;
	}

	unsigned atstring::hashl(const char* origin, unsigned len)
	{
		unsigned hval = 0;
		for (unsigned i = 0; i < len; ++i)
		{
			hval = hval * seed + origin[i];
		}
		return hval & 0x7fffffff;
	}
}
