# KVTable
```
a key-value pair container in runtime
a memory database
a data-driven pattern data structure
```
## create a new table
```cpp
#include "itable.h"   // kvtable interface header file  
Schema::Table tab = Schema::CreateTable();  // create an empty table
```
## add new pairs into table
```cpp
// Note that all keys stored in tables are in string pool, thus you need to put them into pool by call 'cstr' interface, see icstr.h
// But table interface also provides a convenise way to index by instant 'const char*', that is functions with 'c' version like gets/sets
// NOTE that although 'c' version's functions are very fast, but if you use table in some core codes, make sure to manually maintance c_str with your own way, it's OK in most cases
tab.setc("integer", 0);   // add a new pair key = "integer", value = 0
tab.set(cstr("unsigned int"), 1u);
tab.setc("integer64", 0xfffffffffi64);
tab.set(cstr("unsigned int64"), 0xffffffffffffffffui64);
Schema::Table tabReal = tab.set(cstr("real"));
tabReal.setc("float", 3.14f);
tabReal.setc("double", 3.14159265783);
Schema::Table tabStr = tab.setc("string");
c_str strString = cstr_string("string");
c_str strCStr = cstr_string("cstr");
tabStr.setStr(strString, "i'm a string");
tabStr.setCstr(strCStr, cstr("i'm a const string in string pool"));
```
## get stuff from table
```cpp
tab.getc("integer", 0);
tab.get(cstr("unsigned int"), 0u);
tab.getInt64(cstr("integer64"));
tab.getcUint64("unsigned int64");
tabReal = tab.getc("real");
tabReal.getFloat(cstr("float"));
tabReal.getcDouble("double");
tabStr = tab.get(cstr("string"));
tabStr.getStr(strString);
tabStr.getCstr(strCStr);
```
## travel table
```cpp
int Travel(Schema::Table tab)
{
	for (Schema::Iterator it = tab.begin(); it != tab.end(); it = tab.next(it))
	{
		printf("key = %s, value = ", it.key());
		etvaltype typ = it.type();
		switch (typ)
		{
		case etvt_reserved: printf("null\n"); break;
		case etvt_ptr: printf("0x%I64x\n", (uint64)(ULONG_PTR)it.getPtr()); break;
		case etvt_int: printf("%d\n", it.getInteger()); break;
		case etvt_uint: printf("%u\n", it.getUint()); break;
		case etvt_int64: printf("%lld\n", it.getInt64()); break;
		case etvt_uint64: printf("%I64u\n", it.getUint64()); break;
		case etvt_float: printf("%f\n", it.getFloat()); break;
		case etvt_double: printf("%lf\n", it.getDouble()); break;
		case etvt_float2:
		case etvt_float3:
		case etvt_float4: break;
		case etvt_cstr: printf("%s\n", it.getCstr()); break;
		case etvt_reference: break;
		case etvt_table: Travel(it.getTable()); break;
		case etvt_string: printf("%s\n", it.getString()); break;
		case etvt_float4x4: break;
		case etvt_userdata: break;
		}
	}
	return true;
}
```
