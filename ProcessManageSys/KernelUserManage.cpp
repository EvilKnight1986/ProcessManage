//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               ���汣��         ����BUG
//
//
//

#include "KernelUserManage.h"

PKERNELUSER g_pKernelUser = NULL ;
PAGED_LOOKASIDE_LIST g_PagedLookasideList ;
/*******************************************************************************
*
*   �� �� �� : InitializeKernelUserManage
*  �������� : ��ʼ���ں��û�������
*  �����б� : ��
*  ˵      �� :  �˺�����Ҫ��DriverEntry��ִ�� 
*  ���ؽ�� :  ����ɹ�������TRUE, ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL InitializeKernelUserManage(void)
{
        BOOL bResult = FALSE ;
        __try
        {
                // ��ֹ�û���γ�ʼ��
                if (NULL != g_pKernelUser)
                {
                        ReleaseKernelUserManage() ;
                }

                // ��ʼ��PagedLookasideList
                ExInitializePagedLookasideList(&g_PagedLookasideList,
                                                                NULL,
                                                                NULL,
                                                                0,
                                                                sizeof(KERNELUSER),
                                                                'HDHD',
                                                                0) ;

                bResult = ReadKernelUser() ;
        }

        __finally
        {
        }

        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : ReleaseKernelUserManage
*  �������� : �ͷ��û�������
*  �����б� : ��
*   ˵      �� : 
*  ���ؽ�� :  ����ɹ�������TRUE, ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL ReleaseKernelUserManage(void)
{
        BOOL bResult = FALSE ;
        PKERNELUSER pNode = NULL ;

        __try
        {
                if (NULL == g_pKernelUser)
                {
                        KdPrint(("ReleaseKernelUserManage g_pKernelUser is NULL!\r\n")) ;
                        __leave ;
                }

                // ���ﻹҪ��������ڴ�ȫ����
                while (IsListEmpty(&(g_pKernelUser->ListEntry)))
                {
                        pNode = CONTAINING_RECORD(RemoveTailList(&(g_pKernelUser->ListEntry)),
                                                                                                        KERNELUSER,
                                                                                                        ListEntry) ;
                        RtlFreeUnicodeString(&pNode->ustrUserName) ;
                        ExFreeToPagedLookasideList(&g_PagedLookasideList, pNode) ;
                }

                ExFreeToPagedLookasideList(&g_PagedLookasideList, g_pKernelUser) ;
                g_pKernelUser = NULL ;

                ExDeletePagedLookasideList(&g_PagedLookasideList) ;

                bResult = TRUE ;
        }

        __finally
        {
        }
        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : ReadKernelUser
*  �������� : ��ȡ�û����Լ���Ӧ��SID���û���������
*  �����б� : ��
*   ˵      �� : �˺�����Ҫ��DriverEntry��ִ�� 
*  ���ؽ�� :  ����ɹ�������TRUE, ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL ReadKernelUser(void)
{
        BOOL bResult = FALSE ;
        OBJECT_ATTRIBUTES ObjAttributeNames ;
        UNICODE_STRING    ustrObjectNames ;
        HANDLE  hNamesKey = NULL;
        HANDLE hSubKey = NULL;
        PKEY_FULL_INFORMATION pKeyFullInfor = NULL ;
        PKEY_BASIC_INFORMATION pBi = NULL ;
        NTSTATUS ntStatus = STATUS_UNSUCCESSFUL ;

        __try
        {
                RtlInitUnicodeString(&ustrObjectNames, L"\\Registry\\Machine\\SAM\\SAM\\Domains\\Account\\Users\\Names") ;

                InitializeObjectAttributes(&ObjAttributeNames, 
                                                        &ustrObjectNames, 
                                                        OBJ_CASE_INSENSITIVE, 
                                                        NULL, 
                                                        NULL) ;

                // �ȴ�
                ntStatus = ZwOpenKey(&hNamesKey, KEY_ALL_ACCESS, &ObjAttributeNames) ;
                if (! NT_SUCCESS(ntStatus))
                {
                        DbgPrint(("KernelUserManage::ReadKernelUser ZwOpenKey failed!")) ;
                        __leave ;
                }

                ULONG uSize ;
                // ��һ���������С
                ntStatus = ZwQueryKey(hNamesKey, KeyFullInformation, NULL, 0, &uSize) ;

                pKeyFullInfor = (PKEY_FULL_INFORMATION)ExAllocatePool( PagedPool, uSize) ;
                if (NULL == pKeyFullInfor)
                {
                        DbgPrint(("KernelUserManage::ReadKernelUser ExAllocatePool failed!")) ;
                        __leave ;
                }
                // �����ڴ��Ϳ�������
                ntStatus = ZwQueryKey(hNamesKey, KeyFullInformation, pKeyFullInfor, uSize, &uSize) ;
                if (! NT_SUCCESS(ntStatus))
                {
                        DbgPrint(("KernelUserManage::ReadKernelUser ZwOpenKey failed!")) ;
                        __leave ;
                }

                for (ULONG i(0); i < pKeyFullInfor->SubKeys; ++i)
                {
                        ZwEnumerateKey(hNamesKey, i, KeyBasicInformation, NULL, 0, &uSize) ;
                        pBi = (PKEY_BASIC_INFORMATION)ExAllocatePool(PagedPool, uSize) ;
                        ZwEnumerateKey(hNamesKey, i,KeyBasicInformation,pBi,uSize, &uSize) ;

                        PKERNELUSER pNode = (PKERNELUSER)ExAllocateFromPagedLookasideList(&g_PagedLookasideList) ;
                        if (NULL == pNode)
                        {
                                DbgPrint(("KernelUserManage::ReadKernelUser ExAllocateFromPagedLookasideList failed!")) ;
                                __leave ;
                        }

                        // �����ַ���
                        USHORT uNameLength = (USHORT)pBi->NameLength ;
                        pNode->ustrUserName.Length = uNameLength ;
                        pNode->ustrUserName.MaximumLength = uNameLength ;
                        pNode->ustrUserName.Buffer = (PWCHAR)ExAllocatePool(PagedPool, uNameLength) ;
						
                        if (NULL == pNode->ustrUserName.Buffer)
                        {
                                ExFreeToPagedLookasideList(&g_PagedLookasideList, pNode) ;
                                DbgPrint(("KernelUserManage::ReadKernelUser ExAllocateFromPagedLookasideList failed!")) ;
                                __leave ;
                        }

                        // ���0
                        RtlFillMemory(pNode->ustrUserName.Buffer, uNameLength, 0) ;

                        // ������copy��ȥ
                        RtlCopyMemory(pNode->ustrUserName.Buffer, pBi->Name, uNameLength) ;

                        KdPrint(("The %d sub item name: %wZ\n", i, &pNode->ustrUserName)) ;

                        // ������֮��û����ʲô���ˣ����ڴ������
                        ExFreePool(pBi) ;
                        pBi = NULL ;

                        // ��users������Ӽ���Ȼ���ֵ��ת����ֵ
                        OBJECT_ATTRIBUTES ObjAttribSub = {0};
                        InitializeObjectAttributes(&ObjAttribSub, 
                                                                &(pNode->ustrUserName), 
                                                                OBJ_CASE_INSENSITIVE,
                                                                hNamesKey, 
                                                                NULL) ;

                        ntStatus = ZwOpenKey(&hSubKey, KEY_ALL_ACCESS, &ObjAttribSub) ;
                        if (! NT_SUCCESS(ntStatus))
                        {
                                ExFreeToPagedLookasideList(&g_PagedLookasideList, pNode) ;
                                DbgPrint(("KernelUserManage::ReadKernelUser ExAllocateFromPagedLookasideList failed!")) ;
                                __leave ;
                        }

                        // Ȼ��ȥ��ֵ��
                        KEY_VALUE_BASIC_INFORMATION keyVbi = {0} ;  // ��ֵһ�㲻��̫���

                        // ��Ĭ��ֵunicode_string�����Ϳ�����
                        UNICODE_STRING ustrDefaultString = {0} ;
                        RtlInitUnicodeString(&ustrDefaultString, L"") ;
                        uSize = 0 ;

                        ntStatus = ZwQueryValueKey(hSubKey, 
                                                                        &ustrDefaultString, 
                                                                        KeyValueBasicInformation, 
                                                                        (PVOID)&keyVbi, 
                                                                        sizeof(KEY_VALUE_BASIC_INFORMATION), 
                                                                        &uSize) ;
                        if (NT_SUCCESS(ntStatus))
                        {
                                pNode->UID = keyVbi.Type ;
                        }

                        // ���뵽������ȥ
                        if (NULL == g_pKernelUser)
                        {
                                InitializeListHead(&(pNode->ListEntry)) ;
                                g_pKernelUser = pNode ;
                        }
                        else
                        {
                                InsertTailList(&(g_pKernelUser->ListEntry), &(pNode->ListEntry)) ;
                        }
                        bResult = TRUE ;
                }
        }

        __finally
        {
                if (NULL != hSubKey)
                {
                        ZwClose(hSubKey) ;
                }
                if (NULL != hNamesKey)
                {
                        ZwClose(hNamesKey) ;
                }
                if (NULL != pKeyFullInfor)
                {
                        ExFreePool(pKeyFullInfor) ;
                        pKeyFullInfor = NULL ;
                }
                if (NULL != pBi)
                {
                        ExFreePool(pBi) ;
                        pBi = NULL ;
                }
        }

        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : UserIsExist
*  �������� : ͨ��SID���һ���ж��û��Ƿ����
*  �����б� : ��
*   ˵      �� : 
*  ���ؽ�� :  ������ڣ�����TRUE, ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL UserIsExist(IN ULONG uUID)
{
        BOOL bResult = FALSE ;
        UNREFERENCED_PARAMETER(uUID) ;
        __try
        {
                if (NULL == g_pKernelUser)
                {
                        KdPrint(("KernelUsermanage::UserIsExist g_pKernelUser is NULL!")) ;
                        __leave ;
                }
                PLIST_ENTRY pList = NULL ;
                PKERNELUSER pNode = g_pKernelUser ;

                do 
                {
                        if (uUID == pNode->UID)
                        {
                                bResult = TRUE ;
                                __leave ;
                        }

                        pList = pNode->ListEntry.Flink ;
                        pNode = (PKERNELUSER)CONTAINING_RECORD(pList, KERNELUSER, ListEntry) ;

                } while (pNode != g_pKernelUser) ;
        }

        __finally
        {
        }

        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : GetUserNameByUID
*  �������� : ͨ��SIDȡ���û���
*  �����б� : ��
*   ˵      �� : 
*  ���ؽ�� :  ������ڣ������û���unicode_string�û���, ���򷵻�NULL
*
*******************************************************************************/
PUNICODE_STRING GetUserNameByUID(IN ULONG uUID)
{
        UNREFERENCED_PARAMETER(uUID) ;

        if (NULL == g_pKernelUser)
        {
                KdPrint(("KernelUserManage::UserIsExist g_pKernelUser is NULL!")) ;
                return NULL ;
        }

        PLIST_ENTRY pList = NULL ;
        PKERNELUSER pNode = g_pKernelUser ;

        do 
        {
                if (uUID == pNode->UID)
                {
                        return &(pNode->ustrUserName) ;
                }

                pList = pNode->ListEntry.Flink ;
                pNode = (PKERNELUSER)CONTAINING_RECORD(pList, KERNELUSER, ListEntry) ;
        } while (pNode != g_pKernelUser) ;

        return NULL ;
}

