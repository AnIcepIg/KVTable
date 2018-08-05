# KVTable
a key-value pair container in runtime

## Create a new table
#include "itable.h"   // kvtable interface header file

Schema::Table tab = Schema::CreateTable();  // create an empty table


## add new pairs into table
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

## get stuff from table
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
