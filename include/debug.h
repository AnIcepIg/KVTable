/*
 *	debug tool suite
 *	call without visual studio attachment
 */
#pragma once

#include <windows.h>

/*
 *	Set data breakpoint in runtime, only support 4 breakpoints
 */

enum enuDebugBreakpoint
{
	DEBUG_BREAKPOINT_FUNCTION,		// function call
	DEBUG_BREAKPOINT_WRITE,			// write memory
	DEBUG_BREAKPOINT_READWRITE,		// read/write memory
};

enum enuDebugBreakpointSize
{
	DEBUG_BREAKPOINT_SIZE_1,		// 1 byte
	DEBUG_BREAKPOINT_SIZE_2,		// 2 bytes
	DEBUG_BREAKPOINT_SIZE_4,		// 4 bytes
	DEBUG_BREAKPOINT_SIZE_8			// 8 bytes
};

__declspec(dllexport) HANDLE debug_AddDataBreakpoint(enuDebugBreakpoint typ, enuDebugBreakpointSize size, void* addr);
__declspec(dllexport) void debug_RemoveDataBreakPoint(HANDLE dbp);

///*
// *	L1/L2 Cache Profiler
// *	L1/L2 Data&Instruction Cache Hit/Miss
// */
//__declspec(dllexport) int debug_CacheStart();
//__declspec(dllexport) int debug_CacheStop(const char* pcszSavedFile);
//__declspec(dllexport) int debug_CachePause();
//__declspec(dllexport) int debug_CacheMapThread(DWORD dwThreadId, int nIndex);
//__declspec(dllexport) int debug_CacheUnmapThread(DWORD dwThreadId);
