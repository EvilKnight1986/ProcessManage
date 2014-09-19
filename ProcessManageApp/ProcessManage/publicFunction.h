#pragma once

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>


#define NULLVALUE_CHECK(value,fName) if(NULL == value){\
    OutputDebugString(#fName);\
    OutputDebugString(_T(" ")); \
    OutputDebugString(#value) ;\
    OutputDebugString(_T(" ")); \
    OutputDebugString(_T("can't NULL !\r\n")) ;\
    return FALSE;}

#define NULLVALUE_CHECK_NO_RETURN(value,fName) if(NULL == value){\
    OutputDebugString(#fName);\
    OutputDebugString(_T(" ")); \
    OutputDebugString(#value) ;\
    OutputDebugString(_T(" ")); \
    OutputDebugString(_T("can't NULL !\r\n")) ;\
    return ;}

typedef struct _LSA_UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union
    {
        LIST_ENTRY HashLinks;
        PVOID SectionPointer;
    };
    ULONG CheckSum;
    union
    {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;

} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,                // 0 Y N
    SystemProcessorInformation,            // 1 Y N
    SystemPerformanceInformation,        // 2 Y N
    SystemTimeOfDayInformation,            // 3 Y N
    SystemNotImplemented1,                // 4 Y N
    SystemProcessesAndThreadsInformation, // 5 Y N
    SystemCallCounts,                    // 6 Y N
    SystemConfigurationInformation,        // 7 Y N
    SystemProcessorTimes,                // 8 Y N
    SystemGlobalFlag,                    // 9 Y Y
    SystemNotImplemented2,                // 10 Y N
    SystemModuleInformation,            // 11 Y N
    SystemLockInformation,                // 12 Y N
    SystemNotImplemented3,                // 13 Y N
    SystemNotImplemented4,                // 14 Y N
    SystemNotImplemented5,                // 15 Y N
    SystemHandleInformation,            // 16 Y N
    SystemObjectInformation,            // 17 Y N
    SystemPagefileInformation,            // 18 Y N
    SystemInstructionEmulationCounts,    // 19 Y N
    SystemInvalidInfoClass1,            // 20
    SystemCacheInformation,                // 21 Y Y
    SystemPoolTagInformation,            // 22 Y N
    SystemProcessorStatistics,            // 23 Y N
    SystemDpcInformation,                // 24 Y Y
    SystemNotImplemented6,                // 25 Y N
    SystemLoadImage,                    // 26 N Y
    SystemUnloadImage,                    // 27 N Y
    SystemTimeAdjustment,                // 28 Y Y
    SystemNotImplemented7,                // 29 Y N
    SystemNotImplemented8,                // 30 Y N
    SystemNotImplemented9,                // 31 Y N
    SystemCrashDumpInformation,            // 32 Y N
    SystemExceptionInformation,            // 33 Y N
    SystemCrashDumpStateInformation,    // 34 Y Y/N
    SystemKernelDebuggerInformation,    // 35 Y N
    SystemContextSwitchInformation,        // 36 Y N
    SystemRegistryQuotaInformation,        // 37 Y Y
    SystemLoadAndCallImage,                // 38 N Y
    SystemPrioritySeparation,            // 39 N Y
    SystemNotImplemented10,                // 40 Y N
    SystemNotImplemented11,                // 41 Y N
    SystemInvalidInfoClass2,            // 42
    SystemInvalidInfoClass3,            // 43
    SystemTimeZoneInformation,            // 44 Y N
    SystemLookasideInformation,            // 45 Y N
    SystemSetTimeSlipEvent,                // 46 N Y
    SystemCreateSession,                // 47 N Y
    SystemDeleteSession,                // 48 N Y
    SystemInvalidInfoClass4,            // 49
    SystemRangeStartInformation,        // 50 Y N
    SystemVerifierInformation,            // 51 Y Y
    SystemAddVerifier,                    // 52 N Y
    SystemSessionProcessesInformation    // 53 Y N
} SYSTEM_INFORMATION_CLASS;

typedef struct _VM_COUNTERS {
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
} VM_COUNTERS, *PVM_COUNTERS;

typedef LONG KPRIORITY;

/*
*Information Class 5
*/
typedef struct _SYSTEM_PROCESSES
{
    ULONG          NextEntryDelta;          //构成结构序列的偏移量；
    ULONG          ThreadCount;             //线程数目；
    ULONG          Reserved1[6];           
    LARGE_INTEGER CreateTime;              //创建时间；
    LARGE_INTEGER UserTime;                //用户模式(Ring 3)的CPU时间；
    LARGE_INTEGER KernelTime;              //内核模式(Ring 0)的CPU时间；
    UNICODE_STRING ProcessName;             //进程名称；
    KPRIORITY      BasePriority;            //进程优先权；
    ULONG          ProcessId;               //进程标识符；
    ULONG          InheritedFromProcessId; //父进程的标识符；
    ULONG          HandleCount;             //句柄数目；
    ULONG          Reserved2[2];
    VM_COUNTERS    VmCounters;              //虚拟存储器的结构；
    IO_COUNTERS    IoCounters;              //IO计数结构； Windows 2000 only
    //SYSTEM_THREADS Threads[1];              //进程相关线程的结构数组；
}SYSTEM_PROCESSES,*PSYSTEM_PROCESSES;

// ansi字符串转unicode字符串
wchar_t* AnsiToUnicode(IN char *str) ;

#ifdef _UNICODE
#define GetKernelName GetKernelNameW
#else
#define GetKernelName GetKernelNameA
#endif
// 取得当前运行系统内核路径名
BOOL GetKernelNameA(OUT char * KernelName) ;
BOOL GetKernelNameW(OUT WCHAR *KernelName) ;

// 取得进程列表
LPBYTE GetProcessList(void) ;

BOOL Purview(void) ;

// 取得SSDT函数索引值
bool GetSSDTFunctionIndex(char *NtFunctionName, PULONG puIndex) ;