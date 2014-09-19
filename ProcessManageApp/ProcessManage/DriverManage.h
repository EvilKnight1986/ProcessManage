#pragma once

void OutputErrorInformation(PTCHAR pErrorInfo) ;

// 驱动加载
BOOL DriverLoad(PTCHAR pszServerName,  PTCHAR pszSysPath) ;

// 驱动卸载
BOOL DriverUnload(PTCHAR pszServerName) ;

// 驱动启动
BOOL DriverStart(PTCHAR pszServerName) ;

// 驱动停止
BOOL DriverStop(PTCHAR pszServerName) ;