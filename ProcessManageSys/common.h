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

//��֧�ַ��������û������
#define NT_DEVICE_NAME                  L"\\Device\\devProcessManageSys"             // Driver Name
#define SYMBOLIC_LINK_NAME           L"\\DosDevices\\ProcessManageSys"            // Symbolic Link Name
#define WIN32_LINK_NAME              "\\\\.\\ProcessManageSys"                    // Win32 Link Name

//֧�ַ��������û������
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

// ���Ҫ�����Ľ�����
#define IOCTRL_ADD_PROTECT_NAME         MY_CTL_CODE(0)

// ���Ҫ�����Ľ���id
#define IOCTRL_ADD_PROTECT_PID              MY_CTL_CODE(1)

// ���Ҫ���صĽ���ID
#define IOCTRL_HIDE                                    MY_CTL_CODE(2)

#define IOCTRL_START_FILTER                          MY_CTL_CODE(3)

#define IOCTRL_STOP_FILTER                           MY_CTL_CODE(4)        

typedef unsigned int UINT ;

typedef struct _stHIDE_INFO
{
        ULONG uPID ;                                     // Ҫ���ؽ���id
}HIDE_INFO , *PHIDE_INFO;

typedef struct _stPROTECT_PID_INFO
{
        ULONG uPID ;                                        // Ҫ�����Ľ���id
}PROTECT_PID_INFO, *PPROTECT_PID_INFO ;

#define MAX_PROTECT_NAME_LEN (16)
typedef struct _stPROTECT_NAME_INFO
{
        UCHAR ProcessName[MAX_PROTECT_NAME_LEN] ;    // Ҫ�����Ľ���id
}PROTECT_NAME_INFO, *PPROTECT_NAME_INFO ;


//
// TODO: Add your IOCTL define here
//



//
// TODO: Add your struct,enum(public) define here
//



/* EOF */

