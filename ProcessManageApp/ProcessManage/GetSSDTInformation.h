/*******************************************************************************
*  
*  Copyright (c) all 2010 黄瀚 All rights reserved
*  FileName : DllExportFun.h
*  D a t e  : 2010.11.30
*  功   能  : 取得SSDT相关信息,索引号以及函数名
*  说   明  : 通过遍历ntdll.dll的导出表,找到Nt开头的函数,再判断函数的开头是不是
*             mov eax, SSDT ID 来判断是不是SSDT中的函数,并由此得到服务索引号
*             就是mov eax,后面个值
*
*
*******************************************************************************/

#if !defined(AFX_GETSSDTINFORMATION_H__95938184_C2D9_4E2E_9F07_27E37D521123__INCLUDED_)
#define AFX_GETSSDTINFORMATION_H__95938184_C2D9_4E2E_9F07_27E37D521123__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>

class GetSSDTInformation  
{
public:
        // 通过函数地址取得服务索引号
        int  GetServiceID(int nFunctionAddress) ;
        // 打开文件并创建内存映射
        BOOL OpenFileAndCreateMap(void) ;
        // RVA地址转文件偏移地址
        int  RVAToFileOffset(int nRVA);

        BOOL FindFirst(OUT char *pServiceName,OUT int &nServiceID);
        BOOL FindNext (OUT char *pServiceName,OUT int &nServiceID);
        // 释放资源
        BOOL FindClose(void);

        GetSSDTInformation();
        virtual ~GetSSDTInformation();
    
private:
        int m_nIndex;
        int m_nNumberOfNames;
        int m_nOrdinalsAryOffset;
        int m_nFunAddrArrayOffset;
        int m_nNameArrayOffset;
        int m_nSectionCount;
        IMAGE_SECTION_HEADER *m_pSectionHeader;
        LPVOID m_lpFile;
        HANDLE m_hMap;
        HANDLE m_hFile;
};

#endif // !defined(AFX_GETSSDTINFORMATION_H__95938184_C2D9_4E2E_9F07_27E37D521123__INCLUDED_)
