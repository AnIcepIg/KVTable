#include "table.h"
#include "include/bittwiddle.h"
#include "serialization.h"

__delegates::__delegates()
	: _count(0)
	, _capacity(0)
	, _dlgts(0)
{

}

__delegates::~__delegates()
{
	if (_dlgts)
	{
		::free(_dlgts);
		_dlgts = 0;
	}
}

delegate3* __delegates::reg()
{
	if (_count == _capacity)
	{
		unsigned cnt = (_capacity >> 1);
		delegate3* ptr = (delegate3*)::malloc(sizeof(delegate3) * cnt);
		if (!ptr) return nullptr;
		memcpy(ptr, _dlgts, sizeof(delegate3) * _count);
		::free(_dlgts);
		_dlgts = ptr;
		_capacity = cnt;
	}
	delegate3* pdlgt = &_dlgts[_count];
	*pdlgt = 0;
	_count++;
	return pdlgt;
}

void __delegates::unreg(delegate3* pdlgt)
{
	for (unsigned i = 0; i < _count; ++i)
	{
		if (_dlgts[i] == *pdlgt)
		{
			for (unsigned j = i; j < _count - 1; ++j)
			{
				_dlgts[j] = _dlgts[j + 1];
			}
			_count--;
			break;
		}
	}
}

int __delegates::reserve(unsigned cnt)
{
	_dlgts = (delegate3*)::malloc(sizeof(delegate3) * cnt);
	if (!_dlgts) return false;
	_capacity = cnt;
	return true;
}

__delegates* __delegates::create()
{
	__delegates* dlgts = __delegatesalloc->alloc();
	if (!dlgts) return nullptr;
	dlgts = new(dlgts) __delegates;
	if (!dlgts->reserve(4))
	{
		__delegatesalloc->free(dlgts);
		dlgts = nullptr;
	}
	return dlgts;
}

void __delegates::destroy(__delegates* dlgts)
{
	if (dlgts)
	{
		dlgts->__delegates::~__delegates();
		__delegatesalloc->free(dlgts);
	}
}

__pair::__pair()
	: _mask(0)
	, _key(0)
	, _dlgts(0)
{
	_cm.bum = B_USM_NULL;
	_domain.x = _domain.y = 0;
}

__pair::~__pair()
{
	reset();
	if (_dlgts)
	{
		__delegates::destroy(_dlgts);
		_dlgts = nullptr;
	}
}

void __pair::reset()
{
	register etvaltype vt = (etvaltype)_cm.vt;
	switch (vt)
	{
	case etvt_table:
		if (!_cm.external)
			__table::destroy((__table*)_ptr);
		break;
	case etvt_string:
		if (_ptr) ::free(_ptr);
		break;
	case etvt_float4x4:
		if (_ptr) __pair::df4x4((Schema::float4x4*)_ptr);
		break;
	case etvt_userdata:
		if (_userdata.ptr) ::free(_userdata.ptr);
		break;
	}
	_domain.x = _domain.y = 0;
	_cm.external = 0;
	_cm.vt = etvt_reserved;
}

__pair* __pair::create(c_str name)
{
	b_usm_mask bum = B_USM_NULL;
	__pair* pair = __pairalloc->alloc(bum);
	if (!pair) return nullptr;
	pair->_cm.bum = bum;
	pair->_key = name;
	return pair;
}

void __pair::destroy(__pair* pair)
{
	__pairalloc->free(pair, pair->_cm.bum);
}

Schema::float4x4* __pair::cf4x4(float* f4x4)
{
	Schema::float4x4* ptr = __f4x4alloc->alloc();
	if (!ptr) return nullptr;
	ptr = new(ptr) Schema::float4x4(f4x4);
	return ptr;
}

void __pair::df4x4(Schema::float4x4* ptr)
{
	ptr->Schema::float4x4::~float4x4();
	__f4x4alloc->free(ptr);
}



__table::__table()
	: _mask(0)
	, _elements(0)
	, _count(0)
	, _capacity(0)
	, _dlgts(0)
{
	_cm.bum = B_USM_NULL;
	_cm.lexicographical = true;
}

