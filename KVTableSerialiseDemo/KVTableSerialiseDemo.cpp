// KVTableSerialiseDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	Schema::Table tab = Schema::CreateTable();
	tab.setc("integer", -1);
	tab.set(cstr("unsigned int"), 1u);
	tab.setc("integer64", 0xfffffffffi64);
	tab.set(cstr("unsigned int64"), 0xffffffffffffffffui64);
	tab.set(cstr("bool"), true);
	tab.setc("false", false);
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

	tab.save("tab_text.tab.txt", etft_text, false);
	tab.save("tab_text_pretty.tab.txt", etft_text, true);
	tab.save("tab_bin.tab", etft_binary, 0);

	Schema::DestroyTable(tab);

	tab = Schema::OpenTableFile("tab_text.tab.txt");
	Schema::DestroyTable(tab);
	tab = Schema::OpenTableFile("tab_text_pretty.tab.txt");
	Schema::DestroyTable(tab);
	tab = Schema::OpenTableFile("tab_bin.tab");
	Schema::DestroyTable(tab);

    return 0;
}

