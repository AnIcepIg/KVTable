#pragma once

/*******************************************************************************
* c_str is a const string point in string pool
* Unique
* Safe
* Fast
*******************************************************************************/
typedef const char*		c_str;
typedef unsigned		hcstr;

/*******************************************************************************
* initialize a 'C' like string(end with '\0') as c_str
*******************************************************************************/
extern "C" __declspec(dllexport) c_str cstr_string(const char* str);
extern "C" __declspec(dllexport) c_str cstr_stringl(const char* str, unsigned l);

/*******************************************************************************
* initialize a 'C' like string(end with '\0') as c_str
* normalize as windows pattern path string
*******************************************************************************/
extern "C" __declspec(dllexport) c_str cstr_path(const char* str);
extern "C" __declspec(dllexport) c_str cstr_pathl(const char* str, unsigned l);

/*******************************************************************************
* initialize a 'C' like string(end with '\0') as case-insensitive c_str
*******************************************************************************/
extern "C" __declspec(dllexport) c_str cstr_tolower(const char* str);
extern "C" __declspec(dllexport) c_str cstr_tolowerl(const char* str, unsigned l);

/*******************************************************************************
* initialize a 'C' like string(end with '\0') as c_str
* if 'str' is a 0 point, return a c_str point to an empty string
*******************************************************************************/
extern "C" __declspec(dllexport)
c_str cstr_string0(const char* str);

/*******************************************************************************
* initialize a 'C' like string(end with '\0') as c_str
* normalize as windows pattern path string
* if 'str' is a 0 point, return a c_str point to ".\\" string
*******************************************************************************/
extern "C" __declspec(dllexport)
c_str cstr_path0(const char* str);

/*******************************************************************************
* find a 'C' like string in string pool
* if matched, return c_str point
*******************************************************************************/
extern "C" __declspec(dllexport)
c_str cstr_find(const char* str);

/*******************************************************************************
* fast get c_str length
* make sure pass the c_str point, not a const char*
*******************************************************************************/
#define cstr_len(str)		(*((unsigned*)str - 1))
#define cstr_length(str)	(*((unsigned*)str - 1))
#define cstr(str)			cstr_string(str)

extern "C" __declspec(dllexport) c_str cstr_ustring(unsigned u);
extern "C" __declspec(dllexport) c_str cstr_u64string(unsigned __int64 u64);
extern "C" __declspec(dllexport) c_str cstr_istring(int i);
extern "C" __declspec(dllexport) c_str cstr_i64string(__int64 i64);
extern "C" __declspec(dllexport) c_str cstr_cache(const char* str);
