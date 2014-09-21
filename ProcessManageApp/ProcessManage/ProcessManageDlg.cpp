// ProcessManageDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessManage.h"
#include "ProcessManageDlg.h"
#include "publicFunction.h"
#include "ProgramName.h"
#include "./../../ProcessManageSys/common.h"
#include "DriverManage.h"
#include <winioctl.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SYS_FILE_NAME TEXT("HD_ProcessManage.sys") 
#define SYS_SERVICE_NAME TEXT("HD_ProcessManage")

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
        CAboutDlg();

// �Ի�������
        enum { IDD = IDD_ABOUTBOX };

        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
        DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CProcessManageDlg �Ի���




CProcessManageDlg::CProcessManageDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CProcessManageDlg::IDD, pParent),m_hDriver(INVALID_HANDLE_VALUE),
        m_uNtTerrminateProcessIndex(INVALID_INDEX_VALUE),
        m_uNtCreateProcessExIndex(INVALID_INDEX_VALUE)
{
        m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProcessManageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROCESSLIST, m_ProcessList);
}

BEGIN_MESSAGE_MAP(CProcessManageDlg, CDialog)
        ON_WM_SYSCOMMAND()
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
        //}}AFX_MSG_MAP
        ON_BN_CLICKED(BTN_TEST, &CProcessManageDlg::OnBnClickedTest)
        ON_BN_CLICKED(BTN_REFRESH, &CProcessManageDlg::OnBnClickedRefresh)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_PROCESSLIST, &CProcessManageDlg::OnLvnItemchangedProcesslist)
        ON_WM_DESTROY()
        ON_BN_CLICKED(BTN_HIDE, &CProcessManageDlg::OnBnClickedHide)
//    ON_WM_CREATE()
        ON_BN_CLICKED(BTN_PROTECT_PID, &CProcessManageDlg::OnBnClickedProtectPid)
        ON_BN_CLICKED(BTN_PROTECT_NAME, &CProcessManageDlg::OnBnClickedProtectName)
        ON_BN_CLICKED(BTN_START_FILTER, &CProcessManageDlg::OnBnClickedStartFilter)
        ON_BN_CLICKED(BTN_STOP_FILTER, &CProcessManageDlg::OnBnClickedStopFilter)
END_MESSAGE_MAP()


// CProcessManageDlg ��Ϣ�������

