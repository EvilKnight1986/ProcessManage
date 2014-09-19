// ProcessManageDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"

// CProcessManageDlg 对话框
class CProcessManageDlg : public CDialog
{
// 构造
public:
        CProcessManageDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
        enum { IDD = IDD_PROCESSMANAGE_DIALOG };

        protected:
        virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
        HICON m_hIcon;

        // 生成的消息映射函数
        virtual BOOL OnInitDialog();
        afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedTest();
private:
    CStatusBarCtrl m_StatusBar; // 状态栏
    CListCtrl m_ProcessList;      // 进程显示列表
    HANDLE m_hDriver;
    ULONG m_uNtTerrminateProcessIndex ;
    ULONG m_uNtCreateProcessExIndex ;
    ULONG m_uNtOpenProcessIndex ;
public:
    void AutoSize(CListCtrl * pList); // 自动调节ctrllist title宽度
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
