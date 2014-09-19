#pragma once
#include "afxwin.h"


// ProgramName 对话框

class ProgramName : public CDialog
{
	DECLARE_DYNAMIC(ProgramName)

public:
	ProgramName(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ProgramName();

// 对话框数据
	enum { IDD = IDD_PROTECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
    CString m_strProgram;
public:
    afx_msg void OnBnClickedOk();

    bool GetProgramName(CString & strProgramName);
};
