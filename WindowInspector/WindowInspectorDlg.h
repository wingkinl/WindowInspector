
// WindowInspectorDlg.h : header file
//

#pragma once
#include "afxwin.h"

class CInspectWndClassInfo
{
public:
	void Init(HWND hWnd)
	{
		atom = (ATOM)GetClassLongPtr(hWnd, GCW_ATOM);
		style = (UINT)GetClassLongPtr(hWnd, GCL_STYLE);
		lpfnWndProc = (WNDPROC)GetClassLongPtr(hWnd, GCLP_WNDPROC);
		cbClsExtra = (int)GetClassLongPtr(hWnd, GCL_CBCLSEXTRA);
		cbWndExtra = (int)GetClassLongPtr(hWnd, GCL_CBWNDEXTRA);
		hModule = (HINSTANCE)GetClassLongPtr(hWnd, GCLP_HMODULE);
		hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
		hCursor = (HCURSOR)GetClassLongPtr(hWnd, GCLP_HCURSOR);
		hbrBackground = (HBRUSH)GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
		lpszMenuName = (LPCTSTR)GetClassLongPtr(hWnd, GCLP_MENUNAME);
		GetClassName(hWnd, szClassName, MAX_CLASS_NAME);
		hIconSm = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM);
	}
	inline BOOL IsClassType(LPCTSTR pszClass) const
	{
		return _tcsnicmp(szClassName, pszClass, MAX_CLASS_NAME) == 0;
	}
	inline BOOL IsButton() const { return IsClassType(WC_BUTTON); }
	inline BOOL IsEdit() const { return IsClassType(WC_EDIT); }
	inline BOOL IsListBox() const { return IsClassType(WC_LISTBOX); }
	inline BOOL IsListView() const { return IsClassType(WC_LISTVIEW); }
	inline BOOL IsComboBox() const { return IsClassType(WC_COMBOBOX); }
	inline BOOL IsHeaderCtrl() const { return IsClassType(WC_HEADER); }
	inline BOOL IsScrollBar() const { return IsClassType(WC_SCROLLBAR); }
	inline BOOL IsTreeView() const { return IsClassType(WC_TREEVIEW); }
	inline BOOL IsTabCtrl() const { return IsClassType(WC_TABCONTROL); }
	inline BOOL IsStatic() const { return IsClassType(WC_STATIC); }

	inline BOOL IsMenu() const { return IsClassType(_T("#32768")); }
	inline BOOL IsDesktop() const { return IsClassType(_T("#32769")); }
	inline BOOL IsDialog() const { return IsClassType(_T("#32770")); }
	inline BOOL IsTaskSwitchWnd() const { return IsClassType(_T("#32771")); }
	inline BOOL IsIconTitles() const { return IsClassType(_T("#32772")); }

	ATOM        atom;
	UINT        style;
	WNDPROC     lpfnWndProc;
	int         cbClsExtra;
	int         cbWndExtra;
	HMODULE     hModule;
	HICON       hIcon;
	HCURSOR     hCursor;
	HBRUSH      hbrBackground;
	LPCTSTR     lpszMenuName;
	enum { MAX_CLASS_NAME = 256 };
	TCHAR       szClassName[MAX_CLASS_NAME + 1];
	HICON       hIconSm;
};

class CInspectWndInfo
{
public:
	void Init(HWND hWnd)
	{
		m_hWnd = hWnd;
		m_clsInfo.Init(hWnd);

		CWnd* pWndInspect = CWnd::FromHandle(hWnd);
		pWndInspect->GetWindowText(m_strTitle);
		const int nMaxDisplayTitleLen = 100;
		int nPos = m_strTitle.FindOneOf(_T("\r\n"));
		if (nPos > 0 || m_strTitle.GetLength() > nMaxDisplayTitleLen)
		{
			m_strTitle.Truncate(nPos > 0 ? min(nPos, nMaxDisplayTitleLen) : nMaxDisplayTitleLen);
			m_strTitle += _T("...");
		}
		m_dwStyle = pWndInspect->GetStyle();
		m_dwStyleEx = pWndInspect->GetExStyle();
		m_bIsUnicode = IsWindowUnicode(hWnd);
		pWndInspect->GetWindowRect(m_rectWnd);
		pWndInspect->GetClientRect(m_rectClient);
		GetWindowThreadProcessId(hWnd, &m_dwPID);
	}
public:
	HWND					m_hWnd;
	CInspectWndClassInfo	m_clsInfo;

	CString					m_strTitle;
	DWORD					m_dwStyle;
	DWORD					m_dwStyleEx;
	BOOL					m_bIsUnicode;
	CRect					m_rectWnd;
	CRect					m_rectClient;

	DWORD					m_dwPID;
};

// CWindowInspectorDlg dialog
class CWindowInspectorDlg : public CDialogEx
{
// Construction
public:
	CWindowInspectorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINDOWINSPECTOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
protected:
	void UpdateWindowInfo();
// Implementation
protected:
	HICON m_hIcon;
	UINT_PTR	m_nUpdateWndInfoTimer;
	HWND		m_hWndInspect;
	CString		m_strOriginalTitle;
	CEdit		m_editWndInfo;
	CEdit		m_editParentWndInfo;
	CFont		m_fontEdit;
	CSize		m_szDlg;

	int			m_nEditGapToEdge;
	int			m_nEditGapToEdit;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNcDestroy();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
