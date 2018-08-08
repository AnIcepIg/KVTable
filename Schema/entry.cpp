#include "include\utility.h"
#include "cstr_pool.h"
#include "table.h"
#include "_g.h"
#include "tql.h"

using namespace Schema;

extern BigUniformSliceMemory<__pair>*		__pairalloc = 0;
extern BigUniformSliceMemory<__table>*		__taballoc = 0;
extern UniformSliceMemory<__delegates>*		__delegatesalloc = 0;
extern UniformSliceMemory<Schema::float4x4>*	__f4x4alloc = 0;
extern file_open_read _fopenread = nullptr;
extern file_open_write _fopenwrite = nullptr;
extern file_close _fclose = nullptr;
extern file_length _flength = nullptr;
extern file_read _fread = nullptr;
extern file_write _fwrite = nullptr;

class _Global
{
public:
	int _flag;
	_Global() : _flag(false) 
	{
		if (_strpool == 0)
		{
			_strpool = new(std::nothrow) atstring(999983);
			assert(_strpool);
		}

		if (__pairalloc == 0)
		{
			__pairalloc = new(std::nothrow) BigUniformSliceMemory<__pair>();
			assert(__pairalloc);
		}

		if (__taballoc == 0)
		{
			__taballoc = new(std::nothrow) BigUniformSliceMemory<__table>();
			assert(__taballoc);
		}

		if (__f4x4alloc == 0)
		{
			__f4x4alloc = new(std::nothrow) UniformSliceMemory<Schema::float4x4>();
			assert(__f4x4alloc);
		}

		if (__delegatesalloc == 0)
		{
			__delegatesalloc = new(std::nothrow) UniformSliceMemory<__delegates>();
			assert(__delegatesalloc);
		}

		_fopenread = OpenReadFile;
		_fopenwrite = OpenWriteFile;
		_fclose = CloseFile;
		_flength = TellFileLength;
		_fread = FileRead;
		_fwrite = FileWrite;

		_flag = true;
	}
	~_Global()
	{
		if (__tabtql)
		{
			table_destroy(__tabtql);
			__tabtql = nullptr;
		}

		if (__delegatesalloc)
		{
			delete __delegatesalloc;
			__delegatesalloc = nullptr;
		}

		if (__f4x4alloc)
		{
			delete __f4x4alloc;
			__f4x4alloc = nullptr;
		}

		if (__taballoc)
		{
			delete __taballoc;
			__taballoc = nullptr;
		}

		if (__pairalloc)
		{
			delete __pairalloc;
			__pairalloc = nullptr;
		}

		if (_strpool)
		{
			delete _strpool;
			_strpool = nullptr;
		}
		_flag = false;
	}
};

_Global _G;
