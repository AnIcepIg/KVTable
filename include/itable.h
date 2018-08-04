#pragma once

#include "utility.h"
#include "icstr.h"
#include "FastDelegate.h"

extern "C" {
#define invalid_table_handle	0
#define invalid_table_iterator	0xffffffff
	typedef void*		htable;
	typedef unsigned	tabit;

	enum etcallback
	{
		etc_disable = 0,
		etc_enable
	};

	enum etvaltype
	{
		etvt_reserved = 0,
		etvt_ptr,
		etvt_int,
		etvt_uint,
		etvt_int64,
		etvt_uint64,
		etvt_float,
		etvt_double,
		etvt_float2,
		etvt_float3,
		etvt_float4,
		etvt_cstr,

		etvt_reference,

		etvt_table,
		etvt_string,
		etvt_float4x4,
		etvt_userdata,
	};

#define table_lexicographical_mask	0x01
#define table_get_mask(tab)	(*(unsigned*)tab)

	inline int table_lexicographical_order(htable htab)
	{
		if (!htab) return false;
		unsigned mask = table_get_mask(htab);
		return mask & table_lexicographical_mask ? true : false;
	}

	enum etopstatus
	{
		ets_modify = 0,
		ets_erase,
		ets_clear,
		ets_reset,

		ets_count
	};

	enum etopresult
	{
		etr_none = 0,
		etr_processed,
		etr_break,
		etr_abort,
	};

	namespace Schema { struct Table; };
	/*
	 *	table callback delegate, callback can be global functions, class member functions, virtual functions
	 *	format: 
	 *		int OnCallback(etopstatus ops, xUtility::Table tab, c_str key);
	 *		public/protected/private: int Class::OnCallback(etopstatus ops, xUtility::Table tab, c_str key);
	 *		virtual int Interface::OnCallback(etopstatus ops, xUtility::Table tab, c_str key);
	 *	if a callback function returns etr_break, table will break the callback chain
	 *		this can stop calling next callbacks
	 *	if a callback function returns etr_abort, table will immediately return and do nothing
	 *		if you delete this table in the callback, make sure return this value
	 */
	typedef FastDelegate3<etopstatus, Schema::Table, c_str, int>	delegate3;

#define dxt		__declspec(dllexport)
#define il		inline

	dxt htable	table_create();
	dxt void	table_destroy(htable htab);

	dxt htable	table_set(htable htab, c_str name, int callback);
	dxt int		table_set_table(htable htab, c_str name, htable hchild, int callback);
	dxt int		table_set_integer(htable htab, c_str name, int val, int callback);
	dxt int		table_set_float(htable htab, c_str name, float val, int callback);
	dxt int		table_set_double(htable htab, c_str name, double val, int callback);
	dxt int		table_set_int64(htable htab, c_str name, __int64 val, int callback);
	dxt int		table_set_string(htable htab, c_str name, const char* val, int callback);
	dxt int		table_set_stringl(htable htab, c_str name, const char* val, unsigned len, int callback);
	dxt int		table_set_cstr(htable htab, c_str name, c_str val, int callback);
	dxt int		table_set_ptr(htable htab, c_str name, void* ptr, int callback);
	dxt int		table_set_userdata(htable htab, c_str name, void* ud, unsigned size, int callback);
	dxt int		table_set_uint(htable htab, c_str name, uint val, int callback);
	dxt int		table_set_uint64(htable htab, c_str name, uint64 val, int callback);
	dxt int		table_set_float2(htable htab, c_str name, float* val, int callback);
	dxt int		table_set_float3(htable htab, c_str name, float* val, int callback);
	dxt int		table_set_float4(htable htab, c_str name, float* val, int callback);
	dxt int		table_set_float4x4(htable htab, c_str name, float* val, int callback);

	/*
	 *	TODO: begin/end massive insert to an ordered table
	 *	begin -> insert elements, end -> sort
	 *	'set' in between pair begin/end, no callback will be triggered
	 */
	dxt int		table_begin_scale(htable htab);
	dxt int		table_end_scale(htable htab);

	dxt htable		table_get(htable htab, c_str name);
	dxt int			table_get_integer(htable htab, c_str name, int def);
	dxt float		table_get_float(htable htab, c_str name, float def);
	dxt double		table_get_double(htable htab, c_str name, double def);
	dxt int64		table_get_int64(htable htab, c_str name, int64 def);
	dxt const char* table_get_string(htable htab, c_str name);
	dxt c_str		table_get_cstr(htable htab, c_str name);
	dxt void*		table_get_ptr(htable htab, c_str name);
	dxt int			table_get_userdata(htable htab, c_str name, void* ptr, unsigned size);
	dxt uint		table_get_uint(htable htab, c_str name, uint def);
	dxt uint64		table_get_uint64(htable htab, c_str name, uint64 def);
	dxt float*		table_get_float2(htable htab, c_str name);
	dxt float*		table_get_float3(htable htab, c_str name);
	dxt float*		table_get_float4(htable htab, c_str name);
	dxt float*		table_get_float4x4(htable htab, c_str name);

	dxt int			table_reserve(htable htab, c_str name);	// reserve an element with empty value, if key is already existed, nothing happens
	dxt int			table_clear(htable htab, int callback);
	dxt int			table_erase(htable htab, c_str name, int callback);
	dxt int			table_reset(htable htab, c_str name, int callback);	// clear value but keep key

	/*
	 *	whether table arranges as lexicographical order, default is enable
	 *	if you have tons of elements to insert, consider as efficient, disable->insert->enable
	 */
	dxt int			table_lexicographical(htable htab, int enable);

	/*
	 *	table <=> json format serialization
	 */
	dxt htable		table_read(const char* filename);
	dxt int			table_save(htable htab, const char* filename, int pretty);

	dxt tabit		table_begin(htable htab);
	dxt tabit		table_next(htable htab, tabit it);
#define				table_end	invalid_table_iterator

	dxt delegate3*	table_reg_global(htable htab);
	dxt delegate3*	table_reg_element(htable htab, c_str name);
	dxt void		table_unreg_global(htable htab, delegate3* pdlgt);
	dxt void		table_unreg_element(htable htab, c_str name, delegate3* pdlgt);


	dxt c_str		tabit_get_key(htable htab, tabit it);
	dxt etvaltype	tabit_get_type(htable htab, tabit it);
	dxt htable		tabit_get_table(htable htab, tabit it);
	dxt int			tabit_get_integer(htable htab, tabit it);
	dxt float		tabit_get_float(htable htab, tabit it);
	dxt double		tabit_get_double(htable htab, tabit it);
	dxt int64		tabit_get_int64(htable htab, tabit it);
	dxt const char* tabit_get_string(htable htab, tabit it);
	dxt c_str		tabit_get_cstr(htable htab, tabit it);
	dxt void*		tabit_get_ptr(htable htab, tabit it);
	dxt void		tabit_get_userdata(htable htab, tabit it, void** pptr, unsigned* psize);
	dxt uint		tabit_get_uint(htable htab, tabit it);
	dxt uint64		tabit_get_uint64(htable htab, tabit it);
	dxt float*		tabit_get_float2(htable htab, tabit it);
	dxt float*		tabit_get_float3(htable htab, tabit it);
	dxt float*		tabit_get_float4(htable htab, tabit it);
	dxt float*		tabit_get_float4x4(htable htab, tabit it);
}

