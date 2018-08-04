#include "include\utility.h"
#include "include\debug.h"
#include "include\codeflow.h"
#include "include\tcontainer.h"
#include "_g.h"
#include <windows.h>

struct __DBP_PARAM
{
	enuDebugBreakpoint typ;
	enuDebugBreakpointSize size;
	void* addr;
	HANDLE thrd;
	int slot;
};

inline LONG_PTR _set_dr7_bits(LONG_PTR dr7, int l, int b, int n)
{
	LONG_PTR mask = (1 << b) - 1;
	return (dr7 & ~(mask << l)) | (n << l);
}

static DWORD __stdcall AddDataBreakpointThread(void* param)
{
	__DBP_PARAM* pDbp = (__DBP_PARAM*)param;
	::SuspendThread(pDbp->thrd);
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	::GetThreadContext(pDbp->thrd, &context);
	int nSlot = -1;
	if (!(context.Dr7 & 0x01)) { nSlot = 0; context.Dr0 = (LONG_PTR)pDbp->addr; }
	else if (!(context.Dr7 & 0x4)) { nSlot = 1; context.Dr1 = (LONG_PTR)pDbp->addr; }
	else if (!(context.Dr7 & 0x10)) { nSlot = 2; context.Dr2 = (LONG_PTR)pDbp->addr; }
	else if (!(context.Dr7 & 0x40)) { nSlot = 3; context.Dr3 = (LONG_PTR)pDbp->addr; }
	if (nSlot >= 0)
	{
		pDbp->slot = nSlot;
		context.Dr6 = 0;
		int nType = 0;
		int nBits = 0;
		switch (pDbp->size)
		{
		case DEBUG_BREAKPOINT_SIZE_1: nBits = 0; break;
		case DEBUG_BREAKPOINT_SIZE_2: nBits = 1; break;
		case DEBUG_BREAKPOINT_SIZE_4: nBits = 3; break;
		case DEBUG_BREAKPOINT_SIZE_8: nBits = 2; break;
		}
		switch (pDbp->typ)
		{
		case DEBUG_BREAKPOINT_FUNCTION: nType = 0; nBits = 0; break;
		case DEBUG_BREAKPOINT_WRITE: nType = 1; break;
		case DEBUG_BREAKPOINT_READWRITE: nType = 3; break;
		}
		context.Dr7 = _set_dr7_bits(context.Dr7, 16 + nSlot * 4, 2, nType);
		context.Dr7 = _set_dr7_bits(context.Dr7, 18 + nSlot * 4, 2, nBits);
		context.Dr7 = _set_dr7_bits(context.Dr7, nSlot * 2, 1, 1);
		context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		::SetThreadContext(pDbp->thrd, &context);
	}
	::ResumeThread(pDbp->thrd);
	return 0;
}

static DWORD __stdcall RemoveDataBreakpointThread(void* param)
{
	__DBP_PARAM* pDbp = (__DBP_PARAM*)param;
	if (pDbp->slot < 0) return 0;
	::SuspendThread(pDbp->thrd);
	CONTEXT context = { 0 };
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	::GetThreadContext(pDbp->thrd, &context);
	if (pDbp->slot == 0) { context.Dr0 = 0; context.Dr7 &= ~(1); }
	else if (pDbp->slot == 1) { context.Dr1 = 0; context.Dr7 &= ~(1 << 2); }
	else if (pDbp->slot == 2) { context.Dr2 = 0; context.Dr7 &= ~(1 << 4); }
	else if (pDbp->slot == 3) { context.Dr3 = 0; context.Dr7 &= ~(1 << 6); }
	context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	::SetThreadContext(pDbp->thrd, &context);
	::ResumeThread(pDbp->thrd);
	return 0;
}