/*******************************************************************************
*
*   �� �� �� : GetProcessSID
*  �������� : ȡ�ý��������û�SID
*  �����б� : dwPID       --     ����PID
*                  pustrProcessSID    -- �����������SIDֵ
*   ˵      �� : pustrProcessSID��Ҫ�Լ��ͷ�
*  ���ؽ�� :  �ɹ�����TRUE��ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL GetProcessSID(IN ULONG dwPID, INOUT PUNICODE_STRING pustrProcessSID)
{
        BOOL bResult = FALSE ;
        NTSTATUS ntStatus = STATUS_UNSUCCESSFUL ;
        PEPROCESS pEprocess = NULL ;
        //HANDLE         hProcess = NULL ;  
        HANDLE         TokenHandle = NULL;  
        ULONG         ReturnLength;  
        ULONG       uSize; 
        UNICODE_STRING SidString; 
        PTOKEN_USER pTokenInformation = NULL;  
        WCHAR SidStringBuffer[64] ; 

        __try
        {
                if (0 == dwPID)
                {
                        KdPrint(("KernelUserManage::GetProcessSID dwPID can't 0!\r\n")) ;
                        __leave ;
                }
                
                if (NULL == pustrProcessSID)
                {
                        KdPrint(("KernelUserManage::GetProcessSID pustrProcessSID can't NULL!\r\n")) ;
                        __leave ;
                }

                ntStatus = PsLookupProcessByProcessId((HANDLE)dwPID,  &pEprocess) ;
                if (! NT_SUCCESS(ntStatus))
                {
                        KdPrint(("KernelUserManage::GetProcessSID PsLookupProcessByProcessId failed!\r\n")) ;
                        __leave ;
                }
                // ���ص�Ŀ����̣������ſ���ȡ������sid
                KeAttachProcess(pEprocess) ;

                //ntStatus = ZwOpenThreadTokenEx (NtCurrentThread(), 
                //                                                        TOKEN_READ, 
                //                                                        TRUE, 
                //                                                        OBJ_KERNEL_HANDLE, 
                //                                                        &TokenHandle); 
                //if (!NT_SUCCESS(ntStatus))
                //{
                //        KdPrint(("KernelUserManage::GetProcessSID ZwOpenThreadTokenEx failed!\r\n")) ;
                //        __leave ;
                //}

               ntStatus = ZwOpenProcessTokenEx (NtCurrentProcess(), 
                                                                        GENERIC_READ, 
                                                                        OBJ_KERNEL_HANDLE, 
                                                                        &TokenHandle); 

                if ( !NT_SUCCESS( ntStatus ))
                {
                        KdPrint(("KernelUserManage::GetProcessSID ZwOpenProcessTokenEx failed!\r\n")) ;
                        __leave; 
                }

                ntStatus = ZwQueryInformationToken( TokenHandle,  
                                                                                TokenUser,  
                                                                                NULL,  
                                                                                0,  
                                                                                &ReturnLength ); 

                uSize = ReturnLength; 
                pTokenInformation = (PTOKEN_USER)ExAllocatePool( NonPagedPool, uSize ); 

                ntStatus = ZwQueryInformationToken( TokenHandle,  
                                                                                TokenUser,  
                                                                                pTokenInformation,  
                                                                                uSize,  
                                                                                &ReturnLength ); 
                if (! NT_SUCCESS(ntStatus))
                {
                        KdPrint(("KernelUserManage::GetProcessSID NtQueryInformationToken failed!\r\n")) ;
                        __leave ;
                }

                RtlZeroMemory( SidStringBuffer, sizeof(SidStringBuffer) );  
                SidString.Buffer = (PWCHAR)SidStringBuffer;  
                SidString.MaximumLength = sizeof( SidStringBuffer );  

                ntStatus = RtlConvertSidToUnicodeString( &SidString,  
                                                                                ((PTOKEN_USER)pTokenInformation)->User.Sid,  
                                                                                FALSE );  

                pustrProcessSID->Length = SidString.Length ;
                pustrProcessSID->MaximumLength = SidString.Length ;
                pustrProcessSID->Buffer = (PWCHAR)ExAllocatePool(PagedPool, SidString.Length) ;
                RtlCopyUnicodeString(pustrProcessSID, &SidString) ;
                
                bResult = TRUE ;

        }

        __finally
        {
                if (NULL != pEprocess)
                {
                        KeDetachProcess() ;
                        ObDereferenceObject(pEprocess) ;
                        pEprocess = NULL ;
                }
                if (NULL != TokenHandle)
                {
                        ZwClose(TokenHandle) ;
                        TokenHandle = NULL ;
                }
                if (NULL != pTokenInformation)
                {
                        ExFreePool(pTokenInformation) ;
                        pTokenInformation = NULL ;
                }
        }

        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : GetProcessUID
*  �������� : ȡ�ý����������û�id
*  �����б� : dwPID       --     ����PID
*   ˵      �� : ʧ�ܵĻ�������system�û�
*  ���ؽ�� :  �ɹ�����UID��ʧ�ܷ���0
*
*******************************************************************************/
const ULONG uStartIndex = 41 ;
ULONG GetProcessUID(IN ULONG dwPID)
{
        ULONG uResult = 0 ;
        UNICODE_STRING ustrSID = {0} ;

        __try
        {
                if (! GetProcessSID(dwPID, &ustrSID))
                {
                        KdPrint(("KernelUserManage::GetProcessUID GetProcessUID failed!\r\n")) ;
                        __leave ;
                }

                // ���ж�sid�ĳ���
                if (ustrSID.Length <= 82)
                {
                        KdPrint(("KernelUserManage::GetProcessUID ustrSID.Length too small!\r\n")) ;
                        RtlFreeUnicodeString(&ustrSID) ;
                        __leave ;
                }
                for (ULONG i(0); (i + uStartIndex) * 2 < ustrSID.Length; i++)
                {
                        uResult = uResult * 10 + ustrSID.Buffer[i + uStartIndex] - L'0' ;
                }
        }

        __finally
        {
                if(NULL != ustrSID.Buffer)
                {
                        RtlFreeUnicodeString(&ustrSID) ;
                }
        }
        
        return uResult ;
}
