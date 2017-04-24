
// WindowInspectorDlg.h : header file
//

#pragma once


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
	void DrawWindowInfo(CDC& dc);
// Implementation
protected:
	HICON m_hIcon;
	UINT_PTR	m_nUpdateWndInfoTimer;
	HWND		m_hWndInspect;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNcDestroy();
};
