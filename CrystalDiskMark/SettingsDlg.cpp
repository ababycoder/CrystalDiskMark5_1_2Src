// Settings.cpp : 実装ファイル
//

#include "stdafx.h"
#include "DiskMark.h"
#include "SettingsDlg.h"


// CSettingsDlg ダイアログ

IMPLEMENT_DYNCREATE(CSettingsDlg, CDHtmlDialog)

CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialogEx(CSettingsDlg::IDD, CSettingsDlg::IDH, pParent)
{
	m_CurrentLangPath = ((CDHtmlMainDialog*) pParent)->m_CurrentLangPath;
	m_DefaultLangPath = ((CDHtmlMainDialog*) pParent)->m_DefaultLangPath;
	m_ZoomType = ((CDHtmlMainDialog*) pParent)->GetZoomType();

	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskMarkApp*) AfxGetApp())->m_Ini);
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialogEx::DoDataExchange(pDX);

	DDX_DHtml_SelectValue(pDX, _T("SequentialQueues"), m_ValueSequentialQueues);
	DDX_DHtml_SelectIndex(pDX, _T("SequentialQueues"), m_IndexSequentialQueues);
	DDX_DHtml_SelectValue(pDX, _T("SequentialThreads"), m_ValueSequentialThreads);
	DDX_DHtml_SelectIndex(pDX, _T("SequentialThreads"), m_IndexSequentialThreads);
	DDX_DHtml_SelectValue(pDX, _T("RandomQueues"), m_ValueRandomQueues);
	DDX_DHtml_SelectIndex(pDX, _T("RandomQueues"), m_IndexRandomQueues);
	DDX_DHtml_SelectValue(pDX, _T("RandomThreads"), m_ValueRandomThreads);
	DDX_DHtml_SelectIndex(pDX, _T("RandomThreads"), m_IndexRandomThreads);
}

BOOL CSettingsDlg::OnInitDialog()
{
	CDHtmlDialogEx::OnInitDialog();

	m_SequentialQueues = GetPrivateProfileInt(_T("Settings"), _T("SequentialMultiQueues"), 32, m_Ini);
	if (m_SequentialQueues <= 0 || m_SequentialQueues > MAX_QUEUES)
	{
		m_SequentialQueues = 32;	// default value is 32.
	}
	m_SequentialThreads = GetPrivateProfileInt(_T("Settings"), _T("SequentialMultiThreads"), 1, m_Ini);
	if (m_SequentialThreads <= 0 || m_SequentialThreads > MAX_THREADS)
	{
		m_SequentialThreads = 1;	// default value is 1.
	}
	m_RandomQueues = GetPrivateProfileInt(_T("Settings"), _T("RandomMultiQueues"), 32, m_Ini);
	if (m_RandomQueues <= 0 || m_RandomQueues > MAX_QUEUES)
	{
		m_RandomQueues = 32;	// default value is 32.
	}
	m_RandomThreads = GetPrivateProfileInt(_T("Settings"), _T("RandomMultiThreads"), 1, m_Ini);
	if (m_RandomThreads <= 0 || m_RandomThreads > MAX_THREADS)
	{
		m_RandomThreads = 1;	// default value is 1.
	}
	UpdateData(FALSE);

	SetWindowText(i18n(_T("WindowTitle"), _T("QUEUES_THREADS")));

	EnableDpiAware();
	InitDHtmlDialog(SIZE_X, SIZE_Y, ((CDiskMarkApp*) AfxGetApp())->m_SettingsDlgPath);

	return TRUE;
}

void CSettingsDlg::InitSelectBoxQ(CString ElementName, int currentValue, int maxValue, long *index)
{
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	CString option = L"";

	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if (FAILED(hr)) return;

	option.Format(L"<select name=\"%s\" id=\"%s\">\n", ElementName, ElementName);

	*index = -1;
	int count = 0;
	for (int i = 1; i <= maxValue; i<<=1)
	{
		cstr.Format(L"<option value=\"%d\">%d</option>\n", i, i);
		option += cstr;

		if (i == currentValue)
		{
			*index = count;
		}
		count++;
	}

	if (*index == -1)
	{
		cstr.Format(L"<option value=\"%d\">(%d)</option>\n", currentValue, currentValue);
		option += cstr;
		*index = count;
	}

	option += L"</select>\n";

	bstr = option;
	pHtmlElement->put_outerHTML(bstr);
	bstr.Empty();

	UpdateData(FALSE);
}

void CSettingsDlg::InitSelectBoxT(CString ElementName, int currentValue, int maxValue, long *index)
{
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	CString option = L"";

	hr = GetElementInterface(ElementName, IID_IHTMLElement, (void **) &pHtmlElement);
	if (FAILED(hr)) return;

	option.Format(L"<select name=\"%s\" id=\"%s\">\n", ElementName, ElementName);

	int count = 0;
	for (int i = 1; i <= maxValue; i += 1)
	{
		cstr.Format(L"<option value=\"%d\">%d</option>\n", i, i);
		option += cstr;

		if (i == currentValue)
		{
			*index = count;
		}
		count++;
	}
	option += L"</select>\n";

	bstr = option;
	pHtmlElement->put_outerHTML(bstr);
	bstr.Empty();

	UpdateData(FALSE);
}

void CSettingsDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if (cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		m_FlagShowWindow = TRUE;

		ChangeZoomType(m_ZoomType);
		SetClientRect((DWORD) (SIZE_X * m_ZoomRatio), (DWORD) (SIZE_Y * m_ZoomRatio), 0);

		InitSelectBoxQ(L"SequentialQueues", m_SequentialQueues, MAX_QUEUES, &m_IndexSequentialQueues);
		InitSelectBoxT(L"SequentialThreads", m_SequentialThreads, MAX_THREADS, &m_IndexSequentialThreads);
		InitSelectBoxQ(L"RandomQueues", m_RandomQueues, MAX_QUEUES, &m_IndexRandomQueues);
		InitSelectBoxT(L"RandomThreads", m_RandomThreads, MAX_THREADS, &m_IndexRandomThreads);

		UpdateData(FALSE);
		CenterWindow();
		ShowWindow(SW_SHOW);
	}
}
void CSettingsDlg::OnCancel()
{
	UpdateData(TRUE);

	WritePrivateProfileString(_T("Settings"), _T("SequentialMultiQueues"), m_ValueSequentialQueues, m_Ini);
	WritePrivateProfileString(_T("Settings"), _T("SequentialMultiThreads"), m_ValueSequentialThreads, m_Ini);
	WritePrivateProfileString(_T("Settings"), _T("RandomMultiQueues"), m_ValueRandomQueues, m_Ini);
	WritePrivateProfileString(_T("Settings"), _T("RandomMultiThreads"), m_ValueRandomThreads, m_Ini);

	CDHtmlDialogEx::OnCancel();
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDHtmlDialogEx)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CSettingsDlg)
END_DHTML_EVENT_MAP()

