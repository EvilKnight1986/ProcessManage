// ProcessManageDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"

// CProcessManageDlg �Ի���
class CProcessManageDlg : public CDialog
{
// ����
public:
        CProcessManageDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
        enum { IDD = IDD_PROCESSMANAGE_DIALOG };

        protected:
        virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
        HICON m_hIcon;

        // ���ɵ���Ϣӳ�亯��
        virtual BOOL OnInitDialog();
        afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedTest();
private:
    CStatusBarCtrl m_StatusBar; // ״̬��
    CListCtrl m_ProcessList;      // ������ʾ�б�
    HANDLE m_hDriver;
    ULONG m_uNtTerrminateProcessIndex ;
    ULONG m_uNtCreateProcessExIndex ;
    ULONG m_uNtOpenProcessIndex ;
public:
    void AutoSize(CListCtrl * pList); // �Զ�����ctrllist title���
    afx_msg void OnBnClickedRefresh();
    afx_msg void OnLvnItemchangedProcesslist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedHide();
//    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnBnClickedProtectPid();
    afx_msg void OnBnClickedProtectName();
    afx_msg void OnBnClickedStartFilter();
    afx_msg void OnBnClickedStopFilter();
    BOOL Load(void);
    BOOL Unload(void);
    BOOL ReleaseSys(PTCHAR pszSysPath);
};
