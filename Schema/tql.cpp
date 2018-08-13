#include "tql.h"
#include "table.h"

extern __table* __tabtql = nullptr;

dxt int	tql_query(const char* q, htable htab)
{

	return true;
}
dxt int	tql_update()
{
	return true;
}

dxt int	tql_tag(htable htab, const char* name)
{
	if (!name || name[0] == '\0') return false;
	if (!__tabtql)
	{
		__tabtql = (__table*)table_create();
		if (!__tabtql) return false;
	}
	c_str sname = cstr(name);
	if (invalid_table_iterator != table_get_element(__tabtql, sname))
		return false;
	table_set_table(__tabtql, sname, htab, 0);
	return true;
}
