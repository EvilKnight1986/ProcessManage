/***************************************************************************************
* AUTHOR : EvilKnight
* DATE   : 2014-7-6
* MODULE : ProcessManageSys.H
*
* IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2014 EvilKnight.
****************************************************************************************/

#ifndef CXX_PROCESSMANAGESYS_H
#define CXX_PROCESSMANAGESYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ntifs.h>
#include <ntddk.h>
#include <devioctl.h>
//#include "struct.h"
#include "common.h"

//
// TODO: Add your include here
//


//////////////////////////////////////////////////////////////////////////

//
// TODO: Add your struct,enum(private) here
//
#define MAX_PROTECT_COUNT (256)
// 定义设备扩展
typedef struct _stDEVICE_EXTENSION
{
        // 这里先留空，保护的进程名
        bool bIsSetCreateProcessNotifyRoutine ;            // 是否注册了回调PsSetCreateProcessNotifyRoutine
        bool bIsHookNtOpenProcess ;                            // 是否Hook了NtOpenProcess函数
        bool bIsHookNtQuerySystemInformation ;         // 是否Hook了NtQuerySystemInformation函数
        bool bIsFiltrateProcess;                                      // 是否过滤进程
        ULONG uActiveProcessLinksOffset ;                   // ActiveProcessLinks在_EPROCESS中的偏移
        ULONG UniqueProcessIdOffset ;                        // UniqueProcessId在_EPROCESS中的偏移
        ULONG uNtOpenProcessIndex ;                         // NtOpenProcess在SSDT中的索引值 
        ULONG uNtQuerySystemInformationIndex ;      // NtQuerySystemInformation在SSDT中的索引值
        KMUTEX       DeviceIoControlMutex ;                // DeviceIoControl的自旋锁
        KMUTEX       MyThreadMutex ;                         // 对自己数据的一些同步
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _ServiceDescriptorTable
{
        PULONG ServiceTableBase;        // SSDT基址
        PULONG ServiceCounterTable;     // 包含SSDT中每个服务被调用次数的计数器，一般由sysenter更新即Ring3转Ring0中断
        ULONG NumberOfServices; // 由ServiceTableBase描述服务的数目
        PULONG ParamTableBase;          // 包含每个系统服务参数字节数表的基地址-系统服务参数表        
}*PServiceDescriptorTable;

//////////////////////////////////////////////////////////////////////////
//***************************************************************************************
//* NAME:			DriverEntry
//*
//* DESCRIPTION:	Registers dispatch routines.
//*					
//*	PARAMETERS:		pDriverObj						IN		
//*						Address of the DRIVER_OBJECT created by NT for this driver.
//*					pRegistryString					IN		
//*						UNICODE_STRING which represents this drivers KEY in the Registry.  	
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		NTSTATUS
//***************************************************************************************
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString);

// 初始化Device_Extension中我们一些偏移的值
bool InitlizedDeviceExtension(PDEVICE_EXTENSION pde) ;

//***************************************************************************************
//* NAME:			DriverUnload
//*
//* DESCRIPTION:	This routine is our dynamic unload entry point.
//*					
//*	PARAMETERS:		pDriverObj						IN		Address of our DRIVER_OBJECT.
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		None
//***************************************************************************************
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObj);

