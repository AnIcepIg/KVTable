#include "include/icstr.h"
#include "cstr_pool.h"

extern "C" {
	__declspec(dllexport) c_str cstr_string(const char* str)
	{
		return Schema::_strpool->string(str);
	}

	__declspec(dllexport) c_str cstr_cache(const char* str)
	{
		return Schema::_strpool->cache(str);
	}

	__declspec(dllexport) c_str cstr_stringl(const char* str, unsigned l)
	{
		return Schema::_strpool->string(str, l);
	}

	__declspec(dllexport) c_str cstr_path(const char* str)
	{
		return Schema::_strpool->path(str);
	}

	__declspec(dllexport) c_str cstr_pathl(const char* str, unsigned l)
	{
		return Schema::_strpool->path(str, l);
	}

	__declspec(dllexport) c_str cstr_tolower(const char* str)
	{
		return Schema::_strpool->tolower(str);
	}

	__declspec(dllexport) c_str cstr_tolowerl(const char* str, unsigned l)
	{
		return Schema::_strpool->tolower(str, l);
	}

	__declspec(dllexport)
		c_str cstr_string0(const char* str)
	{
		if (str)
			return Schema::_strpool->string(str);
		else
			return Schema::_strpool->string("");
	}

	__declspec(dllexport)
		c_str cstr_path0(const char* str)
	{
		if (str)
			return Schema::_strpool->path(str);
		else
			return Schema::_strpool->path(".\\");
	}

	__declspec(dllexport)
		c_str cstr_find(const char* str)
	{
		return Schema::_strpool->find(str);
	}

	__declspec(dllexport)
		c_str cstr_ustring(unsigned u)
	{
		return Schema::_strpool->ustring(u);
	}

	__declspec(dllexport)
		c_str cstr_u64string(unsigned __int64 u64)
	{
		return Schema::_strpool->u64string(u64);
	}

	__declspec(dllexport)
		c_str cstr_istring(int i)
	{
		return Schema::_strpool->istring(i);
	}

	__declspec(dllexport)
		c_str cstr_i64string(__int64 i64)
	{
		return Schema::_strpool->i64string(i64);
	}

}
