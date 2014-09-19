// ProgramName.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessManage.h"
#include "ProgramName.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// ProgramName 对话框

IMPLEMENT_DYNAMIC(ProgramName, CDialog)

ProgramName::ProgramName(CWnd* pParent /*=NULL*/)
	: CDialog(ProgramName::IDD, pParent)
    , m_strProgram(_T(""))
{

}

ProgramName::~ProgramName()
{
}

void ProgramName::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, EDT_PROGRAM, m_strProgram);
    DDV_MaxChars(pDX, m_strProgram, 255);
}


BEGIN_MESSAGE_MAP(ProgramName, CDialog)
    ON_BN_CLICKED(IDOK, &ProgramName::OnBnClickedOk)
END_MESSAGE_MAP()


// ProgramName 消息处理程序

void ProgramName::OnBnClickedOk()
{
    // TODO: 在此添加控件通知处理程序代码

    // 将控制的值更新到变量
    UpdateData(TRUE) ;
    
    // 这里就懒得检查一些7788的东西，直接判断长度
    int nLen = m_strProgram.GetLength() ;
    if (nLen <= 4 || nLen > MAX_PATH)
    {
        AfxMessageBox(_T("大哥，您是找岔是吧！咱都不是找事的人，检查一下重新输入吧！")) ;
        m_strProgram = _T("") ;
        return ;
    }

    CString str = PathFindExtension(m_strProgram) ;
    // com之类的就不检查了
    // 只检查exe
    if (str.CompareNoCase(_T(".exe")))
    {
        AfxMessageBox(_T("大哥，您是找岔是吧！咱都不是找事的人，检查一下后辍重新输入吧！")) ;
        m_strProgram = _T("") ;
        return ;
    }

    OnOK();
}

// 取得进程名
bool ProgramName::GetProgramName(CString & strProgramName)
{
    // 少于5的话就没有必要去取了，最短的应该是a.exe类似的
    if(m_strProgram.GetLength() > 4 && m_strProgram.GetLength() < MAX_PATH)
    {
        strProgramName = m_strProgram ;
        return true ;
    }
    return false;
}