BOOL CProcessManageDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

        // ��������...���˵�����ӵ�ϵͳ�˵��С�

        // IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
        ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
        ASSERT(IDM_ABOUTBOX < 0xF000);

        CMenu* pSysMenu = GetSystemMenu(FALSE);
        if (pSysMenu != NULL)
        {
                CString strAboutMenu;
                strAboutMenu.LoadString(IDS_ABOUTBOX);
                if (!strAboutMenu.IsEmpty())
                {
                        pSysMenu->AppendMenu(MF_SEPARATOR);
                        pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
                }
        }

        // ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
        //  ִ�д˲���
        SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
        SetIcon(m_hIcon, FALSE);		// ����Сͼ��

        // TODO: �ڴ���Ӷ���ĳ�ʼ������
        // ��ʼ��״̬��
        m_StatusBar.Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, 101) ;
        int arWidth[2] = {260,-1} ;
        m_StatusBar.SetParts(2, arWidth) ;
        wchar_t szKernelName[MAX_PATH] = {0} ;

        if(GetKernelName(szKernelName))
        {
                CString str ;
                str.Format(_T("Kernel : %s"), szKernelName) ;
                m_StatusBar.SetText(str,0,0) ;
        }
        else
        {
                m_StatusBar.SetText(_T("GetKernelName failed!"),1,0) ;
        }

        m_StatusBar.SetText(TEXT("Copyright(c)2014 EvilKnight ����"),1,0) ;

        // �����б��ʼ��
        m_ProcessList.InsertColumn(0, TEXT("Process")) ;
        m_ProcessList.InsertColumn(1, TEXT("PID")) ;
        AutoSize(&m_ProcessList) ;

        // �������ԣ�һ�ο���ѡһ��
        m_ProcessList.SetExtendedStyle( m_ProcessList.GetExtendedStyle() | LVS_EX_FULLROWSELECT) ;
        // ��ʾ������
        m_ProcessList.SetExtendedStyle(m_ProcessList.GetExtendedStyle() | LVS_EX_GRIDLINES) ;

        // ��������·��
        TCHAR szSysPath[MAX_PATH] = {0} ;

        GetModuleFileName(NULL, szSysPath, MAX_PATH) ;
        PathRemoveFileSpec(szSysPath) ;
        PathAppend(szSysPath, SYS_FILE_NAME) ;


        // �ͷ������ļ�
        if(ReleaseSys(szSysPath))
        {
                // ���Զ���
                for (int i(0); i < 3; ++i)
                {
                        if(DriverLoad(SYS_SERVICE_NAME, szSysPath))
                        {
                                DriverStart(SYS_SERVICE_NAME) ;
                                break ;
                        }
                }
        }
        DeleteFile(szSysPath) ;

        

        // ����������
        m_hDriver = CreateFileA(WIN32_LINK_NAME,
                                                        GENERIC_READ | GENERIC_WRITE,
                                                        0,
                                                        NULL,
                                                        OPEN_EXISTING,
                                                        FILE_ATTRIBUTE_NORMAL,
                                                        NULL) ;
        if (INVALID_HANDLE_VALUE == m_hDriver)
        {
                GetDlgItem(BTN_HIDE)->EnableWindow(FALSE) ;
                GetDlgItem(BTN_PROTECT_NAME)->EnableWindow(FALSE) ;
                GetDlgItem(BTN_PROTECT_PID)->EnableWindow(FALSE) ;
                GetDlgItem(BTN_START_FILTER)->EnableWindow(FALSE) ;
                GetDlgItem(BTN_PROTECT_PID)->EnableWindow(FALSE) ;
                GetDlgItem(BTN_START_FILTER)->EnableWindow(FALSE) ;
                AfxMessageBox(_T("���ں˳���ʧ�ܣ������ں˳����Ƿ��Ѿ����أ�")) ;
        }

        OnBnClickedRefresh() ;

        //GetSSDTFunctionIndex("NtQuerySystemInformation", &m_uNtTerrminateProcessIndex) ;
        //GetSSDTFunctionIndex("ZwQuerySystemInformation", &m_uNtCreateProcessExIndex) ;
        //GetSSDTFunctionIndex("NtOpenProcess", &m_uNtOpenProcessIndex) ;


        return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CProcessManageDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
        if ((nID & 0xFFF0) == IDM_ABOUTBOX)
        {
                CAboutDlg dlgAbout;
                dlgAbout.DoModal();
        }
        else
        {
                CDialog::OnSysCommand(nID, lParam);
        }
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CProcessManageDlg::OnPaint()
{
        if (IsIconic())
        {
                CPaintDC dc(this); // ���ڻ��Ƶ��豸������

                SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

                // ʹͼ���ڹ����������о���
                int cxIcon = GetSystemMetrics(SM_CXICON);
                int cyIcon = GetSystemMetrics(SM_CYICON);
                CRect rect;
                GetClientRect(&rect);
                int x = (rect.Width() - cxIcon + 1) / 2;
                int y = (rect.Height() - cyIcon + 1) / 2;

                // ����ͼ��
                dc.DrawIcon(x, y, m_hIcon);
        }
        else
        {
                CDialog::OnPaint();
        }
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CProcessManageDlg::OnQueryDragIcon()
{
        return static_cast<HCURSOR>(m_hIcon);
}
void CProcessManageDlg::OnBnClickedTest()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������

}

