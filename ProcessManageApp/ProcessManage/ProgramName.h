#pragma once
#include "afxwin.h"


// ProgramName �Ի���

class ProgramName : public CDialog
{
	DECLARE_DYNAMIC(ProgramName)

public:
	ProgramName(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~ProgramName();

// �Ի�������
	enum { IDD = IDD_PROTECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
private:
    CString m_strProgram;
public:
    afx_msg void OnBnClickedOk();

    bool GetProgramName(CString & strProgramName);
};
