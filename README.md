# KVTable
```
a key-value pair container in runtime
a memory database
a data-driven pattern data structure
```

## Contents
* [Create a New Table](#create-a-new-table)
* [Add New elements into Table](#add-new-elements-into-table)
* [Get Stuffs from Table](#get-stuffs-from-table)
* [Travel Table](#travel-table)
* [Internal Special Data Type](#internal-special-data-type)
* [Serialization](#serialization)

## create a new table
KVTableBasicDemo gives you more details
```cpp
#include "itable.h"   // kvtable interface header file  
Schema::Table tab = Schema::CreateTable();  // create an empty table
```
## add new elements into table
KVTableBasicDemo gives you more details
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
## get stuffs from table
KVTableBasicDemo gives you more details
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
KVTableTravelDemo gives you more details
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
## internal special data type
KVTableBasicDemo gives you more details
  ### float2,3,4 and float matrix
  ```cpp
  Schema::Table tabMath = tab.setc("math");
  Schema::float2 f2(0, 3.14f);
  Schema::float3 f3(1.f, 2.f, 3.f);
  Schema::float4 f4(0, 0, 0, 1.f);
  Schema::float4x4 f4x4;
  tabMath.setc("float2", f2);
  tabMath.setc("float3", f3);
  tabMath.setc("float4", f4);
  tabMath.setc("float4x4", f4x4);
  
  tabMath.getc("float2", f2);
  tabMath.getc("float3", f3);
  tabMath.getc("float4", f4);
  tabMath.getc("float4x4", f4x4);
  ```
  ### userdata
  ```cpp
  struct _st { int a; float b; };
  _st st = { 1, 3.14f };
  Schema::Table tabUserdata = tab.setc("userdata");
  tabUserdata.setcUserdata("userdata", &st, sizeof(_st));
  st.a = 2; st.b = 0;
  tabUserdata.setcUserdata("userdata2", &st, sizeof(_st));
  
  tabUserdata.getcUserdata("userdata", &st, sizeof(_st));
  tabUserdata.getcUserdata("userdata2", &st, sizeof(_st));
  ```
## serialization
KVTableSerialiseDemo gives you more details
```cpp
tab.save("tab_text.tab.txt", etft_text, false);		// save as readable text file
tab.save("tab_text_pretty.tab.txt", etft_text, true);	// save as readable text file with mutilple lines and indent
tab.save("tab_bin.tab", etft_binary, 0);		// save as a binary file with 'zlib' compress

Schema::DestroyTable(tab);

tab = Schema::OpenTableFile("tab_text.tab.txt");
Schema::DestroyTable(tab);
tab = Schema::OpenTableFile("tab_text_pretty.tab.txt");
Schema::DestroyTable(tab);
tab = Schema::OpenTableFile("tab_bin.tab");
Schema::DestroyTable(tab);
```
