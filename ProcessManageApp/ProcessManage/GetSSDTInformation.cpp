
#include "stdafx.h"
#include <shlwapi.h>
#include "GetSSDTInformation.h"
#pragma comment(lib, "shlwapi.lib")


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GetSSDTInformation::GetSSDTInformation():m_hFile(INVALID_HANDLE_VALUE),
        m_hMap(INVALID_HANDLE_VALUE),m_lpFile(NULL),m_pSectionHeader(NULL),
        m_nNameArrayOffset(0), m_nFunAddrArrayOffset(0),m_nOrdinalsAryOffset(0),
        m_nIndex(0)
{

}

GetSSDTInformation::~GetSSDTInformation()
{
    FindClose() ;
}

/*******************************************************************************
*
*  �� �� �� : FindFirst
*  �������� : ��ҪһЩ��ʼ������,Ȼ����ͨ��FindNextѰ�ҳ��ֵĵ�һ��SSDT����
*  �����б� : pServiceName      --     �������溯����
*             nServiceID        --     ���������������
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����TRUE,ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL GetSSDTInformation::FindFirst(OUT char *pServiceName, IN int &nServiceID)
{
    try
    {
        if (NULL == pServiceName)
        {
            OutputDebugString(_T("pServiceName is NULL!\r\n")) ;
            return FALSE ;
        }

        // �ȴ��ļ��������ڴ�ӳ��
        if( FALSE == OpenFileAndCreateMap() )
        {
            OutputDebugString(_T("OpenFileAndCreateMap fail!\r\n")) ;
            return FALSE ;
        }

        IMAGE_DOS_HEADER *pDos = (IMAGE_DOS_HEADER *)m_lpFile;

        // �ж�DOSͷ�ǲ���MZ
        if (IMAGE_DOS_SIGNATURE != pDos->e_magic)
        {
            // ��Ϊ����򿪷��ļ�,���������ڴ�ӳ��,���Գ���������Ҫ�ͷ�
            FindClose() ;
            OutputDebugString(_T("����һ����ЧPE�ļ�,DOSͷ����!\r\n")) ;
            return FALSE ;
        }

        // �ж�e_lfanew��λ��һ����ļ���λһ��Ϊ0,������������ķǷ�����
        if (pDos->e_lfanew & 0xffff0000)
        {
            FindClose() ;
            OutputDebugString(_T("����һ����ЧPE�ļ�,ָ����ȷ��ntͷ")) ;
            return FALSE ;
        }

        IMAGE_NT_HEADERS *pNt = (IMAGE_NT_HEADERS *)((char *)pDos + pDos->e_lfanew) ;

        // �ж�PE��־
        if (IMAGE_NT_SIGNATURE != pNt->Signature)
        {
            FindClose() ;
            OutputDebugString(_T("����һ����ЧPE�ļ�,NTͷ����")) ;
            return FALSE ;
        }

        // ����Ҫ�ж�һ����û�е�����
        // ���������ڵ�һ��
        if (pNt->OptionalHeader.NumberOfRvaAndSizes < 1)
        {
            FindClose() ;
            OutputDebugString(_T("û�е�����!\r\n")) ;
            return FALSE ;
        }

        // ȡ�õ������RVA��ַ
        // ��һ���ǵ�����,�ڶ����ǵ����
        DWORD ExportDirectoryOffset = (DWORD)(pNt->OptionalHeader.DataDirectory[0].VirtualAddress) ;
    
        if (0 == ExportDirectoryOffset)
        {
            FindClose() ;
            OutputDebugString(_T("������VirtualAddress�����˰�!\r\n")) ;
            return FALSE ;
        }

        // ȡ�ý�����
        m_nSectionCount = pNt->FileHeader.NumberOfSections ;

        // �ȶ�λ��NTͷ
        int nSectionAddress = (int)pNt ;

        // �����ntͷ�Ĵ�С
        int nNtHeaderSize = 4 + sizeof(IMAGE_FILE_HEADER) + pNt->FileHeader.SizeOfOptionalHeader ;

        // �õ����ڴ��е���ʼ��ַ
        nSectionAddress += nNtHeaderSize ;

        // �ٽ��ڵ�����copy����Աָ����ȥ
        if (NULL != m_pSectionHeader)
        {
            delete [] m_pSectionHeader ;
            m_pSectionHeader = NULL ;
        }

        m_pSectionHeader = new IMAGE_SECTION_HEADER[m_nSectionCount] ;

        if(NULL == m_pSectionHeader)
        {
            FindClose() ;
            OutputDebugString(_T("new IMAGE_SECTION_HEADER failed!\r\n")) ;
            return FALSE ;
        }

        memcpy(m_pSectionHeader, (char *)nSectionAddress, 
                    sizeof(IMAGE_SECTION_HEADER) * m_nSectionCount) ;

        // �����RVA��ַ��Ӧ���ļ���ַ
        ExportDirectoryOffset = RVAToFileOffset(ExportDirectoryOffset) ;
        ExportDirectoryOffset += (int)m_lpFile ;

        // ��ָ��ȥ
        IMAGE_EXPORT_DIRECTORY *pExportDir = (IMAGE_EXPORT_DIRECTORY *)ExportDirectoryOffset ;

        m_nNumberOfNames = pExportDir->NumberOfNames ;

        // ָ��������ַ
        m_nNameArrayOffset = RVAToFileOffset(pExportDir->AddressOfNames) ;
        m_nNameArrayOffset += (int)(m_lpFile) ;
        // ָ������ַ����
        m_nFunAddrArrayOffset = RVAToFileOffset(pExportDir->AddressOfFunctions) ;
        m_nFunAddrArrayOffset += (int)(m_lpFile) ;
        // ָ���������
        m_nOrdinalsAryOffset = RVAToFileOffset(pExportDir->AddressOfNameOrdinals) ;
        m_nOrdinalsAryOffset += (int)(m_lpFile) ;

        m_nIndex = 0 ;

        return FindNext(pServiceName, nServiceID) ;
    }
    catch(...)
    {
        FindClose() ;
        OutputDebugString(_T("exception !\r\n")) ;
        return FALSE ;
    }
}

/*******************************************************************************
*
*  �� �� �� : FindFirst
*  �������� : ȡ����һ��SSDT����
*  �����б� : pServiceName      --     �������溯����
*             nServiceID        --     ���������������
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����TRUE,ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL GetSSDTInformation::FindNext(char *pServiceName, int &nServiceID)
{
    try
    {
        if (NULL == pServiceName)
        {
            OutputDebugString(_T("pServiceName is NULL!\r\n")) ;
           
            return FALSE ;
        }
        BOOL bIsFind = FALSE ;

        for (; m_nIndex < m_nNumberOfNames && FALSE == bIsFind; ++m_nIndex)
        {
            // ȡ�ú�������ַ
            char *pFunName = (char *)RVAToFileOffset(*(int *)(m_nNameArrayOffset + m_nIndex * 4)) ;
            pFunName = (char *)((int)pFunName + (int)m_lpFile) ;

            // �����Nt��ͷ�Ļ�,˵������SSDT�ĺ���
            if (pFunName[0] == (char)'N' && pFunName[1] == (char)'t')
            {
                // ��Ϊ��Щ����������ŵ�����,����������Ҫͨ����������ҵ�������ַ
                // �����뿴pe��ʽ
                int nFunctionIndex = *(short *)(m_nOrdinalsAryOffset + m_nIndex * 2) ;

                // ͨ����ŵõ�������ַ
                int pFunAddress = RVAToFileOffset(*(int *)(m_nFunAddrArrayOffset + nFunctionIndex * 4)) ;
                pFunAddress += (int)m_lpFile ;

                // ��ȥȡ�÷���������
                nServiceID = GetServiceID(pFunAddress) ;

                // �������SSDT�����Ļ�,������,��ΪNt��ͷ�Ĳ�һ�����ǵ�
                if (-1 == nServiceID)
                {
                    continue ;
                }

                // �����ַ����򵥵ļ��һ��,��ֹ���������
                int nLen = strlen(pFunName) ;

                if (nLen < 1 || nLen > 50)
                {
                    OutputDebugString(_T("�û������зǷ���ͼ!\r\n")) ;
                    nServiceID = -1 ;
                    return FALSE ;
                }

                // ��������������Ŀ¼buffer��ȥ 
                memcpy(pServiceName, pFunName, nLen) ;
                pServiceName[nLen] = 0 ;

                bIsFind = TRUE ;
            }
        }
        // �ж�һ����û���ҵ������,���ݴ������ؽ��
        if (-1 != nServiceID && bIsFind)
        {
            return bIsFind ;
        }
        return FALSE ;
    }
    catch(...)
    {
        return FALSE ;
    }
}

/*******************************************************************************
*
*  �� �� �� : FindClose
*  �������� : �ͷ���Դ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����TRUE,ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL GetSSDTInformation::FindClose()
{
    if (NULL != m_lpFile)
    {
        UnmapViewOfFile(m_lpFile) ;
        m_lpFile = NULL ;
    }

    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile) ;
        m_hFile = INVALID_HANDLE_VALUE ;
    }

    if (INVALID_HANDLE_VALUE != m_hMap)
    {
        CloseHandle(m_hMap) ;
        m_hMap = INVALID_HANDLE_VALUE ;
    }

    if (NULL != m_pSectionHeader)
    {
        delete [] m_pSectionHeader ;
        m_pSectionHeader = NULL ;
    }

    // ȫ��0
    m_nFunAddrArrayOffset = m_nIndex = m_nNameArrayOffset = 
       m_nNumberOfNames = m_nOrdinalsAryOffset = 0 ;

    return TRUE ;
}

/*******************************************************************************
*
*  �� �� �� : OpenFileAndCreateMap
*  �������� : ���ļ��Ҵ����ڴ�ӳ��
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����TRUE,ʧ�ܷ���FALSE
*
*******************************************************************************/
BOOL GetSSDTInformation::OpenFileAndCreateMap(void)
{
    // �ȴ��ļ�
    TCHAR szWindowsDir[MAX_PATH] = {0} ;
    int nSize = MAX_PATH ;
    DWORD dwError = 0 ;
    
    if (0 == GetSystemDirectory(szWindowsDir, nSize))
    {
        dwError = GetLastError() ;
        OutputDebugString(_T("GetWindowsDirectory failed!\r\n")) ;
        return FALSE ;
    }
    
    PathAppend(szWindowsDir, _T("ntdll.dll")) ;
    
    // ���֮ǰ������������Դ,����û�зŵ�,����ŵ�
    FindClose() ;
    
    m_hFile = CreateFile(szWindowsDir,GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            NULL) ;
    
    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        dwError = GetLastError() ;
        OutputDebugString(_T("CreateFile failed!\r\n")) ;
        return FALSE ;
    }
    
    m_hMap = CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL) ;
    
    if (INVALID_HANDLE_VALUE == m_hMap)
    {
        dwError = GetLastError() ;
        OutputDebugString(_T("CreateFileMapping failed!\r\n")) ;
        // ���������,�ٰѴ򿪵��ļ��ص�
        FindClose() ;
        return FALSE ;
    }
    
    m_lpFile = MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0) ;
    
    if (NULL == m_lpFile)
    {
        dwError = GetLastError() ;
        OutputDebugString(_T("MapViewOfFile failed!\r\n")) ;
        // ���������,�ٰѴ򿪵��ļ��ص�
        FindClose() ;
        return FALSE ;
    }
    
    return TRUE ;
}