/*******************************************************************************
*
*   �� �� �� : AutoSize
*  �������� : �Զ�����List title���
*  �����б� : pList    --  ָ��CListCtrl����
*   ˵      �� : 
*  ���ؽ�� : ��
*
*******************************************************************************/
void CProcessManageDlg::AutoSize(CListCtrl * pList)
{
        if (NULL == pList)
        {
                return ;
        }

        CHeaderCtrl *pHead = pList->GetHeaderCtrl() ;
        int nCount =  pHead->GetItemCount() ;

        pList->SetRedraw(FALSE) ;
        for (int i = 0; i < nCount; ++i)
        {
                pList->SetColumnWidth(i, -1) ;
                int nWidth1 = pList->GetColumnWidth(i) ;

                pList->SetColumnWidth(i, -2) ;
                int nWidth2 = pList->GetColumnWidth(i) ;

                pList->SetColumnWidth(i, max(nWidth1, nWidth2)) ;
        }
        pList->SetRedraw(TRUE) ;
        return ;
}
void CProcessManageDlg::OnBnClickedRefresh()
{
        // TODO: �ڴ���ӿؼ�֪ͨ����������
        LPBYTE pbuf =  GetProcessList() ;
        PSYSTEM_PROCESSES pSysProcess = (PSYSTEM_PROCESSES)pbuf ;
        CString str ;
        int i(0) ;

        // ��������е������Լ�ȡ���ػ�
        m_ProcessList.DeleteAllItems() ;
        m_ProcessList.SetRedraw(FALSE) ;

        if(NULL == pSysProcess)
        {
                MessageBox(_T("�������̴���!"), _T("Error"), MB_ICONERROR) ;
                return ;
        }

        while (TRUE)
        {
                // �����������0
                if (0 ==pSysProcess->ProcessId && NULL == pSysProcess->ProcessName.Buffer )
                {
                        m_ProcessList.InsertItem(i, _T("System Idle Process")) ;
                        m_ProcessList.SetItemText(i, 1, _T("0")) ;
                        m_ProcessList.SetItemData(i, 0) ;
                }
                else
                {
                        if ( pSysProcess->ProcessName.Buffer != NULL )
                        {
                                m_ProcessList.InsertItem(i, pSysProcess->ProcessName.Buffer) ;
                        }
                        else
                        {
                                m_ProcessList.InsertItem(i, _T("Get Process Name filed!")) ;
                        }
                        str.Format(_T("%d"), pSysProcess->ProcessId) ;
                        m_ProcessList.SetItemText(i, 1, str) ;
                        m_ProcessList.SetItemData(i, pSysProcess->ProcessId) ;
                        /*
                        printf ("InheritedFromProcessId:\t\t%d\n",pSysProcess->InheritedFromProcessId);
                        printf ("ProcessId:\t\t\t%d\n",pSysProcess->ProcessId);
                        printf("HandleCount:\t\t\t%d\n",pSysProcess->HandleCount);
                        printf("ThreadCount:\t\t\t%d\n",pSysProcess->ThreadCount);
                        */
                }

                if (0 == pSysProcess->NextEntryDelta )
                {
                        break ;
                }
                pSysProcess = (PSYSTEM_PROCESSES)((DWORD)pSysProcess + pSysProcess->NextEntryDelta) ;
                ++i ;

        }

        if (NULL != pbuf)
        {
                delete [] pbuf ;
                pbuf = NULL ;
        }

        AutoSize(&m_ProcessList) ;
        m_ProcessList.SetRedraw(TRUE) ;
}

void CProcessManageDlg::OnLvnItemchangedProcesslist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    *pResult = 0;
}

void CProcessManageDlg::OnDestroy()
{
        CDialog::OnDestroy();

        // TODO: �ڴ˴������Ϣ����������

        // �ͷŵ�֮ǰ����Դ
        if(INVALID_HANDLE_VALUE != m_hDriver)
        {
        CloseHandle(m_hDriver) ;
        m_hDriver = INVALID_HANDLE_VALUE ;
        }

        DriverStop(SYS_SERVICE_NAME) ;
        Sleep(1000) ;
        DriverUnload(SYS_SERVICE_NAME) ;
        Sleep(500) ;
}

