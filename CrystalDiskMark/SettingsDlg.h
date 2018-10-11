/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The MIT License
//
//                                             Copyright (c) 2007-2015 hiyohiyo
/*---------------------------------------------------------------------------*/

#pragma once

class CSettingsDlg : public CDHtmlDialogEx
{
	DECLARE_DYNCREATE(CSettingsDlg)

	static const int SIZE_X = 240;
	static const int SIZE_Y = 100;

public:
	CSettingsDlg(CWnd* pParent = NULL);
	virtual ~CSettingsDlg();

// ダイアログ データ
	enum { IDD = IDD_SETTINGS, IDH = IDR_HTML_DUMMY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	virtual void OnCancel();

	void InitSelectBoxQ(CString ElementName, int currentValue, int maxValue, long *index);
	void InitSelectBoxT(CString ElementName, int currentValue, int maxValue, long *index);

	CString m_ValueSequentialQueues;
	CString m_ValueSequentialThreads;
	CString m_ValueRandomQueues;
	CString m_ValueRandomThreads;
	long m_SequentialQueues;
	long m_SequentialThreads;
	long m_RandomQueues;
	long m_RandomThreads;
	long m_IndexSequentialQueues;
	long m_IndexSequentialThreads;
	long m_IndexRandomQueues;
	long m_IndexRandomThreads;

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