__table::~__table()
{
	if (_dlgts)
	{
		__delegates::destroy(_dlgts);
		_dlgts = 0;
	}
	if (_elements)
	{
		for (unsigned i = 0; i < _count; ++i)
		{
			__pair* pair = _elements[i];
			assert(pair);
			__pairalloc->free(pair, pair->_cm.bum);
		}
		::free(_elements);
		_elements = nullptr;
		_count = 0;
		_capacity = 0;
	}
}

__table* __table::create()
{
	b_usm_mask bum = B_USM_NULL;
	__table* ptr = __taballoc->alloc(bum);
	if (!ptr) return nullptr;
	ptr->_cm.bum = bum;
	return ptr;
}

void __table::destroy(__table* ptab)
{
	__taballoc->free(ptab, ptab->_cm.bum);
}

int __table::reserve(unsigned cnt)
{
	return increase(cnt);
}

int __table::increase(unsigned cnt)
{
	cnt = bit_round_up_power2(cnt);
	if (_capacity > 2048u) cnt = _capacity + 2048u;
	uint8* buff = (uint8*)::malloc(sizeof(void*) * cnt);
	if (!buff) return false;
	register unsigned c = _count;
	if (_elements)
		memcpy(buff, _elements, sizeof(void*) * c);
	memset(buff + sizeof(void*) * c, 0, sizeof(void*) * (cnt - c));
	if (_elements)
		::free(_elements);
	_elements = (__pair**)buff;
	_capacity = cnt;
	return true;
}

int __table::erase_lexicographical(c_str name)
{
	int left = 0, right = (int)_count - 1, middle = 0;
	c_str result = 0;
	unsigned idx = _count;
	while (left <= right)
	{
		middle = (left + right) >> 1;
		result = (*(_elements + middle))->_key;
		if (result == name)
		{
			idx = (unsigned)middle;
			break;
		}
		if (result < name)
			left = middle + 1;
		else
			right = middle - 1;
	}
	if (idx == _count) return false;
	__pair::destroy(_elements[idx]);
	for (unsigned i = idx; i < _count - 1; ++i) _elements[i] = _elements[i + 1];
	_elements[_count - 1] = 0;
	_count--;
	return true;
}

int __table::erase_disorder(c_str name)
{
	unsigned idx = _count;
	for (unsigned i = 0; i < _count; ++i)
	{
		if (_elements[i]->_key == name)
		{
			idx = i;
			break;
		}
	}
	if (idx == _count) return false;
	__pair::destroy(_elements[idx]);
	for (unsigned i = idx; i < _count - 1; ++i) _elements[i] = _elements[i + 1];
	_elements[_count - 1] = 0;
	_count--;
	return true;
}

il int _cmp(const void* l, const void* r)
{
	__pair* lhs = *(__pair**)l;
	__pair* rhs = *(__pair**)r;
	if (lhs->_key < rhs->_key) return -1;
	else if (lhs->_key == rhs->_key) return 0;
	return 1;
}

int __table::reroder()
{
	if (_elements && _count > 0)
	{
		::qsort(_elements, _count, sizeof(__pair*), _cmp);
	}
	return true;
}

delegate3* __table::reg()
{
	if (!_dlgts)
	{
		_dlgts = __delegates::create();
		if (!_dlgts) return nullptr;
	}
	return _dlgts->reg();
}

void __table::unreg(delegate3* pdlgt)
{
	if (_dlgts && pdlgt)
	{
		_dlgts->unreg(pdlgt);
	}
}

delegate3* __table::reg(c_str name)
{
	__pair* pair = get(name);
	if (!pair) return nullptr;
	if (!pair->_dlgts)
	{
		pair->_dlgts = __delegates::create();
		if (!pair->_dlgts) return nullptr;
	}
	return pair->_dlgts->reg();
}

void __table::unreg(c_str name, delegate3* pdlgt)
{
	__pair* pair = get(name);
	if (pair && pair->_dlgts && pdlgt)
		pair->_dlgts->unreg(pdlgt);
}

__pair* __table::set(c_str name)
{
	if (_cm.lexicographical)
		return set_lexicographical(name);
	return set_disorder(name);
}

