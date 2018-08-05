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
