
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
*  函 数 名 : FindFirst
*  功能描述 : 主要一些初始化操作,然后再通过FindNext寻找出现的第一个SSDT函数
*  参数列表 : pServiceName      --     用来保存函数名
*             nServiceID        --     用来保存服务索引
*  说    明 : 
*  返回结果 : 如果成功返回TRUE,失败返回FALSE
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

        // 先打开文件并创建内存映射
        if( FALSE == OpenFileAndCreateMap() )
        {
            OutputDebugString(_T("OpenFileAndCreateMap fail!\r\n")) ;
            return FALSE ;
        }

        IMAGE_DOS_HEADER *pDos = (IMAGE_DOS_HEADER *)m_lpFile;

        // 判断DOS头是不是MZ
        if (IMAGE_DOS_SIGNATURE != pDos->e_magic)
        {
            // 因为上面打开发文件,并创建了内存映射,所以出错了这里要释放
            FindClose() ;
            OutputDebugString(_T("不是一个有效PE文件,DOS头不对!\r\n")) ;
            return FALSE ;
        }

        // 判断e_lfanew高位，一般的文件高位一般为0,否则会引起程序的非法访问
        if (pDos->e_lfanew & 0xffff0000)
        {
            FindClose() ;
            OutputDebugString(_T("不是一个有效PE文件,指向不正确的nt头")) ;
            return FALSE ;
        }

        IMAGE_NT_HEADERS *pNt = (IMAGE_NT_HEADERS *)((char *)pDos + pDos->e_lfanew) ;

        // 判断PE标志
        if (IMAGE_NT_SIGNATURE != pNt->Signature)
        {
            FindClose() ;
            OutputDebugString(_T("不是一个有效PE文件,NT头不对")) ;
            return FALSE ;
        }

        // 这里要判断一下有没有导出表
        // 导出表是在第一项
        if (pNt->OptionalHeader.NumberOfRvaAndSizes < 1)
        {
            FindClose() ;
            OutputDebugString(_T("没有导出表!\r\n")) ;
            return FALSE ;
        }

        // 取得导出表的RVA地址
        // 第一项是导出表,第二项是导入表
        DWORD ExportDirectoryOffset = (DWORD)(pNt->OptionalHeader.DataDirectory[0].VirtualAddress) ;
    
        if (0 == ExportDirectoryOffset)
        {
            FindClose() ;
            OutputDebugString(_T("导出表VirtualAddress错误了吧!\r\n")) ;
            return FALSE ;
        }

        // 取得节数量
        m_nSectionCount = pNt->FileHeader.NumberOfSections ;

        // 先定位到NT头
        int nSectionAddress = (int)pNt ;

        // 再算出nt头的大小
        int nNtHeaderSize = 4 + sizeof(IMAGE_FILE_HEADER) + pNt->FileHeader.SizeOfOptionalHeader ;

        // 得到在内存中的起始地址
        nSectionAddress += nNtHeaderSize ;

        // 再将节的数据copy到成员指针中去
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

        // 再求出RVA地址对应的文件地址
        ExportDirectoryOffset = RVAToFileOffset(ExportDirectoryOffset) ;
        ExportDirectoryOffset += (int)m_lpFile ;

        // 再指过去
        IMAGE_EXPORT_DIRECTORY *pExportDir = (IMAGE_EXPORT_DIRECTORY *)ExportDirectoryOffset ;

        m_nNumberOfNames = pExportDir->NumberOfNames ;

        // 指向函数名地址
        m_nNameArrayOffset = RVAToFileOffset(pExportDir->AddressOfNames) ;
        m_nNameArrayOffset += (int)(m_lpFile) ;
        // 指向函数地址数组
        m_nFunAddrArrayOffset = RVAToFileOffset(pExportDir->AddressOfFunctions) ;
        m_nFunAddrArrayOffset += (int)(m_lpFile) ;
        // 指向序号数组
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
*  函 数 名 : FindFirst
*  功能描述 : 取得下一个SSDT函数
*  参数列表 : pServiceName      --     用来保存函数名
*             nServiceID        --     用来保存服务索引
*  说    明 : 
*  返回结果 : 如果成功返回TRUE,失败返回FALSE
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
            // 取得函数名地址
            char *pFunName = (char *)RVAToFileOffset(*(int *)(m_nNameArrayOffset + m_nIndex * 4)) ;
            pFunName = (char *)((int)pFunName + (int)m_lpFile) ;

            // 如果是Nt开头的话,说不定是SSDT的函数
            if (pFunName[0] == (char)'N' && pFunName[1] == (char)'t')
            {
                // 因为有些函数是以序号导出的,所以有名的要通过这个表来找到函数地址
                // 祥情请看pe格式
                int nFunctionIndex = *(short *)(m_nOrdinalsAryOffset + m_nIndex * 2) ;

                // 通过序号得到函数地址
                int pFunAddress = RVAToFileOffset(*(int *)(m_nFunAddrArrayOffset + nFunctionIndex * 4)) ;
                pFunAddress += (int)m_lpFile ;

                // 再去取得服务索引号
                nServiceID = GetServiceID(pFunAddress) ;

                // 如果不是SSDT函数的话,往下走,因为Nt开头的不一定都是的
                if (-1 == nServiceID)
                {
                    continue ;
                }

                // 对于字符串简单的检查一下,防止缓冲区溢出
                int nLen = strlen(pFunName) ;

                if (nLen < 1 || nLen > 50)
                {
                    OutputDebugString(_T("用户可能有非法企图!\r\n")) ;
                    nServiceID = -1 ;
                    return FALSE ;
                }

                // 将函数名拷备到目录buffer中去 
                memcpy(pServiceName, pFunName, nLen) ;
                pServiceName[nLen] = 0 ;

                bIsFind = TRUE ;
            }
        }
        // 判断一下有没有找到服务号,根据此来返回结果
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
*  函 数 名 : FindClose
*  功能描述 : 释放资源
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回TRUE,失败返回FALSE
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

    // 全填0
    m_nFunAddrArrayOffset = m_nIndex = m_nNameArrayOffset = 
       m_nNumberOfNames = m_nOrdinalsAryOffset = 0 ;

    return TRUE ;
}

/*******************************************************************************
*
*  函 数 名 : OpenFileAndCreateMap
*  功能描述 : 打开文件且创建内存映射
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回TRUE,失败返回FALSE
*
*******************************************************************************/
BOOL GetSSDTInformation::OpenFileAndCreateMap(void)
{
    // 先打开文件
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
    
    // 如果之前可能申请了资源,但是没有放掉,这里放掉
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
        // 如果出错了,再把打开的文件关掉
        FindClose() ;
        return FALSE ;
    }
    
    m_lpFile = MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0) ;
    
    if (NULL == m_lpFile)
    {
        dwError = GetLastError() ;
        OutputDebugString(_T("MapViewOfFile failed!\r\n")) ;
        // 如果出错了,再把打开的文件关掉
        FindClose() ;
        return FALSE ;
    }
    
    return TRUE ;
}

/*******************************************************************************
*
*  函 数 名 : RVAToFileOffset
*  功能描述 : RVA地址转文件偏移地址
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回偏移量,失败返回-1
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
*  函 数 名 : GetServiceID
*  功能描述 : 取得SSDT函数索引号
*  参数列表 : 
*  说    明 : 
*  返回结果 : 如果成功返回索引号,失败返回-1
*
*******************************************************************************/
int GetSSDTInformation::GetServiceID(int nFunctionAddress)
{
        #define MOV (0xb8)
        try
        {
        // 这里要是unsigned char ,要不然会符号扩展
        // 谨记
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