__pair* __table::set_lexicographical(c_str name)
{
	int left = 0, right = (int)_count - 1, middle = 0;
	c_str result = 0;
	while (left <= right)
	{
		middle = (left + right) >> 1;
		result = (*(_elements + middle))->_key;
		if (result == name)
		{
			return _elements[middle];
		}
		if (result < name)
			left = middle + 1;
		else
			right = middle - 1;
	}
	unsigned pos = (unsigned)middle;
	__pair* pair = __pair::create(name);
	if (!pair) return nullptr;
	if (_count == _capacity)
	{
		if (!increase(_capacity + 1))
		{
			__pair::destroy(pair);
			return nullptr;
		}
	}
	if (_count > 0 && name > _elements[pos]->_key)
	{
		for (unsigned i = _count; i > pos + 1; --i) _elements[i] = _elements[i - 1];
		_elements[pos + 1] = pair;
	}
	else
	{
		for (unsigned i = _count; i > pos; --i) _elements[i] = _elements[i - 1];
		_elements[pos] = pair;
	}
	_count++;
	return pair;
}

__pair* __table::set_disorder(c_str name)
{
	for (unsigned i = 0; i < _count; ++i)
	{
		if (name == _elements[i]->_key) return _elements[i];
	}
	__pair* pair = __pair::create(name);
	if (!pair) return nullptr;
	if (_count == _capacity)
	{
		if (!increase(_capacity + 1))
		{
			__pair::destroy(pair);
			return nullptr;
		}
	}
	_elements[_count] = pair;
	_count++;
	return pair;
}

unsigned __table::getIdx(c_str name)
{
	if (_cm.lexicographical)
		return getIdx_lexicographical(name);
	return getIdx_disorder(name);
}

unsigned __table::getIdx_lexicographical(c_str name)
{
	int left = 0, right = (int)_count - 1, middle = 0;
	c_str result = 0;
	while (left <= right)
	{
		middle = (left + right) >> 1;
		result = (*(_elements + middle))->_key;
		if (result == name)
		{
			return (unsigned)middle;
		}
		if (result < name)
			left = middle + 1;
		else
			right = middle - 1;
	}
	return invalid_table_iterator;
}

unsigned __table::getIdx_disorder(c_str name)
{
	for (unsigned i = 0; i < _count; ++i)
	{
		if (name == _elements[i]->_key)
			return i;
	}
	return invalid_table_iterator;
}

__pair* __table::get(c_str name)
{
	if (_cm.lexicographical)
		return get_lexicographical(name);
	return get_disorder(name);
}

__pair* __table::get_lexicographical(c_str name)
{
	int left = 0, right = (int)_count - 1, middle = 0;
	c_str result = 0;
	while (left <= right)
	{
		middle = (left + right) >> 1;
		result = (*(_elements + middle))->_key;
		if (result == name)
		{
			return _elements[middle];
		}
		if (result < name)
			left = middle + 1;
		else
			right = middle - 1;
	}
	return nullptr;
}

__pair* __table::get_disorder(c_str name)
{
	for (unsigned i = 0; i < _count; ++i)
	{
		if (name == _elements[i]->_key)
			return _elements[i];
	}
	return nullptr;
}

#define _callback(ops)		if (callback) {	\
	if (trigger(ops, pair) == etr_abort)	\
		return 0;							\
}

__table* __table::set(c_str name, int callback)
{
	__table* ptab = invalid_table_handle;
	int result = false;
	do {
		ptab = __table::create();
		if (!ptab) break;
		if (!ptab->reserve(4)) break;
		__pair* pair = set(name);
		if (!pair) break;
		pair->reset();
		pair->_ptr = ptab;
		pair->_cm.vt = etvt_table;
		_callback(ets_modify);
		result = true;
	} while (0);
	if (!result && ptab) { __table::destroy(ptab); ptab = invalid_table_handle; }
	return ptab;
}

int __table::set(c_str name, __table* ptab, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_ptr = ptab;
	pair->_cm.external = true;
	pair->_cm.vt = etvt_table;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, int val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_int = val;
	pair->_cm.vt = etvt_int;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, float val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_flt = val;
	pair->_cm.vt = etvt_float;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, double val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_dbl = val;
	pair->_cm.vt = etvt_double;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, __int64 val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_i64 = val;
	pair->_cm.vt = etvt_int64;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, const char* val, int callback)
{
	int result = false;
	void* ptr = nullptr;
	do {
		unsigned len = (unsigned)strlen(val);
		ptr = malloc(len + 1);
		if (!ptr) break;
		memcpy(ptr, val, len + 1);
		__pair* pair = set(name);
		if (!pair) break;
		pair->reset();
		pair->_ptr = ptr;
		pair->_cm.vt = etvt_string;
		_callback(ets_modify);
		result = true;
	} while (0);
	if (!result && ptr) ::free(ptr);
	return result;
}