namespace Schema
{
#define __it_empty_return(v)	if (_it == invalid_table_iterator) return v
#define __empty_return(v)		if (_htab == invalid_table_handle) return v

	struct Iterator;
	struct Table
	{
		htable _htab;
		il Table() : _htab(invalid_table_handle) {}
		il Table operator[](c_str key) const { Table tab; __empty_return(tab); tab._htab = table_get(_htab, key); return tab; }
		il Table& operator=(const htable tab) { _htab = tab; return *this; }
		il bool operator!() const { return _htab == invalid_table_handle; }
		il bool isEmpty() const { return _htab == invalid_table_handle; }
		il bool isNull() const { return _htab == invalid_table_handle; }

		il void destroy() { if (_htab) table_destroy(_htab); _htab = invalid_table_handle; }

		il Table set(c_str name, etcallback cb = etc_enable) { Table tab; __empty_return(tab); tab._htab = table_set(_htab, name, cb); return tab; }
		il Table setc(const char* name, etcallback cb = etc_enable) { return set(cstr(name), cb); }
		il int set(c_str name, const Table& tab, etcallback cb = etc_enable) { __empty_return(0); return table_set_table(_htab, name, tab._htab, cb); }
		il int setc(const char* name, const Table& tab, etcallback cb = etc_enable) { return set(cstr(name), tab, cb); }
		il int set(c_str name, int val, etcallback cb = etc_enable) { __empty_return(0); return table_set_integer(_htab, name, val, cb); }
		il int setc(const char* name, int val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, float val, etcallback cb = etc_enable) { __empty_return(0); return table_set_float(_htab, name, val, cb); }
		il int setc(const char* name, float val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, double val, etcallback cb = etc_enable) { __empty_return(0); return table_set_double(_htab, name, val, cb); }
		il int setc(const char* name, double val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, int64 val, etcallback cb = etc_enable) { __empty_return(0); return table_set_int64(_htab, name, val, cb); }
		il int setc(const char* name, int64 val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int setStr(c_str name, const char* val, etcallback cb = etc_enable) { __empty_return(0); return table_set_string(_htab, name, val, cb); }
		il int setcStr(const char* name, const char* val, etcallback cb = etc_enable) { return setStr(cstr(name), val, cb); }
		il int set(c_str name, const char* val, unsigned len, etcallback cb = etc_enable) { __empty_return(0); return table_set_stringl(_htab, name, val, len, cb); }
		il int setc(const char* name, const char* val, unsigned len, etcallback cb = etc_enable) { return set(cstr(name), val, len, cb); }
		il int setCstr(c_str name, c_str val, etcallback cb = etc_enable) { __empty_return(0); return table_set_cstr(_htab, name, val, cb); }
		il int setcCstr(const char* name, c_str val, etcallback cb = etc_enable) { return setCstr(cstr(name), val, cb); }
		il int set2Cstr(c_str name, const char* val, etcallback cb = etc_enable) { __empty_return(0); c_str str = cstr_string(val); return setCstr(name, str, cb); }
		il int setc2Cstr(const char* name, const char* val, etcallback cb = etc_enable) { return set2Cstr(cstr(name), val, cb); }
		il int setPtr(c_str name, void* val, etcallback cb = etc_enable) { __empty_return(0); return table_set_ptr(_htab, name, val, cb); }
		il int setcPtr(const char* name, void* val, etcallback cb = etc_enable) { return setPtr(cstr(name), val, cb); }
		il int setUserdata(c_str name, void* val, unsigned size, etcallback cb = etc_enable) { __empty_return(0); return table_set_userdata(_htab, name, val, size, cb); }
		il int setcUserdata(const char* name, void* val, unsigned size, etcallback cb = etc_enable) { return setUserdata(cstr(name), val, size, cb); }
		il int set(c_str name, uint val, etcallback cb = etc_enable) { __empty_return(0); return table_set_uint(_htab, name, val, cb); }
		il int setc(const char* name, uint val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, uint64 val, etcallback cb = etc_enable) { __empty_return(0); return table_set_uint64(_htab, name, val, cb); }
		il int setc(const char* name, uint64 val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, float2& val, etcallback cb = etc_enable) { __empty_return(0); return table_set_float2(_htab, name, val.m, cb); }
		il int setc(const char* name, float2& val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, float3& val, etcallback cb = etc_enable) { __empty_return(0); return table_set_float3(_htab, name, val.m, cb); }
		il int setc(const char* name, float3& val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, float4& val, etcallback cb = etc_enable) { __empty_return(0); return table_set_float4(_htab, name, val.m, cb); }
		il int setc(const char* name, float4& val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }
		il int set(c_str name, float4x4& val, etcallback cb = etc_enable) { __empty_return(0); return table_set_float4x4(_htab, name, val.m, cb); }
		il int setc(const char* name, float4x4& val, etcallback cb = etc_enable) { return set(cstr(name), val, cb); }

		il Table	get(c_str name) const { Table tab; __empty_return(tab); tab._htab = table_get(_htab, name); return tab; }
		il Table	getc(const char* name) const { return get(cstr(name)); }
		il int		get(c_str name, int def) const { __empty_return(def); return table_get_integer(_htab, name, def); }
		il int		getc(const char* name, int def) const { get(cstr(name), def); }
		il int		getInteger(c_str name) const { return get(name, 0); }
		il int		getcInteger(const char* name) const { return getInteger(cstr(name)); }
		il float	get(c_str name, float def) const { __empty_return(def); return table_get_float(_htab, name, def); }
		il float	getc(const char* name, float def) const { return get(cstr(name), def); }
		il float	getFloat(c_str name) const { return get(name, 0.f); }
		il float	getcFloat(const char* name) const { return getFloat(cstr(name)); }
		il double	get(c_str name, double def) const { __empty_return(def); return table_get_double(_htab, name, def); }
		il double	getc(const char* name, double def) const { return get(cstr(name), def); }
		il double	getDouble(c_str name) const { return get(name, 0.0); }
		il double	getcDouble(const char* name) const { return getDouble(cstr(name)); }
		il int64	get(c_str name, __int64 def) const { __empty_return(def); return table_get_int64(_htab, name, def); }
		il int64	getc(const char* name, __int64 def) const { return get(cstr(name), def); }
		il int64	getInt64(c_str name) const { return get(name, 0ll); }
		il int64	getcInt64(const char* name) const { return getInt64(cstr(name)); }
		il const char* getStr(c_str name) const { __empty_return(0); return table_get_string(_htab, name); }
		il const char* getcStr(const char* name) const { return getStr(cstr(name)); }
		il c_str	getCstr(c_str name) const { __empty_return(0); return table_get_cstr(_htab, name); }
		il c_str	getcCstr(const char* name) const { return getCstr(cstr(name)); }
		il void*	getPtr(c_str name) const { __empty_return(0); return table_get_ptr(_htab, name); }
		il void*	getcPtr(const char* name) const { return getPtr(cstr(name)); }
		il int		getUserdata(c_str name, void* ptr, unsigned size) const { __empty_return(0); return table_get_userdata(_htab, name, ptr, size); }
		il int		getcUserdata(const char* name, void* ptr, unsigned size) const { return getUserdata(cstr(name), ptr, size); }
		il uint		get(c_str name, uint def) const { __empty_return(0); return table_get_uint(_htab, name, def); }
		il uint		getc(const char* name, uint def) const { return get(cstr(name), def); }
		il uint		getUint(c_str name) const { return get(name, 0u); }
		il uint		getcUnit(const char* name) const { return getUint(cstr(name)); }
		il uint64	get(c_str name, uint64 def) const { __empty_return(0); return table_get_uint64(_htab, name, def); }
		il uint64	getc(const char* name, uint64 def) const { return get(cstr(name), def); }
		il uint64	getUint64(c_str name) const { return get(name, 0ull); }
		il uint64	getcUint64(const char* name) const { return getUint64(cstr(name)); }
		il int		get(c_str name, float2& val) const { __empty_return(0); float* ret = table_get_float2(_htab, name); if (!ret) return 0; val = float2(val); return 1; }
		il int		getc(const char* name, float2& val) const { return get(cstr(name), val); }
		il int		get(c_str name, float3& val) const { __empty_return(0); float* ret = table_get_float3(_htab, name); if (!ret) return 0; val = float3(val); return 1; }
		il int		getc(const char* name, float3& val) const { return get(cstr(name), val); }
		il int		get(c_str name, float4& val) const { __empty_return(0); float* ret = table_get_float4(_htab, name); if (!ret) return 0; val = float4(val); return 1; }
		il int		getc(const char* name, float4& val) const { return get(cstr(name), val); }
		il int		get(c_str name, float4x4& val) const { __empty_return(0); float* ret = table_get_float4x4(_htab, name); if (!ret) return 0; val = float4x4(val); return 1; }
		il int		getc(const char* name, float4x4& val) const { return get(cstr(name), val); }

		il int		reserve(c_str name) { __empty_return(0); return table_reserve(_htab, name); }
		il int		reservec(const char* name) { return reserve(cstr(name)); }
		il int		clear(etcallback cb = etc_enable) { __empty_return(0); return table_clear(_htab, cb); }
		il int		erase(c_str name, etcallback cb = etc_enable) { __empty_return(0); return table_erase(_htab, name, cb); }
		il int		erasec(const char* name, etcallback cb = etc_enable) { return erase(cstr(name), cb); }
		il int		reset(c_str name, etcallback cb = etc_enable) { __empty_return(0); return table_reset(_htab, name, cb); }
		il int		resetc(const char* name, etcallback cb = etc_enable) { return reset(cstr(name), cb); }
		il int		setLexicographical(int enable) { __empty_return(0); return table_lexicographical(_htab, enable); }

		il int		save(const char* filename, int pretty) { __empty_return(0); return table_save(_htab, filename, pretty); }

		il Iterator begin() const;
		il Iterator next(Iterator& it) const;
		il Iterator end() const;

		template<class X, class Y>
		il int reg(Y *pthis, int(X::* func)(etopstatus, Table, c_str))
		{
			__empty_return(false);
			delegate3* pdlgt = table_reg_global(_htab);
			if (!pdlgt) return false;
			pdlgt->bind(pthis, func);
			return true;
		}
		template<class X, class Y>
		il int unreg(Y *pthis, int(X::* func)(etopstatus, Table, c_str))
		{
			__empty_return(false);
			delegate3 dlgt(pthis, func);
			return table_unreg_global(_htab, &dlgt);
		}
		template<class X, class Y>
		il int reg(c_str name, Y *pthis, int(X::* func)(etopstatus, Table, c_str))
		{
			__empty_return(false);
			delegate3* pdlgt = table_reg_element(_htab, name);
			if (!pdlgt) return false;
			pdlgt->bind(pthis, func);
			return true;
		}
		template<class X, class Y>
		il int regc(const char* name, Y *pthis, int(X::* func)(etopstatus, Table, c_str)) { return reg(cstr(name), pthis, func); }
		template<class X, class Y>
		il int reserve_reg(c_str name, Y *pthis, int(X::* func)(etopstatus, Table, c_str))
		{
			__empty_return(false);
			if (!table_reserve(_htab, name)) return false;
			return reg(name, pthis, func);
		}
		template<class X, class Y>
		il int reserve_regc(const char* name, Y *pthis, int(X::* func)(etopstatus, Table, c_str)) { return reserve_reg(cstr(name), pthis, func); }
		template<class X, class Y>
		il int unreg(c_str name, Y *pthis, int(X::* func)(etopstatus, Table, c_str))
		{
			__empty_return(false);
			delegate3 dlgt(pthis, func);
			table_unreg_element(_htab, name, &dlgt);
			return true;
		}
		template<class X, class Y>
		il int unregc(const char* name, Y *pthis, int(X::* func)(etopstatus, Table, c_str)) { return unreg(cstr(name), pthis, func); }
	};

