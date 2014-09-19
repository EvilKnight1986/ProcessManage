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

// ��ʼ���û�������
BOOL InitializeKernelUserManage(void) ;

// �ͷ��û�������
BOOL ReleaseKernelUserManage(void) ;

// ��ȡ�û����Լ���Ӧ��SID���û���������
BOOL ReadKernelUser(void) ;

// ͨ��SID���һ���ж��û��Ƿ����
BOOL UserIsExist(IN ULONG uUID) ;

// ͨ��SIDȡ���û���
PUNICODE_STRING GetUserNameByUID(IN ULONG uUID) ;

// ȡ�ý��������û�SID
BOOL GetProcessSID(IN ULONG dwPID, INOUT PUNICODE_STRING pustrProcessSID) ;

// ȡ�ý����������û�id
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