int __table::set(c_str name, const char* val, unsigned len, int callback)
{
	int result = false;
	char* ptr = nullptr;
	do {
		ptr = (char*)malloc(len + 1);
		if (!ptr) break;
		memcpy(ptr, val, len);
		ptr[len] = 0;
		__pair* pair = set(name);
		if (!pair) break;
		pair->reset();
		pair->_ptr = ptr;
		pair->_cm.vt = etvt_string;
		_callback(ets_modify);
		result = true;
	} while (0);
	if (!result && ptr) ::free(ptr);
	return result;
}

int __table::setCstr(c_str name, c_str val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_str = val;
	pair->_cm.vt = etvt_cstr;
	_callback(ets_modify);
	return true;
}

int __table::setPtr(c_str name, void* val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_ptr = val;
	pair->_cm.vt = etvt_ptr;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, void* ud, unsigned size, int callback)
{
	int result = false;
	void* ptr = nullptr;
	do {
		ptr = malloc(size);
		if (!ptr) break;
		memcpy(ptr, ud, size);
		__pair* pair = set(name);
		if (!pair) break;;
		pair->reset();
		pair->_userdata.ptr = ptr;
		pair->_userdata.size = size;
		pair->_cm.vt = etvt_userdata;
		_callback(ets_modify);
		result = true;
	} while (0);
	if (!result && ptr) ::free(ptr);
	return result;
}

int __table::set(c_str name, uint val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_uint = val;
	pair->_cm.vt = etvt_uint;
	_callback(ets_modify);
	return true;
}

int __table::set(c_str name, uint64 val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_u64 = val;
	pair->_cm.vt = etvt_uint64;
	_callback(ets_modify);
	return true;
}

int __table::setf2(c_str name, float* val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_f2 = Schema::float2(val);
	pair->_cm.vt = etvt_float2;
	_callback(ets_modify);
	return true;
}

int __table::setf3(c_str name, float* val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_f3 = Schema::float3(val);
	pair->_cm.vt = etvt_float3;
	_callback(ets_modify);
	return true;
}

int __table::setf4(c_str name, float* val, int callback)
{
	__pair* pair = set(name);
	if (!pair) return false;
	pair->reset();
	pair->_f4 = Schema::float4(val);
	pair->_cm.vt = etvt_float4;
	_callback(ets_modify);
	return true;
}

int __table::setf4x4(c_str name, float* val, int callback)
{
	Schema::float4x4* ptr = nullptr;
	int result = false;
	do {
		ptr = __pair::cf4x4(val);
		if (!ptr) break;
		__pair* pair = set(name);
		if (!pair) break;
		pair->reset();
		pair->_ptr = ptr;
		pair->_cm.vt = etvt_float4x4;
		_callback(ets_modify);
		result = true;
	} while (0);
	if (!result && ptr) __pair::df4x4(ptr);
	return result;
}

int __table::erase(c_str name, int callback)
{
	int flag = false;
	if (_cm.lexicographical) flag = erase_lexicographical(name);
	else flag = erase_disorder(name);
	if (flag && callback) trigger(ets_erase, name);
	return flag;
}

int __table::reset(c_str name, int callback)
{
	__pair* pair = get(name);
	if (!pair) return false;
	pair->reset();
	if (callback) trigger(ets_reset, pair);
	return true;
}

int __table::trigger(etopstatus ops, c_str name)
{
	int result = etr_none;
	if (!_dlgts) return etr_none;
	Schema::Table tab;
	tab._htab = this;
	for (unsigned i = 0; i < _dlgts->_count; ++i)
	{
		delegate3& dlgt = _dlgts->_dlgts[i];
		result = dlgt(ops, tab, name);
		switch (result)
		{
		case etr_break:
		case etr_abort:
			return result;
		}
	}
	return etr_processed;
}