void CProcessManageDlg::OnBnClickedHide()
{
        // TODO: �ڴ���ӿؼ�֪ͨ����������
        // �ȰѰ�ť�ûң�ʡ���û��ҵ����
        GetDlgItem(BTN_HIDE)->EnableWindow(FALSE) ;

        POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();
        DWORD dwBytesReturned = 0 ;
        if(NULL == pos)
        {
                AfxMessageBox(_T("��ѡ��Ҫ���صĽ���")) ;
                GetDlgItem(BTN_HIDE)->EnableWindow(TRUE) ;
                return ;
        }
        int nIndex = m_ProcessList.GetNextSelectedItem(pos) ;

        HIDE_INFO hi = {0} ; 
        // ���ﴫҪ���ؽ���ID��
        hi.uPID = m_ProcessList.GetItemData(nIndex) ;

        if(FALSE == DeviceIoControl(m_hDriver, 
                                                        IOCTRL_HIDE,
                                                        &hi,
                                                        sizeof(hi),
                                                        NULL,
                                                        0,
                                                        &dwBytesReturned,
                                                        NULL) )
        {
                AfxMessageBox(_T("DeviceIoControl Error")) ;
        }

        GetDlgItem(BTN_HIDE)->EnableWindow(TRUE) ;

        // ����֮��ˢ��һ�¿�Ч���ɣ�
        OnBnClickedRefresh() ;
}

void CProcessManageDlg::OnBnClickedProtectPid()
{
        // TODO: �ڴ���ӿؼ�֪ͨ����������
        GetDlgItem(BTN_PROTECT_PID)->EnableWindow(FALSE) ;

        POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();
        DWORD dwBytesReturned = 0 ;
        if(NULL == pos)
        {
                AfxMessageBox(_T("��ѡ��Ҫ�����Ľ���")) ;
                GetDlgItem(BTN_PROTECT_PID)->EnableWindow(TRUE) ;
                return ;
        }
        int nIndex = m_ProcessList.GetNextSelectedItem(pos) ;

        PROTECT_PID_INFO pi = {0} ; 
        // ���ﴫҪ��������ID��
        pi.uPID = m_ProcessList.GetItemData(nIndex) ;

        if(FALSE == DeviceIoControl(m_hDriver, 
                                                        IOCTRL_ADD_PROTECT_PID,
                                                        &pi,
                                                        sizeof(pi),
                                                        NULL,
                                                        0,
                                                        &dwBytesReturned,
                NULL) )
        {
                AfxMessageBox(_T("DeviceIoControl Error")) ;
        }

        GetDlgItem(BTN_PROTECT_PID)->EnableWindow(TRUE) ;
}

void CProcessManageDlg::OnBnClickedProtectName()
{
        // TODO: �ڴ���ӿؼ�֪ͨ����������
        GetDlgItem(BTN_PROTECT_NAME)->EnableWindow(FALSE) ;
        TCHAR szBuffer[MAX_PATH] = {0} ;
        char ansiBuffer[MAX_PATH] = {0} ;

        POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();
        DWORD dwBytesReturned = 0 ;
        if(NULL == pos)
        {
                AfxMessageBox(_T("��ѡ��Ҫ�����Ľ���")) ;
                GetDlgItem(BTN_PROTECT_PID)->EnableWindow(TRUE) ;
                return ;
        }
        int nIndex = m_ProcessList.GetNextSelectedItem(pos) ;

        PROTECT_NAME_INFO pNi = {0} ; 
        // ���ﴫҪ����������
        m_ProcessList.GetItemText(nIndex, 0, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])) ;

        DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,szBuffer,-1,ansiBuffer,MAX_PATH,NULL,FALSE);
        strncpy((PCHAR)pNi.ProcessName, ansiBuffer, MAX_PROTECT_NAME_LEN - 1) ;

        if(FALSE == DeviceIoControl(m_hDriver, 
                IOCTRL_ADD_PROTECT_NAME,
                &pNi,
                sizeof(pNi),
                NULL,
                0,
                &dwBytesReturned,
                NULL) )
        {
                AfxMessageBox(_T("DeviceIoControl Error")) ;
        }

        GetDlgItem(BTN_PROTECT_NAME)->EnableWindow(TRUE) ;

}

