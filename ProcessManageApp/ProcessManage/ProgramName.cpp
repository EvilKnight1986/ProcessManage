// ProgramName.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessManage.h"
#include "ProgramName.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// ProgramName �Ի���

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


// ProgramName ��Ϣ�������

void ProgramName::OnBnClickedOk()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������

    // �����Ƶ�ֵ���µ�����
    UpdateData(TRUE) ;
    
    // ��������ü��һЩ7788�Ķ�����ֱ���жϳ���
    int nLen = m_strProgram.GetLength() ;
    if (nLen <= 4 || nLen > MAX_PATH)
    {
        AfxMessageBox(_T("��磬�����Ҳ��ǰɣ��۶��������µ��ˣ����һ����������ɣ�")) ;
        m_strProgram = _T("") ;
        return ;
    }

    CString str = PathFindExtension(m_strProgram) ;
    // com֮��ľͲ������
    // ֻ���exe
    if (str.CompareNoCase(_T(".exe")))
    {
        AfxMessageBox(_T("��磬�����Ҳ��ǰɣ��۶��������µ��ˣ����һ�º����������ɣ�")) ;
        m_strProgram = _T("") ;
        return ;
    }

    OnOK();
}

// ȡ�ý�����
bool ProgramName::GetProgramName(CString & strProgramName)
{
    // ����5�Ļ���û�б�Ҫȥȡ�ˣ���̵�Ӧ����a.exe���Ƶ�
    if(m_strProgram.GetLength() > 4 && m_strProgram.GetLength() < MAX_PATH)
    {
        strProgramName = m_strProgram ;
        return true ;
    }
    return false;
}
