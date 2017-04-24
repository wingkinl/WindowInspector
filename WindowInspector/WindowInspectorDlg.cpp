
// WindowInspectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WindowInspector.h"
#include "WindowInspectorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWindowInspectorDlg dialog



CWindowInspectorDlg::CWindowInspectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_WINDOWINSPECTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nUpdateWndInfoTimer = 0;
}

void CWindowInspectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWindowInspectorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CWindowInspectorDlg message handlers

enum
{
	TIMER_UPDATE_WIN_INFO_ID		= 100,
	TIMER_UPDATE_WIN_INFO_ELAPSE	= 250,
};

BOOL CWindowInspectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_nUpdateWndInfoTimer = SetTimer(TIMER_UPDATE_WIN_INFO_ID, TIMER_UPDATE_WIN_INFO_ELAPSE, nullptr);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWindowInspectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		DrawWindowInfo(dc);

		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWindowInspectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWindowInspectorDlg::DrawWindowInfo(CDC& dc)
{
	CPoint pos(0,0);
	GetCursorPos(&pos);
	HWND hWndInspect = ::WindowFromPoint(pos);
	CRect rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect, afxGlobalData.clrBtnFace);
	if (!hWndInspect)
		return;
	m_hWndInspect = hWndInspect;
	CWnd* pWndInspect = CWnd::FromHandle(m_hWndInspect);
	auto pOldFont = dc.SelectObject(&GetGlobalData()->fontRegular);
	
	CString strLine, strText;
	strLine.Format(_T("Pos: %d, %d"), pos.x, pos.y);
	strText = strLine;

	strLine.Format(_T("HWND: 0x%p"), m_hWndInspect);
	strText += _T("\r\n");
	strText += strLine;

	pWndInspect->GetWindowText(strLine);
	strText += _T("\r\nTitle: ");
	strText += strLine;

	const int nMaxClassLen = 256;
	TCHAR szClassName[nMaxClassLen +1] = {0};
	GetClassName(m_hWndInspect, szClassName, nMaxClassLen);
	strText += _T("\r\nClass: ");
	strText += szClassName;

	HMODULE hModule = (HMODULE)GetClassLongPtr(m_hWndInspect, GCLP_HMODULE);
	strLine.Format(_T("HMODULE: 0x%p"), hModule);
	strText += _T("\r\n");
	strText += strLine;


	dc.DrawText(strText, rect, DT_NOPREFIX);

	dc.SelectObject(pOldFont);
}


void CWindowInspectorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_UPDATE_WIN_INFO_ID)
	{
		Invalidate();
		UpdateWindow();
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CWindowInspectorDlg::OnNcDestroy()
{
	if (m_nUpdateWndInfoTimer)
	{
		KillTimer(m_nUpdateWndInfoTimer);
		m_nUpdateWndInfoTimer = 0;
	}
	CDialogEx::OnNcDestroy();
}
