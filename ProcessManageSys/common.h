/**************************************************************************************
* AUTHOR : EvilKnight
* DATE   : 2014-7-6
* MODULE : common.h
*
* Command: 
*	IOCTRL Common Header
*
* Description:
*	Common data for the IoCtrl driver and application
*
****************************************************************************************
* Copyright (C) 2014 EvilKnight.
****************************************************************************************/

#pragma once 

//#######################################################################################
// D E F I N E S
//#######################################################################################

#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif
#define INVALID_INDEX_VALUE (0xFFFFFFFF)

//不支持符号链接用户相关性
#define NT_DEVICE_NAME                  L"\\Device\\devProcessManageSys"             // Driver Name
#define SYMBOLIC_LINK_NAME           L"\\DosDevices\\ProcessManageSys"            // Symbolic Link Name
#define WIN32_LINK_NAME              "\\\\.\\ProcessManageSys"                    // Win32 Link Name

//支持符号链接用户相关性
#define SYMBOLIC_LINK_GLOBAL_NAME    L"\\DosDevices\\Global\\ProcessManageSys"    // Symbolic Link Name

#define DATA_TO_APP                  "This string from driver to app"

//
// Device IO Control Codes
//
#define IOCTL_BASE          0x800
#define MY_CTL_CODE(i)        \
    CTL_CODE                  \
    (                         \
        FILE_DEVICE_UNKNOWN,  \
        IOCTL_BASE + i,       \
        METHOD_BUFFERED,      \
        FILE_ANY_ACCESS       \
    )

// 添加要保护的进程名
#define IOCTRL_ADD_PROTECT_NAME         MY_CTL_CODE(0)

// 添加要保护的进程id
#define IOCTRL_ADD_PROTECT_PID              MY_CTL_CODE(1)

// 添加要隐藏的进程ID
#define IOCTRL_HIDE                                    MY_CTL_CODE(2)

#define IOCTRL_START_FILTER                          MY_CTL_CODE(3)

#define IOCTRL_STOP_FILTER                           MY_CTL_CODE(4)        

typedef unsigned int UINT ;

typedef struct _stHIDE_INFO
{
        ULONG uPID ;                                     // 要隐藏进程id
}HIDE_INFO , *PHIDE_INFO;

typedef struct _stPROTECT_PID_INFO
{
        ULONG uPID ;                                        // 要保护的进程id
}PROTECT_PID_INFO, *PPROTECT_PID_INFO ;

#define MAX_PROTECT_NAME_LEN (16)
typedef struct _stPROTECT_NAME_INFO
{
        UCHAR ProcessName[MAX_PROTECT_NAME_LEN] ;    // 要保护的进程id
}PROTECT_NAME_INFO, *PPROTECT_NAME_INFO ;


//
// TODO: Add your IOCTL define here
//



//
// TODO: Add your struct,enum(public) define here
//



/* EOF */

