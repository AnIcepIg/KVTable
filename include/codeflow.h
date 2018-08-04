#pragma once

#ifndef RET_OK
#define RET_OK		1
#endif

#ifndef RET_FAIL
#define RET_FAIL	0
#endif

#if defined(_MSC_VER)
#define TEMP_DISABLE_WARNING(warningCode, expression)   \
	__pragma(warning(push))                             \
	__pragma(warning(disable:warningCode))              \
	expression                                          \
	__pragma(warning(pop))
#else
#define TEMP_DISABLE_WARNING(warningCode, expression)  expression
#endif 

#define WHILE_FALSE_NO_WARNING          \
	TEMP_DISABLE_WARNING(4127, while (false))

#ifndef _DEBUG

#ifndef ERROR_BREAK
#define ERROR_BREAK(condition)				if (!(condition)) break
#endif

#else

#ifndef ERROR_BREAK
#define ERROR_BREAK(condition)		\
	if (!(condition))				\
	{								\
		printf("ERROR_BREAK(%s) at line %d in %s\n", #condition, __LINE__, __FUNCTION__);		\
		break;						\
	} NULL
#endif

#endif

#ifndef _DEBUG

#ifndef CHECK_ERROR
#define CHECK_ERROR(condition)		condition
#endif

#else

#ifndef CHECK_ERROR
#define CHECK_ERROR(condition)		\
do									\
{									\
	if (!(condition))				\
		printf("CHECK_ERROR(%s) at line %d in %s\n", #condition, __LINE__, __FUNCTION__);	\
} WHILE_FALSE_NO_WARNING
#endif

#endif

#ifndef ERROR_GOTO
#define ERROR_GOTO(condition)		\
do									\
{									\
	if (!(condition)) goto err;		\
} WHILE_FALSE_NO_WARNING
#endif

#ifndef SUCCESS_BREAK_CODE
#define SUCCESS_BREAK_CODE(condition, code)	if (condition) { (code) = RET_OK; break; }
#endif
#ifndef SUCCESS_BREAK
#define SUCCESS_BREAK(condition)			if (condition) break
#endif

#ifndef _DEBUG

#ifndef COM_ERROR_BREAK
#define COM_ERROR_BREAK(condition)			if (FAILED(condition)) break
#endif

#else

#ifndef COM_ERROR_BREAK
#define COM_ERROR_BREAK(condition)		\
	if (FAILED(condition))				\
	{								\
		printf("COM_ERROR_BREAK(%s) at line %d in %s\n", #condition, __LINE__, __FUNCTION__);		\
		break;						\
	} NULL
#endif

#endif

#ifndef COM_ERROR_GOTO
#define COM_ERROR_GOTO(code)			\
do										\
{										\
	if (FAILED(code)) goto err;			\
} WHILE_FALSE_NO_WARNING 
#endif

#ifndef COM_SUCCESS_BREAK
#define COM_SUCCESS_BREAK(code)				if (SUCCEEDED(code)) break
#endif
