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
    ULONG          NextEntryDelta;          //���ɽṹ���е�ƫ������
    ULONG          ThreadCount;             //�߳���Ŀ��
    ULONG          Reserved1[6];           
    LARGE_INTEGER CreateTime;              //����ʱ�䣻
    LARGE_INTEGER UserTime;                //�û�ģʽ(Ring 3)��CPUʱ�䣻
    LARGE_INTEGER KernelTime;              //�ں�ģʽ(Ring 0)��CPUʱ�䣻
    UNICODE_STRING ProcessName;             //�������ƣ�
    KPRIORITY      BasePriority;            //��������Ȩ��
    ULONG          ProcessId;               //���̱�ʶ����
    ULONG          InheritedFromProcessId; //�����̵ı�ʶ����
    ULONG          HandleCount;             //�����Ŀ��
    ULONG          Reserved2[2];
    VM_COUNTERS    VmCounters;              //����洢���Ľṹ��
    IO_COUNTERS    IoCounters;              //IO�����ṹ�� Windows 2000 only
    //SYSTEM_THREADS Threads[1];              //��������̵߳Ľṹ���飻
}SYSTEM_PROCESSES,*PSYSTEM_PROCESSES;

// ansi�ַ���תunicode�ַ���
wchar_t* AnsiToUnicode(IN char *str) ;

#ifdef _UNICODE
#define GetKernelName GetKernelNameW
#else
#define GetKernelName GetKernelNameA
#endif
// ȡ�õ�ǰ����ϵͳ�ں�·����
BOOL GetKernelNameA(OUT char * KernelName) ;
BOOL GetKernelNameW(OUT WCHAR *KernelName) ;

// ȡ�ý����б�
LPBYTE GetProcessList(void) ;

BOOL Purview(void) ;

// ȡ��SSDT��������ֵ
bool GetSSDTFunctionIndex(char *NtFunctionName, PULONG puIndex) ;