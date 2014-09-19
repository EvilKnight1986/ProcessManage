#pragma once

#ifndef CXX_KERNELUSERMANAGE_H
#define CXX_KERNELUSERMANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#define INOUT

#include <ntifs.h>
#include <ntddk.h>
#include "struct.h"

typedef struct _KERNELUSER
{
        LIST_ENTRY      ListEntry ;
        ULONG            UID ;
        UNICODE_STRING ustrUserName ;
}KERNELUSER, *PKERNELUSER ;

// 初始化用户管理器
BOOL InitializeKernelUserManage(void) ;

// 释放用户管理器
BOOL ReleaseKernelUserManage(void) ;

// 读取用户名以及对应的SID到用户管理器中
BOOL ReadKernelUser(void) ;

// 通过SID最后一段判断用户是否存在
BOOL UserIsExist(IN ULONG uUID) ;

// 通过SID取得用户名
PUNICODE_STRING GetUserNameByUID(IN ULONG uUID) ;

// 取得进程所属用户SID
BOOL GetProcessSID(IN ULONG dwPID, INOUT PUNICODE_STRING pustrProcessSID) ;

// 取得进程所属的用户id
ULONG GetProcessUID(IN ULONG dwPID) ;

#ifdef ALLOC_PRAGMA
// Allow the DriverEntry routine to be discarded once initialization is completed
#pragma alloc_text(INIT, InitializeKernelUserManage)
#pragma alloc_text(PAGE, ReleaseKernelUserManage)
#pragma alloc_text(INIT, ReadKernelUser)
#pragma alloc_text(PAGE, UserIsExist)
#pragma alloc_text(PAGE, GetUserNameByUID)
#pragma alloc_text(PAGE, GetProcessSID)
#pragma alloc_text(PAGE, GetProcessUID)
#endif // ALLOC_PRAGMA

#ifdef __cplusplus
}
#endif
#endif