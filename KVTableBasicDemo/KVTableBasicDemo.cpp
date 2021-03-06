// KVTableBasicDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main()
{
	Schema::Table tab = Schema::CreateTable();
	tab.setc("integer", 0);
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

	Schema::Table tabMath = tab.setc("math");
	Schema::float2 f2(0, 3.14f);
	Schema::float3 f3(1.f, 2.f, 3.f);
	Schema::float4 f4(0, 0, 0, 1.f);
	Schema::float4x4 f4x4;
	tabMath.setc("float2", f2);
	tabMath.setc("float3", f3);
	tabMath.setc("float4", f4);
	tabMath.setc("float4x4", f4x4);

	struct _st { int a; float b; };
	_st st = { 1, 3.14f };
	Schema::Table tabUserdata = tab.setc("userdata");
	tabUserdata.setcUserdata("userdata", &st, sizeof(_st));
	st.a = 2; st.b = 0;
	tabUserdata.setcUserdata("userdata2", &st, sizeof(_st));

	/////////////////////i'm a seperate line
	printf("integer = %d\n", tab.getc("integer", 0));
	printf("unsigned int = %u\n", tab.get(cstr("unsigned int"), 0u));
	printf("integer64 = %lld\n", tab.getInt64(cstr("integer64")));
	printf("unsigned int64 = %llu\n", tab.getcUint64("unsigned int64"));

	tabReal = tab.getc("real");
	printf("float = %f\n", tabReal.getFloat(cstr("float")));
	printf("double = %.12lf\n", tabReal.getcDouble("double"));

	tabStr = tab.get(cstr("string"));
	printf("string = %s\n", tabStr.getStr(strString));
	printf("cstr = %s\n", tabStr.getCstr(strCStr));

	tabMath = tab.getc("math");
	tabMath.getc("float2", f2);
	tabMath.getc("float3", f3);
	tabMath.getc("float4", f4);
	tabMath.getc("float4x4", f4x4);

	tabUserdata = tab.getc("userdata");
	tabUserdata.getcUserdata("userdata", &st, sizeof(_st));
	tabUserdata.getcUserdata("userdata2", &st, sizeof(_st));

	Schema::DestroyTable(tab);

    return 0;
}