int __table::trigger(etopstatus ops, __pair* pair)
{
	int result = etr_none;
	Schema::Table tab;
	tab._htab = this;
	for (unsigned i = 0; _dlgts && i < _dlgts->_count; ++i)
	{
		delegate3& dlgt = _dlgts->_dlgts[i];
		result = dlgt(ops, tab, pair->_key);
		switch (result)
		{
		case etr_break:
		case etr_abort:
			return result;
		}
	}
	if (!pair->_dlgts) return etr_processed;
	for (unsigned i = 0; i < pair->_dlgts->_count; ++i)
	{
		delegate3& dlgt = pair->_dlgts->_dlgts[i];
		result = dlgt(ops, tab, pair->_key);
		switch (result)
		{
		case etr_break:
		case etr_abort:
			return result;
		}
	}
	return etr_processed;
}

extern "C"
{
#define _convert()		__table* ptab = (__table*)htab
#define _common_set()	_convert(); return ptab->set(name, val, callback)
#define _common_get()	_convert(); __pair* pair = ptab->get(name);

	dxt htable table_create()
	{
		__table* ptab = __table::create();
		if (!ptab) return invalid_table_handle;
		if (!ptab->reserve(4))
		{
			__table::destroy(ptab);
			return invalid_table_handle;
		}
		return (htable)ptab;
	}
	dxt void table_destroy(htable htab) { _convert(); __table::destroy(ptab); }
	dxt htable table_set(htable htab, c_str name, int callback) { _convert(); return (htable)ptab->set(name, callback); }
	dxt int	table_set_table(htable htab, c_str name, htable hchild, int callback) { _convert(); return ptab->set(name, (__table*)hchild, callback); }
	dxt int	table_set_integer(htable htab, c_str name, int val, int callback) { _common_set(); }
	dxt int	table_set_float(htable htab, c_str name, float val, int callback) { _common_set(); }
	dxt int	table_set_double(htable htab, c_str name, double val, int callback) { _common_set(); }
	dxt int	table_set_int64(htable htab, c_str name, __int64 val, int callback) { _common_set(); }
	dxt int	table_set_string(htable htab, c_str name, const char* val, int callback) { _common_set(); }
	dxt int	table_set_stringl(htable htab, c_str name, const char* val, unsigned len, int callback) { _convert(); return ptab->set(name, val, len, callback); }
	dxt int	table_set_cstr(htable htab, c_str name, c_str val, int callback) { _convert(); return ptab->setCstr(name, val, callback); }
	dxt int	table_set_ptr(htable htab, c_str name, void* val, int callback) { _convert(); return ptab->setPtr(name, val, callback); }
	dxt int	table_set_userdata(htable htab, c_str name, void* ud, unsigned size, int callback) { _convert(); return ptab->set(name, ud, size, callback); }
	dxt int	table_set_uint(htable htab, c_str name, uint val, int callback) { _common_set(); }
	dxt int	table_set_uint64(htable htab, c_str name, uint64 val, int callback) { _common_set(); }
	dxt int	table_set_float2(htable htab, c_str name, float* val, int callback) { _convert(); return ptab->setf2(name, val, callback); }
	dxt int	table_set_float3(htable htab, c_str name, float* val, int callback) { _convert(); return ptab->setf3(name, val, callback); }
	dxt int	table_set_float4(htable htab, c_str name, float* val, int callback) { _convert(); return ptab->setf4(name, val, callback); }
	dxt int	table_set_float4x4(htable htab, c_str name, float* val, int callback) { _convert(); return ptab->setf4x4(name, val, callback); }

	dxt htable table_get(htable htab, c_str name)
	{
		_common_get();
		if (!pair || pair->_cm.vt != etvt_table) return 0;
		return (htable)pair->_ptr;
	}
	dxt int	table_get_integer(htable htab, c_str name, int def) { _common_get(); if (!pair) return def; return pair->_int; }
	dxt float table_get_float(htable htab, c_str name, float def) { _common_get(); if (!pair) return def; return pair->_flt; }
	dxt double table_get_double(htable htab, c_str name, double def) { _common_get(); if (!pair) return def; return pair->_dbl; }
	dxt int64 table_get_int64(htable htab, c_str name, int64 def) { _common_get(); if (!pair) return def; return pair->_i64; }
	dxt const char* table_get_string(htable htab, c_str name)
	{
		_common_get();
		if (!pair || (pair->_cm.vt != etvt_string && pair->_cm.vt != etvt_cstr)) return nullptr;
		return (const char*)pair->_ptr;
	}
	dxt c_str table_get_cstr(htable htab, c_str name)
	{
		_common_get();
		if (!pair || (pair->_cm.vt != etvt_cstr && pair->_cm.vt != etvt_string)) return nullptr;
		return pair->_str;
	}
	dxt void* table_get_ptr(htable htab, c_str name)
	{
		_common_get();
		if (!pair || pair->_cm.vt != etvt_ptr) return nullptr;
		return pair->_ptr;
	}
	dxt int	table_get_userdata(htable htab, c_str name, void* ptr, unsigned size)
	{
		_common_get();
		if (!pair || pair->_cm.vt != etvt_userdata) return 0;
		size = size > pair->_userdata.size ? pair->_userdata.size : size;
		memcpy(ptr, pair->_userdata.ptr, size);
		return true;
	}
	dxt uint table_get_uint(htable htab, c_str name, uint def) { _common_get(); if (!pair) return def; return pair->_uint; }
	dxt uint64 table_get_uint64(htable htab, c_str name, uint64 def) { _common_get(); if (!pair) return def; return pair->_u64; }
	dxt float* table_get_float2(htable htab, c_str name)
	{
		_common_get();
		if (!pair) return nullptr;
		unsigned vt = pair->_cm.vt;
		switch (vt)
		{
		case etvt_float2: return (float*)&pair->_f2;
		case etvt_float3: return (float*)&pair->_f3;
		case etvt_float4: return (float*)&pair->_f4;
		case etvt_float4x4: return (float*)pair->_ptr;
		}
		return nullptr;
	}
	dxt float* table_get_float3(htable htab, c_str name)
	{
		_common_get();
		if (!pair) return nullptr;
		unsigned vt = pair->_cm.vt;
		switch (vt)
		{
		case etvt_float3: return (float*)&pair->_f3;
		case etvt_float4: return (float*)&pair->_f4;
		case etvt_float4x4: return (float*)pair->_ptr;
		}
		return nullptr;
	}
	dxt float* table_get_float4(htable htab, c_str name)
	{
		_common_get();
		if (!pair) return nullptr;
		unsigned vt = pair->_cm.vt;
		switch (vt)
		{
		case etvt_float4: return (float*)&pair->_f4;
		case etvt_float4x4: return (float*)pair->_ptr;
		}
		return nullptr;
	}
	dxt float* table_get_float4x4(htable htab, c_str name)
	{
		_common_get();
		if (!pair || pair->_cm.vt != etvt_float4x4) return nullptr;
		return (float*)pair->_ptr;
	}

	dxt int	table_reserve(htable htab, c_str name) { _convert(); if (ptab->set(name)) return true; return false; }
	dxt int	table_clear(htable htab, int callback)
	{
		_convert();
		for (unsigned i = 0; i < ptab->_count; ++i)
		{
			__pair::destroy(ptab->_elements[i]);
			ptab->_elements[i] = 0;
		}
		ptab->_count = 0;
		ptab->trigger(ets_clear, (__pair*)0);
		return true;
	}
	dxt int	table_erase(htable htab, c_str name, int callback) { _convert(); return ptab->erase(name, callback); }
	dxt int	table_reset(htable htab, c_str name, int callback) { _convert(); return ptab->reset(name, callback); }

	dxt int	table_lexicographical(htable htab, int enable)
	{
		_convert();
		int prev = (int)ptab->_cm.lexicographical;
		if ((enable && prev) || (!enable && !prev)) return true;
		unsigned flag = false;
		if (!prev)
		{
			if (!ptab->reroder()) return false;
			flag = true;
		}
		ptab->_cm.lexicographical = flag;
		return true;
	}

	dxt htable	table_read(const char* filename)
	{
		etfiletype ftyp = (etfiletype)tab_peek_type(filename);
		switch (ftyp)
		{
		case etft_unknown:
			return nullptr;
		case etft_text:
		{
			ttrd td = tabtxt_read(filename);
			if (td._addr == nullptr) return nullptr;
			__table* ptab = tabtxt_to(td);
			tabtxt_free_ttrd(td);
			return ptab;
		}
		case etft_binary:
		{
			ttrd td = tabbin_read(filename);
			if (td._addr == nullptr) return nullptr;
			__table* ptab = tabbin_to(td);
			tabbin_free_ttrd(td);
			return ptab;
		}
		}
		return nullptr;
	}
	dxt int		table_save(htable htab, const char* filename, etfiletype ftyp, int pretty)
	{
		_convert();
		switch (ftyp)
		{
		case etft_unknown:
			return false;
		case etft_text:
		{
			htabtxt htt = tabtxt_from(ptab, pretty);
			if (!htt) return false;
			if (!tabtxt_save(htt, filename))
			{
				tabtxt_free(htt);
				return false;
			}
			tabtxt_free(htt);
		}
			break;
		case etft_binary:
		{
			htabbin htb = tabbin_from(ptab);
			if (!htb) return false;
			if (!tabbin_save(htb, filename))
			{
				tabbin_free(htb);
				return false;
			}
			tabbin_free(htb);
		}
			break;
		}
		return true;
	}

	dxt tabit table_begin(htable htab)
	{
		_convert();
		if (ptab->_count == 0) return invalid_table_iterator;
		return 0;
	}
	dxt tabit table_next(htable htab, tabit it)
	{
		_convert();
		it++;
		if (it >= ptab->_count) return invalid_table_iterator;
		return it;
	}
	dxt tabit table_get_element(htable htab, c_str name) { _convert(); return ptab->getIdx(name); }

	dxt delegate3* table_reg_global(htable htab) { _convert(); return ptab->reg(); }
	dxt delegate3* table_reg_element(htable htab, c_str name) { _convert(); return ptab->reg(name); }
	dxt void table_unreg_global(htable htab, delegate3* pdlgt) { _convert(); ptab->unreg(pdlgt); }
	dxt void table_unreg_element(htable htab, c_str name, delegate3* pdlgt) { _convert(); ptab->unreg(name, pdlgt); }


#define _get_element()	_convert(); __pair* pair = ptab->_elements[it]
	dxt c_str tabit_get_key(htable htab, tabit it) { _get_element(); return pair->_key; }
	dxt etvaltype tabit_get_type(htable htab, tabit it) { _get_element(); return (etvaltype)pair->_cm.vt; }
	dxt htable tabit_get_table(htable htab, tabit it) { _get_element(); return (htable)pair->_ptr; }
	dxt int	tabit_get_integer(htable htab, tabit it) { _get_element(); return pair->_int; }
	dxt float tabit_get_float(htable htab, tabit it) { _get_element(); return pair->_flt; }
	dxt double tabit_get_double(htable htab, tabit it) { _get_element(); return pair->_dbl; }
	dxt int64 tabit_get_int64(htable htab, tabit it) { _get_element(); return pair->_i64; }
	dxt const char* tabit_get_string(htable htab, tabit it) { _get_element(); return (const char*)pair->_ptr; }
	dxt c_str tabit_get_cstr(htable htab, tabit it) { _get_element(); return pair->_str; }
	dxt void* tabit_get_ptr(htable htab, tabit it) { _get_element(); return pair->_ptr; }
	dxt void tabit_get_userdata(htable htab, tabit it, void** pptr, unsigned* psize) { _get_element(); *pptr = pair->_userdata.ptr; *psize = pair->_userdata.size; }
	dxt uint tabit_get_uint(htable htab, tabit it) { _get_element(); return pair->_uint; }
	dxt uint64 tabit_get_uint64(htable htab, tabit it) { _get_element(); return pair->_u64; }
	dxt float* tabit_get_float2(htable htab, tabit it) { _get_element(); return pair->_f2.m; }
	dxt float* tabit_get_float3(htable htab, tabit it) { _get_element(); return pair->_f3.m; }
	dxt float* tabit_get_float4(htable htab, tabit it) { _get_element(); return pair->_f4.m; }
	dxt float* tabit_get_float4x4(htable htab, tabit it) { _get_element(); return (float*)pair->_ptr; }

	dxt int file_link(Schema::file_open_read fnor
		, Schema::file_open_write fnow
		, Schema::file_close fnclose
		, Schema::file_length fnlength
		, Schema::file_read fnr
		, Schema::file_write fnw)
	{
		if (fnor) _fopenread = fnor;
		if (fnow) _fopenwrite = fnow;
		if (fnclose) _fclose = fnclose;
		if (fnlength) _flength = fnlength;
		if (fnr) _fread = fnr;
		if (fnw) _fwrite = fnw;
		return true;
	}
}
