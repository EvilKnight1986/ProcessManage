#pragma once

void OutputErrorInformation(PTCHAR pErrorInfo) ;

// ��������
BOOL DriverLoad(PTCHAR pszServerName,  PTCHAR pszSysPath) ;

// ����ж��
BOOL DriverUnload(PTCHAR pszServerName) ;

// ��������
BOOL DriverStart(PTCHAR pszServerName) ;

// ����ֹͣ
BOOL DriverStop(PTCHAR pszServerName) ;