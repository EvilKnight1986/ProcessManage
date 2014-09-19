#include "stdafx.h"
#include "PublicHead.h"
#include "publicFunction.h"
#include "GetSSDTInformation.h"

#pragma comment(lib, "ntdll.lib")

// ���ֽڿ����ļ�
// pszDescFile   -- Ŀ���ļ�·��
// pszSourceFile -- Դ�ļ�·��
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

    // ��ʼһ�ֽ�һ�ֽڵ�������д��
    // �ѿ��ŵĳ���д��ȥ
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
*   �� �� �� : AnsiToUnicode
*  �������� : Ansi�ַ���תUnicode�ַ���
*  �����б� : str    --  Ansi�ַ���
*   ˵      �� : ���ص�Unicode�ַ�����Ҫdelete []�ͷ�
*  ���ؽ�� : �ɹ�����Unicode�ַ���,ʧ�ܷ���NULL
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

// ����һЩʹ��NtQuerySystemInformation������Ҫ�õ���һЩ�ṹ��������Ϣ
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
*   �� �� �� : GetKernelNameA
*  �������� : ȡ�õ�ǰ����ϵͳ�ں�·����
*  �����б� : KernelName    --  �������յ�ǰ����ϵͳ�ں�·����
*   ˵      �� : ���ص�KernelNameΪAnsi�汾
*  ���ؽ�� : �ɹ�����TRUE,���򷵻�FALSE
*
*******************************************************************************/
BOOL GetKernelNameA(OUT char * KernelName) 
{
    BOOL bResult = FALSE ;
    DWORD    dwNeededSize,rc;
    PMODULES    pModules=(PMODULES)&pModules;
    rc=NtQuerySystemInformation(SystemModuleInformation,pModules,4,&dwNeededSize);
    if (rc==STATUS_INFO_LENGTH_MISMATCH) //����ڴ治��
    {
        pModules=(PMODULES)GlobalAlloc(GPTR,dwNeededSize) ; //���·����ڴ�
        rc=NtQuerySystemInformation(SystemModuleInformation,pModules,dwNeededSize,NULL); //ϵͳ�ں��ļ��������ڵ�һ����ö��1��
    } 
    if (!NT_SUCCESS(rc))
    {
        TRACE("NtQuerySystemInformation() Failed !\n"); //NtQuerySystemInformationִ��ʧ�ܣ���鵱ǰ����Ȩ��
        return bResult;
    }

    strcpy_s(KernelName,MAX_PATH,  pModules->smi.ImageName + pModules->smi.ModuleNameOffset) ;
    bResult = TRUE ;
    return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : GetKernelNameW
*  �������� : ȡ�õ�ǰ����ϵͳ�ں�·����
*  �����б� : KernelName    --  �������յ�ǰ����ϵͳ�ں�·����
*   ˵      �� : ���ص�KernelNameΪUnicode�汾
*  ���ؽ�� : �ɹ�����TRUE,���򷵻�FALSE
*
*******************************************************************************/
BOOL GetKernelNameW(OUT WCHAR *KernelName) 
{
    char szBuf[MAX_PATH] = {0} ;
    BOOL bResult = GetKernelNameA(szBuf) ;       // ��������ѽ��������Ansi�棬ֱ�ӽ����תUNICODE

    // �����ڴ�Ĳ���һ��Ҫ���
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
*   �� �� �� : GetProcessList
*  �������� : ȡ�ý����б�
*  �����б� :
*   ˵      �� : ��Ҫ�������ֶ��ͷ��ڴ�
*  ���ؽ�� : �ɹ�����PSYSTEM_PROCESSES�ṹ����,���򷵻�NULL
*
*******************************************************************************/
LPBYTE GetProcessList(void)
{
    NTSTATUS    status ;
    UINT        nSize = DEF_BUF_SIZE ;
    LPBYTE        lpBuf = NULL ;

    // �������Ȳ���֪����Ҫ���ٿռ����洢������Ϣ
    // �������ѭ�����Է���
    while ( TRUE )
    {
        // ��̬����ռ䣬�����洢������Ϣ
        if ( ( lpBuf = new BYTE [ nSize ] ) == NULL )
        {
            return NULL;
        }

        // ö�ٽ�����Ϣ
        status = NtQuerySystemInformation ( SystemProcessesAndThreadsInformation, lpBuf, nSize, 0 ) ;
        if ( !NT_SUCCESS(status) )
        {
            // ����Ƿ񷵻ػ�����������
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

// ��Ȩ�õ�
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
        // ������ɹ�������ǰ�򿪵ľ���ص�
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
*   �� �� �� : GetSSDTFunctionIndex
*  �������� : ȡ��SSDT��������ֵ
*  �����б� : NtFunctionName --    Nt*������
*                  puIndex               --     ��������ֵ
*   ˵      �� : 
*  ���ؽ�� : �ɹ�����true,ʧ�ܷ���false
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
                // ����ҵ���
                if (0 == stricmp(szServiceName, NtFunctionName))
                {
                        bResult = true ;
                        *puIndex = (ULONG)nServiceID ;
                }
        } while (TRUE == ssdt.FindNext(szServiceName, nServiceID));
        // ����ǵ�FindCloseһ��
        ssdt.FindClose() ;
        return bResult ;
}