	struct Iterator
	{
		htable _tab;
		tabit _it;
		il Iterator() : _it(invalid_table_iterator), _tab(invalid_table_handle) {}
		il bool operator!() const { return _it == invalid_table_iterator; }
		il bool operator==(const tabit& rhs) const { return _it == rhs; }
		il bool operator!=(const tabit& rhs) const { return _it != rhs; }

		il c_str		key() const { return tabit_get_key(_tab, _it); }
		il etvaltype	type() const { return tabit_get_type(_tab, _it); }
		il Table		getTable() const { Table tab; tab._htab = tabit_get_table(_tab, _it); return tab; }
		il int			getInteger() const { return tabit_get_integer(_tab, _it); }
		il float		getFloat() const { return tabit_get_float(_tab, _it); }
		il double		getDouble() const { return tabit_get_double(_tab, _it); }
		il int64		getInt64() const { return tabit_get_int64(_tab, _it); }
		il const char*	getString() const { return tabit_get_string(_tab, _it); }
		il c_str		getCstr() const { return tabit_get_cstr(_tab, _it); }
		il void*		getPtr() const { return tabit_get_ptr(_tab, _it); }
		il void			getUserdata(void*& ptr, unsigned& size) const { tabit_get_userdata(_tab, _it, &ptr, &size); }
		il uint			getUint() const { return tabit_get_uint(_tab, _it); }
		il uint64		getUint64() const { return tabit_get_uint64(_tab, _it); }
		il int			get(float2& val) const { float* ret = tabit_get_float2(_tab, _it); if (!ret) return 0; val = float2(val); return 1; }
		il int			get(float3& val) const { float* ret = tabit_get_float3(_tab, _it); if (!ret) return 0; val = float3(val); return 1; }
		il int			get(float4& val) const { float* ret = tabit_get_float4(_tab, _it); if (!ret) return 0; val = float4(val); return 1; }
		il int			get(float4x4& val) const { float* ret = tabit_get_float4x4(_tab, _it); if (!ret) return 0; val = float4x4(val); return 1; }
	};