void CProcessManageDlg::OnBnClickedStartFilter()
{
        // TODO: �ڴ���ӿؼ�֪ͨ����������
        GetDlgItem(BTN_START_FILTER)->EnableWindow(FALSE) ;
        DWORD dwBytesReturned = 0 ;

        if(FALSE == DeviceIoControl(m_hDriver, 
                                                        IOCTRL_START_FILTER,
                                                        NULL,
                                                        0,
                                                        NULL,
                                                        0,
                                                        &dwBytesReturned,
                                                        NULL) )
        {
                AfxMessageBox(_T("DeviceIoControl Error")) ;
        }

        GetDlgItem(BTN_START_FILTER)->EnableWindow(TRUE) ;
}

void CProcessManageDlg::OnBnClickedStopFilter()
{
        // TODO: �ڴ���ӿؼ�֪ͨ����������
        GetDlgItem(BTN_STOP_FILTER)->EnableWindow(FALSE) ;
        DWORD dwBytesReturned = 0 ;

        if(FALSE == DeviceIoControl(m_hDriver, 
                                                        IOCTRL_STOP_FILTER,
                                                        NULL,
                                                        0,
                                                        NULL,
                                                        0,
                                                        &dwBytesReturned,
                                                        NULL) )
        {
                AfxMessageBox(_T("DeviceIoControl Error")) ;
        }

        GetDlgItem(BTN_STOP_FILTER)->EnableWindow(TRUE) ;
}

BOOL CProcessManageDlg::ReleaseSys(PTCHAR pszSysPath)
{
        BOOL bResult = FALSE ;
        HRSRC hrSrc = NULL ;
        HGLOBAL hGlobal = NULL ;
        LPVOID pRcData = NULL ;
        HANDLE hFile = INVALID_HANDLE_VALUE ;

        __try
        {
                if (NULL == pszSysPath)
                {
                        OutputErrorInformation(TEXT("CProcessManageDlg::ReleaseSys pszSysPath can't NULL!")) ;
                        __leave ;
                }

                hrSrc = FindResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_SYS), TEXT("SYS"));
                if (NULL == hrSrc)
                {
                        OutputErrorInformation(TEXT("CProcessManageDlg::ReleaseSys FindResource failed!")) ;
                        __leave ;
                }

                hGlobal = LoadResource(GetModuleHandle(0), hrSrc) ;
                if (NULL == hGlobal)
                {
                        OutputErrorInformation(TEXT("CProcessManageDlg::ReleaseSys LoadResource failed!")) ;
                        __leave ;
                }

                pRcData = LockResource(hGlobal) ;
                if (NULL == pRcData)
                {
                        OutputErrorInformation(TEXT("CProcessManageDlg::ReleaseSys LockResource failed!")) ;
                        __leave ;
                }

                DWORD dwSize = SizeofResource(GetModuleHandle(0), hrSrc) ;

                hFile = CreateFile(pszSysPath,
                                            FILE_ALL_ACCESS,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            NULL,
                                            CREATE_ALWAYS,
                                            FILE_ATTRIBUTE_HIDDEN,
                                            NULL) ;
                if (INVALID_HANDLE_VALUE == hFile)
                {
                        OutputErrorInformation(TEXT("CProcessManageDlg::ReleaseSys CreateFile failed!")) ;
                        __leave ;
                }

                DWORD dwNumberOfBytesWritten ;
                if(!WriteFile(hFile,
                               pRcData,
                                dwSize,
                                &dwNumberOfBytesWritten,
                                NULL) 
                        && 0 == dwNumberOfBytesWritten)
                {
                        OutputErrorInformation(TEXT("CProcessManageDlg::ReleaseSys WriteFile failed!")) ;
                        __leave ;
                }

                bResult = TRUE ;

        }

        __finally
        {
                if (INVALID_HANDLE_VALUE !=  hFile)
                {
                        CloseHandle(hFile) ;
                        hFile = INVALID_HANDLE_VALUE ;
                }
        }

        return bResult;
}
