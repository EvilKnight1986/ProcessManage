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
*   函 数 名 : DriverLoad
*  功能描述 : 驱动加载
*  参数列表 : pszServerName --    服务名
*                  pszSysPath       --     驱动路径
*   说      明 : 如果服务存在的话，DriverLoad失败的话，外面可以再重试一次
*  返回结果 : 成功返回true,失败返回false
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
*   函 数 名 : DriverUnload
*  功能描述 : 驱动卸载
*  参数列表 : pszServerName --    服务名
* 
*   说      明 : 
*  返回结果 : 成功返回true,失败返回false
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
*   函 数 名 : DriverStart
*  功能描述 : 驱动启动
*  参数列表 : pszServerName --    服务名
* 
*   说      明 : 
*  返回结果 : 成功返回true,失败返回false
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
*   函 数 名 : DriverStop
*  功能描述 : 驱动停止
*  参数列表 : pszServerName --    服务名
* 
*   说      明 : 
*  返回结果 : 成功返回true,失败返回false
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