/*******************************************************************************
*
*  �� �� �� : RVAToFileOffset
*  �������� : RVA��ַת�ļ�ƫ�Ƶ�ַ
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����ƫ����,ʧ�ܷ���-1
*
*******************************************************************************/
int GetSSDTInformation::RVAToFileOffset(int nRVA)
{
    if (0 == nRVA || NULL == m_pSectionHeader)
    {
        return -1 ;
    }
    
    int nFileOffset = -1 ;

    for (int i = 0; i < m_nSectionCount; ++i)
    {
        if ((DWORD)nRVA >= m_pSectionHeader[i].VirtualAddress
            && (DWORD)nRVA <= m_pSectionHeader[i].VirtualAddress + m_pSectionHeader[i].Misc.VirtualSize)
        {
            nFileOffset =  nRVA - m_pSectionHeader[i].VirtualAddress;
            nFileOffset += m_pSectionHeader[i].PointerToRawData ;
            return nFileOffset ;
        }
    }

    return nFileOffset ;
}

/*******************************************************************************
*
*  �� �� �� : GetServiceID
*  �������� : ȡ��SSDT����������
*  �����б� : 
*  ˵    �� : 
*  ���ؽ�� : ����ɹ�����������,ʧ�ܷ���-1
*
*******************************************************************************/
int GetSSDTInformation::GetServiceID(int nFunctionAddress)
{
        #define MOV (0xb8)
        try
        {
        // ����Ҫ��unsigned char ,Ҫ��Ȼ�������չ
        // ����
        unsigned char *pInstruction = (unsigned char *)nFunctionAddress ;
        if (MOV != *pInstruction)
        {
            return -1 ;
        }

        int nResult = *(int*)(pInstruction + 1) ;

        return nResult ;
        }
        catch (...)
        {
        return -1 ;
        }
}
