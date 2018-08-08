#pragma once

#include "include/itable.h"
#include "include/particular_alloc.h"

struct __delegates
{
	unsigned _count;
	unsigned _capacity;
	delegate3* _dlgts;

	il __delegates();
	il ~__delegates();

	il delegate3* reg();
	il void unreg(delegate3* pdlgt);

	il int reserve(unsigned cnt);

	static il __delegates* create();
	static il void destroy(__delegates* dlgts);
};

struct __table;
struct __pair
{
	union
	{
		unsigned _mask;
#pragma pack(1)
		struct {
			b_usm_mask bum : 8;
			uint8 vt : 8;
			unsigned external : 1;
			unsigned reserve : 15;
		}_cm;
#pragma pack()
	};

	c_str _key;
	union
	{
		void* _ptr;
		c_str _str;
		int _int;
		uint _uint;
		int64 _i64;
		uint64 _u64;
		float _flt;
		double _dbl;
		struct {
			unsigned size;
			void* ptr;
		}_userdata;
		Schema::float2 _f2;
		Schema::float3 _f3;
		Schema::float4 _f4;
		struct {
			uint64 x, y;
		}_domain;
	};
	__delegates* _dlgts;

	il __pair();
	il ~__pair();

	static il __pair* create(c_str name);
	static il void destroy(__pair* pair);
	static il Schema::float4x4* cf4x4(float* f4x4);
	static il void df4x4(Schema::float4x4* ptr);

	il void reset();
};

struct __table
{
	union
	{
		unsigned _mask;
#pragma pack(1)
		struct {
			b_usm_mask bum : 8;
			unsigned reserve : 23;
			unsigned lexicographical : 1;
		}_cm;
#pragma pack()
	};

	__pair** _elements;
	unsigned _count;
	unsigned _capacity;

	__delegates* _dlgts;

	il __table();
	il ~__table();

	static il __table* create();
	static il void destroy(__table* ptab);

	il int reserve(unsigned cnt);
	il int increase(unsigned cnt);
	il int reroder();

	il __table* set(c_str name, int callback);
	il int set(c_str name, __table* ptab, int callback);
	il int set(c_str name, int val, int callback);
	il int set(c_str name, float val, int callback);
	il int set(c_str name, double val, int callback);
	il int set(c_str name, __int64 val, int callback);
	il int set(c_str name, const char* val, int callback);
	il int set(c_str name, const char* val, unsigned len, int callback);
	il int setCstr(c_str name, c_str val, int callback);
	il int setPtr(c_str name, void* val, int callback);
	il int set(c_str name, void* ud, unsigned size, int callback);
	il int set(c_str name, uint val, int callback);
	il int set(c_str name, uint64 val, int callback);
	il int setf2(c_str name, float* val, int callback);
	il int setf3(c_str name, float* val, int callback);
	il int setf4(c_str name, float* val, int callback);
	il int setf4x4(c_str name, float* val, int callback);

	il __pair* set(c_str name);
	il __pair* set_disorder(c_str name);
	il __pair* set_lexicographical(c_str name);

	il int trigger(etopstatus ops, __pair* pair);
	il int trigger(etopstatus ops, c_str name);
	il int erase(c_str name, int callback);
	il int reset(c_str name, int callback);

	il __pair* get(c_str name);
	il __pair* get_disorder(c_str name);
	il __pair* get_lexicographical(c_str name);

	il unsigned getIdx(c_str name);
	il unsigned getIdx_disorder(c_str name);
	il unsigned getIdx_lexicographical(c_str name);

	il int erase_disorder(c_str name);
	il int erase_lexicographical(c_str name);

	il delegate3* reg();
	il delegate3* reg(c_str name);
	il void unreg(delegate3* pdlgt);
	il void unreg(c_str name, delegate3* pdlgt);
};

extern BigUniformSliceMemory<__pair>*		__pairalloc;
extern BigUniformSliceMemory<__table>*		__taballoc;
extern UniformSliceMemory<__delegates>*		__delegatesalloc;
extern UniformSliceMemory<Schema::float4x4>*	__f4x4alloc;

extern Schema::file_open_read _fopenread;
extern Schema::file_open_write _fopenwrite;
extern Schema::file_close _fclose;
extern Schema::file_length _flength;
extern Schema::file_read _fread;
extern Schema::file_write _fwrite;

struct __iterator
{

};