__declspec(dllexport) 
HANDLE debug_AddDataBreakpoint(enuDebugBreakpoint typ, enuDebugBreakpointSize size, void* addr)
{
	int nResult = false;
	__DBP_PARAM* param = nullptr;
	HANDLE hThread = NULL;

	do 
	{
		param = (__DBP_PARAM*)malloc(sizeof(__DBP_PARAM));
		ERROR_BREAK(param);
		memset(param, 0, sizeof(__DBP_PARAM));
		param->addr = addr;
		param->size = size;
		param->typ = typ;
		param->slot = -1;

		DWORD dwThreadId = ::GetCurrentThreadId();
		hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
		ERROR_BREAK(hThread);
		param->thrd = hThread;

		HANDLE hWork = ::CreateThread(nullptr, 0, AddDataBreakpointThread, param, 0, &dwThreadId);
		ERROR_BREAK(hWork);
		::WaitForSingleObject(hWork, 0xFFFFFFFF);
		ERROR_BREAK(param->slot != -1);

		nResult = true;
	} while (0);

	if (hThread)
	{
		::CloseHandle(hThread);
		hThread = NULL;
	}
	if (!nResult && param)
	{
		::free(param);
		param = nullptr;
	}
	return (HANDLE)param;
}

__declspec(dllexport)
void debug_RemoveDataBreakPoint(HANDLE dbp)
{
	__DBP_PARAM* param = (__DBP_PARAM*)dbp;
	if (!param) return;
	do 
	{
		DWORD dwThreadId = ::GetCurrentThreadId();
		HANDLE hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
		ERROR_BREAK(hThread);
		param->thrd = hThread;

		HANDLE hWork = ::CreateThread(nullptr, 0, RemoveDataBreakpointThread, param, 0, &dwThreadId);
		if (hWork) ::WaitForSingleObject(hWork, 0xFFFFFFFF);
		::CloseHandle(hThread);
	} while (0);
	::free(param);
}

//static GenericHashTable<StackKey, StackValue> sStacks;
//static GenericHashTable<RipKey, RipStats> sStats;
//static StackData sStackData;
//static uintptr_t sfnRaiseException;
//static int sEnabled = false;
//
//__declspec(dllexport) int debug_CacheStart()
//{
//	sStacks.Clear();
//	sStats.Clear();
//	memset(&sStackData, 0, sizeof(sStackData));
//
//	HMODULE hModule = ::LoadLibraryA("kernelbase.dll");
//	if (!hModule) return false;
//	sfnRaiseException = (uintptr_t)::GetProcAddress(hModule, "RaiseException");
//	if (!sfnRaiseException) return false;
//
//	if (sEnabled) return true;
//
//	return true;
//}
//
//__declspec(dllexport) int debug_CacheStop(const char* pcszSavedFile)
//{
//	return true;
//}
//__declspec(dllexport) int debug_CachePause()
//{
//	return true;
//}
//__declspec(dllexport) int debug_CacheMapThread(DWORD dwThreadId, int nIndex)
//{
//	return true;
//}
//__declspec(dllexport) int debug_CacheUnmapThread(DWORD dwThreadId)
//{
//	return true;
//}

namespace Schema
{
	hfile OpenReadFile(const char* pcszFile)
	{
		if (!pcszFile) return 0;
		FILE* fp = nullptr;
		fopen_s(&fp, pcszFile, "rb+");
		if (!fp) return 0;
		return (hfile)fp;
	}
	hfile OpenWriteFile(const char* pcszFile)
	{
		if (!pcszFile) return 0;
		FILE* fp = nullptr;
		fopen_s(&fp, pcszFile, "wb+");
		if (!fp) return 0;
		return (hfile)fp;
	}
	void CloseFile(hfile hFile)
	{
		FILE* fp = (FILE*)hFile;
		if (fp)
		{
			fclose(fp);
			fp = nullptr;
		}
	}
	unsigned TellFileLength(hfile hFile)
	{
		FILE* fp = (FILE*)hFile;
		if (!fp) return 0;
		::fseek(fp, 0, SEEK_END);
		unsigned uLen = (unsigned)ftell(fp);
		::fseek(fp, 0, SEEK_SET);
		return uLen;
	}
	unsigned FileRead(hfile hFile, void* pvBuff, unsigned uReadBytes)
	{
		FILE* fp = (FILE*)hFile;
		if (!fp) return 0;
		return (unsigned)fread_s(pvBuff, uReadBytes, sizeof(char), uReadBytes, fp);
	}
	unsigned FileWrite(hfile hFile, void* pvBuff, unsigned uWriteBytes)
	{
		FILE* fp = (FILE*)hFile;
		if (!fp) return 0;
		return (unsigned)fwrite(pvBuff, 1, uWriteBytes, fp);
	}
}
