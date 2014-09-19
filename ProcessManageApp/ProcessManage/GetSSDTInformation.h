/*******************************************************************************
*  
*  Copyright (c) all 2010 ��� All rights reserved
*  FileName : DllExportFun.h
*  D a t e  : 2010.11.30
*  ��   ��  : ȡ��SSDT�����Ϣ,�������Լ�������
*  ˵   ��  : ͨ������ntdll.dll�ĵ�����,�ҵ�Nt��ͷ�ĺ���,���жϺ����Ŀ�ͷ�ǲ���
*             mov eax, SSDT ID ���ж��ǲ���SSDT�еĺ���,���ɴ˵õ�����������
*             ����mov eax,�����ֵ
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
        // ͨ��������ַȡ�÷���������
        int  GetServiceID(int nFunctionAddress) ;
        // ���ļ��������ڴ�ӳ��
        BOOL OpenFileAndCreateMap(void) ;
        // RVA��ַת�ļ�ƫ�Ƶ�ַ
        int  RVAToFileOffset(int nRVA);

        BOOL FindFirst(OUT char *pServiceName,OUT int &nServiceID);
        BOOL FindNext (OUT char *pServiceName,OUT int &nServiceID);
        // �ͷ���Դ
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
