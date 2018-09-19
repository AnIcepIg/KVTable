#include "tql.h"
#include "table.h"
#include <atlbase.h>

extern __table* __tabtql = nullptr;

struct __tql_shared_header
{
	char name[256];
	unsigned capacity;
};

struct __tql_shared_body
{
	unsigned size;
	void* buff;
};

struct __tql_region
{
	HANDLE _request;
	HANDLE _response;
	HANDLE _header;
	HANDLE _body;
	c_str _identity;
	__tql_shared_header* _pheader;
	__tql_shared_body* _pbody;
};
static __tql_region __tqlregion = { 0 };

dxt int	tql_query(const char* q, htable htab)
{
	if (!q || !__tabtql) return false;
	return true;
}
dxt int	tql_update()
{
	if (!__tabtql) return false;
	return true;
}

dxt int	tql_tag(htable htab, const char* name)
{
	if (!name || name[0] == '\0') return false;
	if (!__tabtql && !tql_initialize()) return false;
	c_str sname = cstr(name);
	if (invalid_table_iterator != table_get_element(__tabtql, sname))
		return false;
	table_set_table(__tabtql, sname, htab, 0);
	return true;
}

int tql_initialize()
{
	int rst = false;
	do 
	{
		__tabtql = (__table*)table_create();
		if (!__tabtql) break;
		__tqlregion._identity = tql_generate_identity();
		if (!__tqlregion._identity) break;
		if (!tql_create_region()) break;
		rst = true;
	} while (0);
	if (!rst) tql_uninitialize();
	return rst;
}

int tql_uninitialize()
{
	tql_destroy_region();
	if (__tabtql)
	{
		table_destroy(__tabtql);
		__tabtql = nullptr;
	}
	return true;
}

c_str tql_generate_identity()
{
	char id[128];
	char ent[128];
	for (int i = 0; i < TQL_MAX_PROCESS_NUMBER; ++i)
	{
		sprintf_s(id, _countof(id), "Global\\%s%d", TQL_IDENTITY_PREFIX, i);
		sprintf_s(ent, _countof(ent), "%s%s", id, TQL_IDENTITY_REQUEST);
		HANDLE hnt = ::OpenEventA(EVENT_ALL_ACCESS, FALSE, ent);
		if (hnt)
		{
			::CloseHandle(hnt);
			continue;
		}
		return cstr_string(id);
	}
	return nullptr;
}

int tql_create_region()
{
	char id[128];
	c_str identity = __tqlregion._identity;
	sprintf_s(id, _countof(id), "%s%s", identity, TQL_IDENTITY_REQUEST);
	HANDLE hnt = ::CreateEventA(NULL, FALSE, FALSE, id);
	if (!hnt) return false;
	__tqlregion._request = hnt;
	sprintf_s(id, _countof(id), "%s%s", identity, TQL_IDENTITY_RESPONSE);
	hnt = ::CreateEventA(NULL, FALSE, FALSE, id);
	if (!hnt) return false;
	__tqlregion._response = hnt;
	sprintf_s(id, _countof(id), "%s%s", identity, TQL_IDENTITY_HEADER);
	hnt = ::CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(__tql_shared_header), id);
	if (!hnt) return false;
	__tqlregion._header = hnt;
	__tql_shared_header* pheader = (__tql_shared_header*)::MapViewOfFile(hnt, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!pheader) return false;
	__tqlregion._pheader = pheader;
	memset(pheader, 0, sizeof(__tql_shared_header));
	char path[MAX_PATH];
	::GetModuleFileNameA(NULL, path, _countof(path));
	_splitpath_s(path, nullptr, 0, nullptr, 0, id, _countof(id), nullptr, 0);
	strcpy_s(pheader->name, id);
	unsigned capacity = sizeof(__tql_shared_body) + TQL_MAX_REQUEST_SIZE;
	sprintf_s(id, _countof(id), "%s%s", identity, TQL_IDENTITY_BODY);
	hnt = ::CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, capacity, id);
	if (!hnt) return false;
	__tqlregion._body = hnt;
	pheader->capacity = TQL_MAX_REQUEST_SIZE;
	__tql_shared_body* pbody = (__tql_shared_body*)::MapViewOfFile(hnt, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!pbody) return false;
	memset(pbody, 0, capacity);
	pbody->buff = (void*)(pbody + 1);
	__tqlregion._pbody = pbody;
	return true;
}

int tql_destroy_region()
{
	if (__tqlregion._pbody)
	{
		::UnmapViewOfFile(__tqlregion._pbody);
		__tqlregion._pbody = nullptr;
	}
	if (__tqlregion._body)
	{
		::CloseHandle(__tqlregion._body);
		__tqlregion._body = NULL;
	}
	if (__tqlregion._pheader)
	{
		::UnmapViewOfFile(__tqlregion._pheader);
		__tqlregion._pheader = nullptr;
	}
	if (__tqlregion._header)
	{
		::CloseHandle(__tqlregion._header);
		__tqlregion._header = NULL;
	}
	if (__tqlregion._response)
	{
		::CloseHandle(__tqlregion._response);
		__tqlregion._response = NULL;
	}
	if (__tqlregion._request)
	{
		::CloseHandle(__tqlregion._request);
		__tqlregion._request = NULL;
	}
	return true;
}