//***************************************************************************************
//* NAME:			DispatchCreate, DispatchClose
//*
//* DESCRIPTION:	This two methods are the dispatch entry point for IRP_MJ_CREATE and IRP_MJ_CLOSE 
//*					routines.  This sample simply completes the requests with success.
//*					
//*	PARAMETERS:		pDevObj							IN		Address of our DRIVER_OBJECT.
//*					pIrp							IN		Address of the IRP.
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		STATUS_SUCCESS
//***************************************************************************************
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS DispatchClose(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

//***************************************************************************************
//* NAME:			DispatchDeviceControl
//*
//* DESCRIPTION:	This is the dispatch entry point for IRP_MJ_DEVICE_CONTROL.
//*					
//*	PARAMETERS:		pDevObj							IN		Address of our DRIVER_OBJECT.
//*					pIrp							IN		Address of the IRP.
//*
//*	IRQL:			IRQL_PASSIVE_LEVEL.
//*
//*	RETURNS:		NTSTATUS
//*
//*	NOTES:			IRP_MJ_DEVICE_CONTROL
//*					Parameters:
//*					Parameters.DeviceIoControl.OutputBufferLength	Length of OutBuffer 
//*					in bytes (length of buffer from GUI)
//*					Parameters.DeviceIoControl.InputBufferLength	Length of InBuffer 
//*					in bytes (length of buffer from DRIVER)
//*					Parameters.DeviceIoControl.ControlCode			I/O control code
//***************************************************************************************
NTSTATUS DispatchDeviceControl(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

// common dispatch
//***************************************************
//* #define IRP_MJ_CREATE                   0x00
//* #define IRP_MJ_CREATE_NAMED_PIPE        0x01
//* #define IRP_MJ_CLOSE                    0x02
//* #define IRP_MJ_READ                     0x03
//* #define IRP_MJ_WRITE                    0x04
//* #define IRP_MJ_QUERY_INFORMATION        0x05
//* #define IRP_MJ_SET_INFORMATION          0x06
//* #define IRP_MJ_QUERY_EA                 0x07
//* #define IRP_MJ_SET_EA                   0x08
//* #define IRP_MJ_FLUSH_BUFFERS            0x09
//* #define IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a
//* #define IRP_MJ_SET_VOLUME_INFORMATION   0x0b
//* #define IRP_MJ_DIRECTORY_CONTROL        0x0c
//* #define IRP_MJ_FILE_SYSTEM_CONTROL      0x0d
//* #define IRP_MJ_DEVICE_CONTROL           0x0e
//* #define IRP_MJ_INTERNAL_DEVICE_CONTROL  0x0f
//* #define IRP_MJ_SHUTDOWN                 0x10
//* #define IRP_MJ_LOCK_CONTROL             0x11
//* #define IRP_MJ_CLEANUP                  0x12
//* #define IRP_MJ_CREATE_MAILSLOT          0x13
//* #define IRP_MJ_QUERY_SECURITY           0x14
//* #define IRP_MJ_SET_SECURITY             0x15
//* #define IRP_MJ_POWER                    0x16
//* #define IRP_MJ_SYSTEM_CONTROL           0x17
//* #define IRP_MJ_DEVICE_CHANGE            0x18
//* #define IRP_MJ_QUERY_QUOTA              0x19
//* #define IRP_MJ_SET_QUOTA                0x1a
//* #define IRP_MJ_PNP                      0x1b
//* #define IRP_MJ_PNP_POWER                IRP_MJ_PNP      // Obsolete....
//* #define IRP_MJ_MAXIMUM_FUNCTION         0x1b -->
//***************************************************************************************
NTSTATUS DispatchCommon (IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

// 隐藏进程 
bool HideProcess(IN PDEVICE_OBJECT pDevObj, ULONG uPID) ;

// 去除页面保护
void PageProtectOFF(void) ;

// 开启页面保护
void PageProtectON(void) ;

// 拷贝指定地址指定长度的数据到目标地址
bool MyMemCopy(PUCHAR pDstAddr, PUCHAR* pSrcAddr, ULONG uSrcSize) ;

// 用来接收创建和销毁进程的消息
VOID
CreateProcessNotifyRoutine (
                                   IN HANDLE  ParentId,
                                   IN HANDLE  ProcessId,
                                   IN BOOLEAN  Create
                                   );
// 挂钩的 NtOpenProcess函数
NTSTATUS DetoursNtOpenProcess(OUT PHANDLE             ProcessHandle,
                                                     IN ACCESS_MASK          AccessMask,
                                                     IN POBJECT_ATTRIBUTES   ObjectAttributes,
                                                     IN PCLIENT_ID           ClientId  ) ;

// SSDT Hook NtOpenProcess
bool HookNtOpenProcess(IN PDEVICE_OBJECT pDevObj)  ;

// SSDT Unhook NtOpenProcess
bool UnHookNtOpenProcess(IN PDEVICE_OBJECT pDevObj)  ;

// 挂钩的 NtQuerySystemInformation函数
NTSTATUS DetoursNtQuerySystemInformation(IN ULONG SystemInformationClass,
                                                                        OUT PVOID SystemInformation,
                                                                        IN ULONG SystemInformationLength,
                                                                        OUT PULONG ReturnLength OPTIONAL) ;
// SSDT Hook NtQuerySystemInformation
bool HookNtQuerySystemInformation(IN PDEVICE_OBJECT pDevObj) ;

// SSDT UnHook NtQuerySystemInformation
bool UnHookNtQuerySystemInformation(IN PDEVICE_OBJECT pDevObj) ;

// 判断是否为保护进程
bool isProtectProcess(IN PCLIENT_ID     ClientId ) ;

// 添加需要保护的进程id
bool AddProtectProcessPID(ULONG uPID) ;

// 添加需要保护的进程名
bool AddProtectProcessName(PUCHAR pProcessName) ;

// 通过pid查找保护进程pid表，判断是否为要保护的进程id
bool FindProtectProcessPID(ULONG uPID) ;

// 通过进程名查找保护进程名表，判断是否为要保护的进程名
bool FindProtectProcessName(PUCHAR pProcessName) ;

NTSTATUS Test(void) ;
//////////////////////////////////////////////////////////////////////////

#ifdef ALLOC_PRAGMA
// Allow the DriverEntry routine to be discarded once initialization is completed
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, InitlizedDeviceExtension)
#pragma alloc_text(PAGE, DriverUnload)
#pragma alloc_text(PAGE, DispatchCreate)
#pragma alloc_text(PAGE, DispatchClose)
#pragma alloc_text(PAGE, DispatchDeviceControl)
#pragma alloc_text(PAGE, DispatchCommon)
#pragma alloc_text(PAGE, HideProcess )
#pragma alloc_text(PAGE, PageProtectOFF)
#pragma alloc_text(PAGE, PageProtectON)
#pragma alloc_text(PAGE, MyMemCopy)
#pragma alloc_text(PAGE, CreateProcessNotifyRoutine)
#pragma alloc_text(PAGE, DetoursNtOpenProcess)
#pragma alloc_text(PAGE, HookNtOpenProcess)
#pragma alloc_text(PAGE, UnHookNtOpenProcess)
#pragma alloc_text(PAGE, DetoursNtQuerySystemInformation)
#pragma alloc_text(PAGE, HookNtQuerySystemInformation)
#pragma alloc_text(PAGE, UnHookNtQuerySystemInformation)
#pragma alloc_text(PAGE, isProtectProcess)
#pragma alloc_text(PAGE, AddProtectProcessPID)
#pragma alloc_text(PAGE, AddProtectProcessName)
#pragma alloc_text(PAGE, FindProtectProcessPID)
#pragma alloc_text(PAGE, FindProtectProcessName)
#pragma alloc_text(PAGE, Test)
#endif // ALLOC_PRAGMA

//////////////////////////////////////////////////////////////////////////

//
// TODO: Add your module declarations here
//



#ifdef __cplusplus
}
#endif
//////////////////////////////////////////////////////////////////////////

#endif	//CXX_PROCESSMANAGESYS_H
/* EOF */
