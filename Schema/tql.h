#pragma once

#include "..\include\itable.h"
#include "table.h"

extern __table* __tabtql;

#define TQL_IDENTITY_PREFIX			"tql"
#define TQL_IDENTITY_REQUEST		"request"
#define TQL_IDENTITY_RESPONSE		"response"
#define TQL_IDENTITY_HEADER			"header"
#define TQL_IDENTITY_BODY			"body"	
#define TQL_MAX_PROCESS_NUMBER		128
#define TQL_MAX_REQUEST_SIZE		1024

int tql_initialize();
int tql_uninitialize();
c_str tql_generate_identity();
int tql_create_region();
int tql_destroy_region();
