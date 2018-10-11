// InputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DiskMark.h"
#include "InputDlg.h"
#include "afxdialogex.h"


// CInputDlg dialog

IMPLEMENT_DYNAMIC(CInputDlg, CDialogEx)

CInputDlg::CInputDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INPUT_DIALOG, pParent)
{
	
}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CInputDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CInputDlg message handlers


void CInputDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_inputNum = GetDlgItemInt(IDC_EDIT);
	CDialogEx::OnOK();
}

BOOL CInputDlg::OnInitDialog()
{
	SetDlgItemInt(IDC_EDIT, m_inputNum);

	return true;
}