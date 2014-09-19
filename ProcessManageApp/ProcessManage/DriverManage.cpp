#include "StdAfx.h"
#include "DriverManage.h"
#include <strsafe.h>
#include <Winsvc.h>

#pragma comment(lib, "strsafe.lib")

void OutputErrorInformation(PTCHAR pErrorInfo = NULL)
{
        WCHAR szBuffer[MAX_PATH] = {0} ;
        StringCbPrintfW(szBuffer, sizeof(szBuffer), L"Error Code: %p\r\n", GetLastError()) ;
        if (NULL != pErrorInfo)
        {
                OutputDebugString(pErrorInfo) ;
        }
        OutputDebugStringW(szBuffer) ;
}
/*******************************************************************************
*
*   �� �� �� : DriverLoad
*  �������� : ��������
*  �����б� : pszServerName --    ������
*                  pszSysPath       --     ����·��
*   ˵      �� : ���������ڵĻ���DriverLoadʧ�ܵĻ����������������һ��
*  ���ؽ�� : �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
BOOL DriverLoad(PTCHAR pszServerName,  PTCHAR pszSysPath) 
{
        BOOL bResult = FALSE ;
        SC_HANDLE hSCManage = NULL ;
        SC_HANDLE hService = NULL ;
        __try
        {
                hSCManage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
                if (NULL == hSCManage)
                {
                        OutputErrorInformation(TEXT("DriverLoad::OpenSCManager failed!\r\n")) ;
                        __leave ;
                }
                SC_HANDLE hService = CreateService(hSCManage,
                                                                                pszServerName, 
                                                                                pszServerName,
                                                                                SERVICE_ALL_ACCESS, 
                                                                                SERVICE_KERNEL_DRIVER, 
                                                                                SERVICE_DEMAND_START, 
                                                                                SERVICE_ERROR_NORMAL,
                                                                                pszSysPath, 
                                                                                NULL, 
                                                                                NULL, 
                                                                                NULL, 
                                                                                NULL,
                                                                                NULL) ;
                if (NULL == hService)
                {
                        OutputErrorInformation(TEXT("DriverLoad::CreateService failed!\r\n")) ;
                        if (ERROR_SERVICE_EXISTS == GetLastError())
                        {
                                DriverStop(pszServerName) ;
                                Sleep(1000) ;
                                DriverUnload(pszServerName) ;
                                Sleep(200) ;
                        }
                        else
                        {
                                __leave ;
                        }
                }
                bResult = TRUE ;
        }

        __finally
        {
                if (NULL != hService)
                {
                        CloseServiceHandle(hService) ;
                        hService = NULL ;
                }

                if (NULL !=hSCManage)
                {
                        CloseServiceHandle(hSCManage) ;
                        hSCManage = NULL ;
                }
        }

        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : DriverUnload
*  �������� : ����ж��
*  �����б� : pszServerName --    ������
* 
*   ˵      �� : 
*  ���ؽ�� : �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
BOOL DriverUnload(PTCHAR pszServerName)
{
        BOOL bResult = FALSE ;
        SC_HANDLE hSCManage = NULL ;
        SC_HANDLE hService = NULL ;

        __try
        {
                hSCManage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
                if (NULL == hSCManage)
                {
                        OutputErrorInformation(TEXT("DriverUnload::OpenSCManager failed!\r\n")) ;
                        __leave ;
                }

                hService = OpenService(hSCManage, pszServerName, SERVICE_ALL_ACCESS) ;
                if (NULL == hService)
                {
                        OutputErrorInformation(TEXT("DriverUnload::OpenService failed!\r\n")) ;
                        __leave ;
                }

                if( ! DeleteService(hService))
                {
                        OutputErrorInformation(TEXT("DriverUnload::DeleteService failed!\r\n")) ;
                        __leave ;
                }

                bResult = TRUE;
        }

        __finally
        {
                if (NULL != hService)
                {
                        CloseServiceHandle(hService) ;
                        hService = NULL ;
                }

                if (NULL != hSCManage)
                {
                        CloseServiceHandle(hSCManage) ;
                        hSCManage = NULL ;
                }
        }
        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : DriverStart
*  �������� : ��������
*  �����б� : pszServerName --    ������
* 
*   ˵      �� : 
*  ���ؽ�� : �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
BOOL DriverStart(PTCHAR pszServerName)
{
        BOOL bResult = FALSE ;
        SC_HANDLE hSCManage = NULL ;
        SC_HANDLE hService = NULL ;

        __try
        {
                hSCManage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
                if (NULL == hSCManage)
                {
                        OutputErrorInformation(TEXT("DriverUnload::OpenSCManager failed!\r\n")) ;
                        __leave ;
                }

                hService = OpenService(hSCManage, pszServerName, SERVICE_ALL_ACCESS) ;
                if (NULL == hService)
                {
                        OutputErrorInformation(TEXT("DriverUnload::OpenService failed!\r\n")) ;
                        __leave ;
                }

                if(FALSE == StartService(hService, NULL, NULL))
                {
                        OutputErrorInformation(TEXT("DriverUnload::StartService failed!")) ;
                        __leave ;
                }

                bResult = TRUE;
        }

        __finally
        {
                if (NULL != hService)
                {
                        CloseServiceHandle(hService) ;
                        hService = NULL ;
                }

                if (NULL != hSCManage)
                {
                        CloseServiceHandle(hSCManage) ;
                        hSCManage = NULL ;
                }
        }
        return bResult ;
}

/*******************************************************************************
*
*   �� �� �� : DriverStop
*  �������� : ����ֹͣ
*  �����б� : pszServerName --    ������
* 
*   ˵      �� : 
*  ���ؽ�� : �ɹ�����true,ʧ�ܷ���false
*
*******************************************************************************/
BOOL DriverStop(PTCHAR pszServerName)
{
        BOOL bResult = FALSE ;
        SC_HANDLE hSCManage = NULL ;
        SC_HANDLE hService = NULL ;

        __try
        {
                hSCManage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ;
                if (NULL == hSCManage)
                {
                        OutputErrorInformation(TEXT("DriverUnload::OpenSCManager failed!\r\n")) ;
                        __leave ;
                }

                hService = OpenService(hSCManage, pszServerName, SERVICE_ALL_ACCESS) ;
                if (NULL == hService)
                {
                        OutputErrorInformation(TEXT("DriverUnload::OpenService failed!\r\n")) ;
                        __leave ;
                }

                SERVICE_STATUS status = {0} ;
                if(FALSE == ControlService(hService, SERVICE_CONTROL_STOP, &status))
                {
                        OutputErrorInformation(TEXT("DriverUnload::ControlService failed!\r\n")) ;
                        __leave ;
                }

                bResult = TRUE;
        }

        __finally
        {
                if (NULL != hService)
                {
                        CloseServiceHandle(hService) ;
                        hService = NULL ;
                }

                if (NULL != hSCManage)
                {
                        CloseServiceHandle(hSCManage) ;
                        hSCManage = NULL ;
                }
        }
        return bResult ;
}