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

    return 0;
}