	Iterator Table::begin() const { Iterator it; __empty_return(it); it._it = table_begin(_htab); return it; }
	Iterator Table::next(Iterator& it) const { Iterator nt; __empty_return(nt); nt._it = table_next(_htab, it._it); return nt; }
	Iterator Table::end() const { Iterator it; it._it = table_end; return it; }

	il Table CreateTable()
	{
		Table tab;
		tab._htab = table_create();
		return tab;
	}

	il void DestroyTable(Table& tab)
	{
		if (tab._htab)
		{
			table_destroy(tab._htab);
			tab._htab = invalid_table_handle;
		}
	}

	il Table OpenTableFile(const char* filename)
	{
		Table tab;
		tab._htab = table_read(filename);
		return tab;
	}

	typedef void*	hfile;
	typedef hfile(*file_open_read)(const char* filename);
	typedef hfile(*file_open_write)(const char* filename);
	typedef void(*file_close)(hfile hfl);
	typedef unsigned(*file_length)(hfile hfl);
	typedef unsigned(*file_read)(hfile hfl, void* buff, unsigned bytes);
	typedef unsigned(*file_write)(hfile hfl, void* buff, unsigned bytes);
}

extern "C" dxt int file_link(Schema::file_open_read fnor
	, Schema::file_open_write fnow
	, Schema::file_close fnclose
	, Schema::file_length fnlength
	, Schema::file_read fnr
	, Schema::file_write fnw);
