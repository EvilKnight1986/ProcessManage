#include "stdafx.h"
#include "PublicHead.h"
#include "publicFunction.h"
#include "GetSSDTInformation.h"

#pragma comment(lib, "ntdll.lib")

// 单字节拷备文件
// pszDescFile   -- 目标文件路径
// pszSourceFile -- 源文件路径
BOOL MyCopyFile(TCHAR *pszDescFile , TCHAR *pszSourceFile)
{
    if (NULL == pszSourceFile || NULL == pszDescFile)
    {
        return FALSE ;
    }

    HANDLE hSourceFile = CreateFile(pszSourceFile, 
									GENERIC_READ,
									FILE_SHARE_READ, 
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL) ;

    if (INVALID_HANDLE_VALUE == hSourceFile)
    {
        return FALSE ;
    }

    HANDLE hDescFile   = CreateFile(pszDescFile, 
									GENERIC_WRITE, 
									FILE_SHARE_WRITE,
									NULL, 
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
    if (INVALID_HANDLE_VALUE == hDescFile)
    {
        goto FUN_EXIT ;
    }

    // 开始一字节一字节的往里面写了
    // 把可信的程序写进去
    SetFilePointer(hSourceFile, 0, NULL, FILE_BEGIN);
    SetFilePointer(hDescFile, 0, NULL, FILE_BEGIN);

    char chBuf = 0 ;
    DWORD dwBytes = 0 ;

    while (ReadFile(hSourceFile, &chBuf, 1, &dwBytes, NULL) && dwBytes == 1)
    {
        WriteFile(hDescFile, &chBuf, 1, &dwBytes, NULL);
    }
FUN_EXIT:
    if(INVALID_HANDLE_VALUE != hSourceFile)
    {
        CloseHandle(hSourceFile) ;
        hSourceFile = INVALID_HANDLE_VALUE ;
    }
    if(INVALID_HANDLE_VALUE != hDescFile)
    {
        CloseHandle(hDescFile) ;
        hDescFile = INVALID_HANDLE_VALUE ;
    }

    return TRUE;
}

DWORD CALLBACK ThreadNull(LPVOID)
{
    __asm
    {
        xor eax, eax;
NULL_LOOP:
        dec eax;
        jnz NULL_LOOP;
    }
    return 0;
}

void AntiVM(void)
{
    __asm
    {
        rdtsc;
        push edx;
        //-------------------
        push 0;
        push 0;
        push 0;
        push offset ThreadNull;
        push 0;
        push 0;
        call dword ptr CreateThread;
        //-------------------
        push 5000;
        call dword ptr Sleep;
        //-------------------
        rdtsc;
        pop eax;
        cmp eax, edx;
        jnz NOVM;
        __emit 0xCC;
        mov eax, dword ptr ExitProcess;
        inc eax;
        jmp eax;
NOVM:
    }
}

/*******************************************************************************
*
*   函 数 名 : AnsiToUnicode
*  功能描述 : Ansi字符串转Unicode字符串
*  参数列表 : str    --  Ansi字符串
*   说      明 : 返回的Unicode字符串需要delete []释放
*  返回结果 : 成功返回Unicode字符串,失败返回NULL
*
*******************************************************************************/
wchar_t* AnsiToUnicode(IN char *str)
{
	DWORD dwNum = 0 ;
	wchar_t *pwText;
    if (NULL == str)
    {
        return NULL ;
    }

    dwNum = MultiByteToWideChar (CP_ACP, 0, str, -1, NULL, 0);
	pwText = new wchar_t[dwNum];
	if(!pwText)
	{
		delete []pwText;
	}
	MultiByteToWideChar (CP_ACP, 0, str, -1, pwText, dwNum);
	return pwText;
}

// 定义一些使用NtQuerySystemInformation函数需要用到的一些结构体等相关信息
typedef LONG NTSTATUS;
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define STATUS_ACCESS_DENIED        ((NTSTATUS)0xC0000022L)

extern "C"
NTSYSAPI
NTSTATUS
NTAPI
NtQuerySystemInformation(   IN SYSTEM_INFORMATION_CLASS SysInfoClass,
                         IN OUT PVOID SystemInformation,
                         IN ULONG SystemInformationLength,
                         OUT PULONG RetLen
                         );

typedef struct _SYSTEM_MODULE_INFORMATION {//Information Class 11
    ULONG    Reserved[2];
    PVOID    Base;
    ULONG    Size;
    ULONG    Flags;
    USHORT    Index;
    USHORT    Unknown;
    USHORT    LoadCount;
    USHORT    ModuleNameOffset;
    CHAR    ImageName[256];
}SYSTEM_MODULE_INFORMATION,*PSYSTEM_MODULE_INFORMATION;

typedef struct {
    DWORD    dwNumberOfModules;
    SYSTEM_MODULE_INFORMATION    smi;
} MODULES, *PMODULES;

/*******************************************************************************
*
*   函 数 名 : GetKernelNameA
*  功能描述 : 取得当前运行系统内核路径名
*  参数列表 : KernelName    --  用来接收当前运行系统内核路径名
*   说      明 : 返回的KernelName为Ansi版本
*  返回结果 : 成功返回TRUE,否则返回FALSE
*
*******************************************************************************/
BOOL GetKernelNameA(OUT char * KernelName) 
{
    BOOL bResult = FALSE ;
    DWORD    dwNeededSize,rc;
    PMODULES    pModules=(PMODULES)&pModules;
    rc=NtQuerySystemInformation(SystemModuleInformation,pModules,4,&dwNeededSize);
    if (rc==STATUS_INFO_LENGTH_MISMATCH) //如果内存不够
    {
        pModules=(PMODULES)GlobalAlloc(GPTR,dwNeededSize) ; //重新分配内存
        rc=NtQuerySystemInformation(SystemModuleInformation,pModules,dwNeededSize,NULL); //系统内核文件是总是在第一个，枚举1次
    } 
    if (!NT_SUCCESS(rc))
    {
        TRACE("NtQuerySystemInformation() Failed !\n"); //NtQuerySystemInformation执行失败，检查当前进程权限
        return bResult;
    }

    strcpy_s(KernelName,MAX_PATH,  pModules->smi.ImageName + pModules->smi.ModuleNameOffset) ;
    bResult = TRUE ;
    return bResult ;
}

/*******************************************************************************
*
*   函 数 名 : GetKernelNameW
*  功能描述 : 取得当前运行系统内核路径名
*  参数列表 : KernelName    --  用来接收当前运行系统内核路径名
*   说      明 : 返回的KernelName为Unicode版本
*  返回结果 : 成功返回TRUE,否则返回FALSE
*
*******************************************************************************/
BOOL GetKernelNameW(OUT WCHAR *KernelName) 
{
    char szBuf[MAX_PATH] = {0} ;
    BOOL bResult = GetKernelNameA(szBuf) ;       // 代码重用呀，调用完Ansi版，直接将结果转UNICODE

    // 对于内存的操作一定要检查
    if (bResult && KernelName != NULL)
    {
        wchar_t *pUni = AnsiToUnicode(szBuf) ;
        if(NULL != pUni)
        {
            wcscpy_s(KernelName, MAX_PATH, pUni) ;
            bResult = TRUE ;
        }
        delete [] pUni ;
    }
    return bResult ;
}

typedef ULONG   ACCESS_MASK;
typedef DWORD    ACCESS_MASK ;
const int DEF_BUF_SIZE(1024) ;
/*******************************************************************************
*
*   函 数 名 : GetProcessList
*  功能描述 : 取得进程列表
*  参数列表 :
*   说      明 : 需要调用者手动释放内存
*  返回结果 : 成功返回PSYSTEM_PROCESSES结构数据,否则返回NULL
*
*******************************************************************************/
LPBYTE GetProcessList(void)
{
    NTSTATUS    status ;
    UINT        nSize = DEF_BUF_SIZE ;
    LPBYTE        lpBuf = NULL ;

    // 由于事先并不知道需要多少空间来存储进程信息
    // 因而采用循环测试法，
    while ( TRUE )
    {
        // 动态分配空间，用来存储进程信息
        if ( ( lpBuf = new BYTE [ nSize ] ) == NULL )
        {
            return NULL;
        }

        // 枚举进程信息
        status = NtQuerySystemInformation ( SystemProcessesAndThreadsInformation, lpBuf, nSize, 0 ) ;
        if ( !NT_SUCCESS(status) )
        {
            // 检测是否返回缓冲区不够大
            if ( status == STATUS_INFO_LENGTH_MISMATCH )
            {
                nSize += DEF_BUF_SIZE ;
                delete [] lpBuf ;
                continue ;
            }
            else
            {
                return NULL;
            }
        }
        else
            break ;
    }

    PSYSTEM_PROCESSES pSysProcess = (PSYSTEM_PROCESSES)lpBuf ;
    while ( pSysProcess->NextEntryDelta != 0 )
    {
        if ( pSysProcess->ProcessName.Buffer != NULL )
        {
            printf ( "ProcessName:\t%30S\n", pSysProcess->ProcessName.Buffer ) ;
        }
        printf ("InheritedFromProcessId:\t\t%d\n",pSysProcess->InheritedFromProcessId);
        printf ("ProcessId:\t\t\t%d\n",pSysProcess->ProcessId);
        printf("HandleCount:\t\t\t%d\n",pSysProcess->HandleCount);
        printf("ThreadCount:\t\t\t%d\n",pSysProcess->ThreadCount);
        pSysProcess = (PSYSTEM_PROCESSES)( (DWORD)pSysProcess + pSysProcess->NextEntryDelta ) ;
    }
    return lpBuf ;
}

// 提权用的
BOOL Purview(void)
{
    HANDLE handle ;

    if (0 == ::OpenProcessToken(::GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_ADJUST_SESSIONID | TOKEN_ALL_ACCESS,
        &handle) )
    {
        return FALSE ;
    }

    LUID luid ;
    if (0 == ::LookupPrivilegeValue(NULL, TEXT("SeDebugPrivilege"), &luid) )
    {
        // 如果不成功，把先前打开的句柄关掉
        CloseHandle( handle) ;
        return FALSE ;
    }

    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (0 == ::AdjustTokenPrivileges( handle, FALSE, &tkp, 
        sizeof(TOKEN_PRIVILEGES),NULL, NULL))
    {
        ::CloseHandle( handle) ;
        return FALSE ;
    }

    ::CloseHandle( handle) ;
    return TRUE ;
}

/*******************************************************************************
*
*   函 数 名 : GetSSDTFunctionIndex
*  功能描述 : 取得SSDT函数索引值
*  参数列表 : NtFunctionName --    Nt*函数名
*                  puIndex               --     接收索引值
*   说      明 : 
*  返回结果 : 成功返回true,失败返回false
*
*******************************************************************************/
bool GetSSDTFunctionIndex(char *NtFunctionName, PULONG puIndex)
{
        bool bResult = false ;
        GetSSDTInformation ssdt ;
        char szServiceName[MAXBYTE] = {0} ;
        int  nServiceID = 0 ;
        int nCount = 0 ;

        if (NULL == NtFunctionName
                || NULL == puIndex)
        {
                return bResult ;
        }
        

        if (FALSE == ssdt.FindFirst(szServiceName, nServiceID))
        {
                ssdt.FindClose() ;
                return bResult ;
        }
        do 
        {
                // 如果找到了
                if (0 == stricmp(szServiceName, NtFunctionName))
                {
                        bResult = true ;
                        *puIndex = (ULONG)nServiceID ;
                }
        } while (TRUE == ssdt.FindNext(szServiceName, nServiceID));
        // 用完记得FindClose一下
        ssdt.FindClose() ;
        return bResult ;
}
