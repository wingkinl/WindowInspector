
// WindowInspectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WindowInspector.h"
#include "WindowInspectorDlg.h"
#include "afxdialogex.h"
#include <inttypes.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// taken from http://www.catch22.net/software/winspy
#define INVERT_BORDER 3

//
//	Invert the specified window's border
//
void InvertWindow(HWND hwnd, BOOL fShowHidden)
{
	RECT rect;
	RECT rect2;
	RECT rectc;
	HDC hdc;
	int x1, y1;

	int border = INVERT_BORDER;

	if (hwnd == 0)
		return;

	//window rectangle (screen coords)
	GetWindowRect(hwnd, &rect);

	//client rectangle (screen coords)
	GetClientRect(hwnd, &rectc);
	ClientToScreen(hwnd, (POINT *)&rectc.left);
	ClientToScreen(hwnd, (POINT *)&rectc.right);
	//MapWindowPoints(hwnd, 0, (POINT *)&rectc, 2);

	x1 = rect.left;
	y1 = rect.top;
	OffsetRect(&rect, -x1, -y1);
	OffsetRect(&rectc, -x1, -y1);

	if (rect.bottom - border * 2 < 0)
		border = 1;

	if (rect.right - border * 2 < 0)
		border = 1;

	if (fShowHidden == TRUE)
		hwnd = 0;

	hdc = GetWindowDC(hwnd);

	if (hdc == 0)
		return;

	//top edge
	//border = rectc.top-rect.top;
	SetRect(&rect2, 0, 0, rect.right, border);
	if (fShowHidden == TRUE) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	//left edge
	//border = rectc.left-rect.left;
	SetRect(&rect2, 0, border, border, rect.bottom);
	if (fShowHidden == TRUE) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	//right edge
	//border = rect.right-rectc.right;
	SetRect(&rect2, border, rect.bottom - border, rect.right, rect.bottom);
	if (fShowHidden == TRUE) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);

	//bottom edge
	//border = rect.bottom-rectc.bottom;
	SetRect(&rect2, rect.right - border, border, rect.right, rect.bottom - border);
	if (fShowHidden == TRUE) OffsetRect(&rect2, x1, y1);
	InvertRect(hdc, &rect2);


	ReleaseDC(hwnd, hdc);
}

// CWindowInspectorDlg dialog



CWindowInspectorDlg::CWindowInspectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_WINDOWINSPECTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hWndInspect = nullptr;
	m_nUpdateWndInfoTimer = 0;
}

void CWindowInspectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_WINDOW_INFO, m_editWndInfo);
	DDX_Control(pDX, IDC_EDIT_PARENT_WINDOW_INFO, m_editParentWndInfo);
}

BEGIN_MESSAGE_MAP(CWindowInspectorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_NCDESTROY()
	ON_WM_HOTKEY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CWindowInspectorDlg message handlers

enum
{
	TIMER_UPDATE_WIN_INFO_ID		= 100,
	TIMER_UPDATE_WIN_INFO_ELAPSE	= 250,

	IDH_FREEZE = 1000,
};


BOOL CWindowInspectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	GetWindowText(m_strOriginalTitle);

	CRect rect;
	GetClientRect(rect);
	m_szDlg = rect.Size();

	CRect rcEdit;
	m_editWndInfo.GetWindowRect(rcEdit);
	ScreenToClient(rcEdit);
	CRect rcEditParent;
	m_editParentWndInfo.GetWindowRect(rcEditParent);
	ScreenToClient(rcEditParent);
	m_nEditGapToEdge = rcEdit.left - rect.left;
	m_nEditGapToEdit = rcEditParent.left - rcEdit.right;

	//m_fontEdit.CreateStockObject(ANSI_FIXED_FONT);
	m_fontEdit.CreatePointFont(110, _T("Courier New"));
	m_editWndInfo.SetFont(&m_fontEdit);
	m_editParentWndInfo.SetFont(&m_fontEdit);

	RegisterHotKey(GetSafeHwnd(), IDH_FREEZE, MOD_WIN|MOD_SHIFT, 'A');
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWindowInspectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static bool _GetBrushText(ULONG_PTR hBrush, CString& strText)
{
#define HANDLE_BRUSH_CASE(_x)	\
	case _x:\
		strText.Format(_T("%s (%d)"), _T(#_x), _x);\
		return true\

	if (hBrush >= 1)
	{
		switch (hBrush - 1)
		{
		HANDLE_BRUSH_CASE(COLOR_SCROLLBAR);
		HANDLE_BRUSH_CASE(COLOR_BACKGROUND);
		HANDLE_BRUSH_CASE(COLOR_ACTIVECAPTION);
		HANDLE_BRUSH_CASE(COLOR_INACTIVECAPTION);
		HANDLE_BRUSH_CASE(COLOR_MENU);
		HANDLE_BRUSH_CASE(COLOR_WINDOW);
		HANDLE_BRUSH_CASE(COLOR_WINDOWFRAME);
		HANDLE_BRUSH_CASE(COLOR_MENUTEXT);
		HANDLE_BRUSH_CASE(COLOR_WINDOWTEXT);
		HANDLE_BRUSH_CASE(COLOR_CAPTIONTEXT);
		HANDLE_BRUSH_CASE(COLOR_ACTIVEBORDER);
		HANDLE_BRUSH_CASE(COLOR_INACTIVEBORDER);
		HANDLE_BRUSH_CASE(COLOR_APPWORKSPACE);
		HANDLE_BRUSH_CASE(COLOR_HIGHLIGHT);
		HANDLE_BRUSH_CASE(COLOR_HIGHLIGHTTEXT);
		HANDLE_BRUSH_CASE(COLOR_BTNFACE);
		HANDLE_BRUSH_CASE(COLOR_BTNSHADOW);
		HANDLE_BRUSH_CASE(COLOR_GRAYTEXT);
		HANDLE_BRUSH_CASE(COLOR_BTNTEXT);
		HANDLE_BRUSH_CASE(COLOR_INACTIVECAPTIONTEXT);
		HANDLE_BRUSH_CASE(COLOR_BTNHIGHLIGHT);
		HANDLE_BRUSH_CASE(COLOR_3DDKSHADOW);
		HANDLE_BRUSH_CASE(COLOR_3DLIGHT);
		HANDLE_BRUSH_CASE(COLOR_INFOTEXT);
		HANDLE_BRUSH_CASE(COLOR_INFOBK);
		HANDLE_BRUSH_CASE(COLOR_HOTLIGHT);
		HANDLE_BRUSH_CASE(COLOR_GRADIENTACTIVECAPTION);
		HANDLE_BRUSH_CASE(COLOR_GRADIENTINACTIVECAPTION);
		HANDLE_BRUSH_CASE(COLOR_MENUHILIGHT);
		HANDLE_BRUSH_CASE(COLOR_MENUBAR);
		default:
			return false;
		}
	}
	return false;
}

static void _GetWindowStyleText(CString& strText, const CInspectWndInfo& info)
{
	CString strLine;
#define HANDLE_WND_STYLE(_x, _mask)	\
		if ((info.m_dwStyle & _mask) == _x) \
		{\
			strLine.Format(_T("%-30s    (0x%08" PRIXPTR ")"), _T(#_x), _x);\
			strText += strLine;\
			strText += _T("\r\n");\
		}

#define HANDLE_WND_STYLE_EX(_x, _mask)	\
		if ((info.m_dwStyleEx & _mask) == _x) \
		{\
			strLine.Format(_T("%-30s    (0x%08" PRIXPTR ")"), _T(#_x), _x);\
			strText += strLine;\
			strText += _T("\r\n");\
		}

#define HANDLE_WND_STYLE_EXT(_ext, _x, _mask)	\
		if ((_ext & _mask) == _x) \
		{\
			strLine.Format(_T("%-30s    (0x%08" PRIXPTR ")"), _T(#_x), _x);\
			strText += strLine;\
			strText += _T("\r\n");\
		}

	HANDLE_WND_STYLE(WS_POPUP, WS_POPUP);
	HANDLE_WND_STYLE(WS_CHILD, WS_CHILD);
	HANDLE_WND_STYLE(WS_MINIMIZE, WS_MINIMIZE);
	HANDLE_WND_STYLE(WS_VISIBLE, WS_VISIBLE);
	HANDLE_WND_STYLE(WS_DISABLED, WS_DISABLED);
	HANDLE_WND_STYLE(WS_CLIPSIBLINGS, WS_CLIPSIBLINGS);
	HANDLE_WND_STYLE(WS_CLIPCHILDREN, WS_CLIPCHILDREN);
	HANDLE_WND_STYLE(WS_MAXIMIZE, WS_MAXIMIZE);
	HANDLE_WND_STYLE(WS_CAPTION, WS_CAPTION);
	HANDLE_WND_STYLE(WS_BORDER, WS_BORDER);
	HANDLE_WND_STYLE(WS_DLGFRAME, WS_DLGFRAME);
	HANDLE_WND_STYLE(WS_VSCROLL, WS_VSCROLL);
	HANDLE_WND_STYLE(WS_HSCROLL, WS_HSCROLL);
	HANDLE_WND_STYLE(WS_SYSMENU, WS_SYSMENU);
	HANDLE_WND_STYLE(WS_THICKFRAME, WS_THICKFRAME);
	if (info.m_dwStyle & WS_CHILD)
	{
		HANDLE_WND_STYLE(WS_GROUP, WS_GROUP);
		HANDLE_WND_STYLE(WS_TABSTOP, WS_TABSTOP);
	}
	else
	{
		HANDLE_WND_STYLE(WS_MINIMIZEBOX, WS_MINIMIZEBOX);
		HANDLE_WND_STYLE(WS_MAXIMIZEBOX, WS_MAXIMIZEBOX);
	}

	strText += _T("\r\n");

	HANDLE_WND_STYLE_EX(WS_EX_DLGMODALFRAME, WS_EX_DLGMODALFRAME);
	HANDLE_WND_STYLE_EX(WS_EX_NOPARENTNOTIFY, WS_EX_NOPARENTNOTIFY);
	HANDLE_WND_STYLE_EX(WS_EX_TOPMOST, WS_EX_TOPMOST);
	HANDLE_WND_STYLE_EX(WS_EX_ACCEPTFILES, WS_EX_ACCEPTFILES);
	HANDLE_WND_STYLE_EX(WS_EX_TRANSPARENT, WS_EX_TRANSPARENT);
	HANDLE_WND_STYLE_EX(WS_EX_MDICHILD, WS_EX_MDICHILD);
	HANDLE_WND_STYLE_EX(WS_EX_TOOLWINDOW, WS_EX_TOOLWINDOW);
	HANDLE_WND_STYLE_EX(WS_EX_WINDOWEDGE, WS_EX_WINDOWEDGE);
	HANDLE_WND_STYLE_EX(WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);
	HANDLE_WND_STYLE_EX(WS_EX_CONTEXTHELP, WS_EX_CONTEXTHELP);
	HANDLE_WND_STYLE_EX(WS_EX_RIGHT, WS_EX_RIGHT);
	HANDLE_WND_STYLE_EX(WS_EX_RTLREADING, WS_EX_RTLREADING);
	HANDLE_WND_STYLE_EX(WS_EX_LEFTSCROLLBAR, WS_EX_LEFTSCROLLBAR);
	HANDLE_WND_STYLE_EX(WS_EX_CONTROLPARENT, WS_EX_CONTROLPARENT);
	HANDLE_WND_STYLE_EX(WS_EX_STATICEDGE, WS_EX_STATICEDGE);
	HANDLE_WND_STYLE_EX(WS_EX_APPWINDOW, WS_EX_APPWINDOW);
	HANDLE_WND_STYLE_EX(WS_EX_LAYERED, WS_EX_LAYERED);
	HANDLE_WND_STYLE_EX(WS_EX_NOINHERITLAYOUT, WS_EX_NOINHERITLAYOUT);
	HANDLE_WND_STYLE_EX(WS_EX_NOREDIRECTIONBITMAP, WS_EX_NOREDIRECTIONBITMAP);
	HANDLE_WND_STYLE_EX(WS_EX_LAYOUTRTL, WS_EX_LAYOUTRTL);
	HANDLE_WND_STYLE_EX(WS_EX_COMPOSITED, WS_EX_COMPOSITED);
	HANDLE_WND_STYLE_EX(WS_EX_NOACTIVATE, WS_EX_NOACTIVATE);

	strText += _T("\r\n");

	if (info.m_clsInfo.IsButton())
	{
		HANDLE_WND_STYLE(BS_DEFPUSHBUTTON, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_CHECKBOX, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_AUTOCHECKBOX, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_RADIOBUTTON, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_3STATE, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_AUTO3STATE, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_GROUPBOX, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_USERBUTTON, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_AUTORADIOBUTTON, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_PUSHBOX, BS_TYPEMASK);
		HANDLE_WND_STYLE(BS_OWNERDRAW, BS_TYPEMASK);

		HANDLE_WND_STYLE(BS_LEFTTEXT, BS_LEFTTEXT);
		HANDLE_WND_STYLE(BS_ICON, BS_ICON);
		HANDLE_WND_STYLE(BS_BITMAP, BS_BITMAP);

		enum {BS_POS_MASK = BS_LEFT|BS_RIGHT|BS_CENTER|BS_TOP};
		HANDLE_WND_STYLE(BS_LEFT, BS_POS_MASK)
		HANDLE_WND_STYLE(BS_RIGHT, BS_POS_MASK)
		HANDLE_WND_STYLE(BS_CENTER, BS_POS_MASK)
		HANDLE_WND_STYLE(BS_TOP, BS_POS_MASK)

		HANDLE_WND_STYLE(BS_BOTTOM, BS_BOTTOM);
		HANDLE_WND_STYLE(BS_VCENTER, BS_VCENTER);

		HANDLE_WND_STYLE(BS_PUSHLIKE, BS_PUSHLIKE);
		HANDLE_WND_STYLE(BS_MULTILINE, BS_MULTILINE);
		HANDLE_WND_STYLE(BS_NOTIFY, BS_NOTIFY);
		HANDLE_WND_STYLE(BS_FLAT, BS_FLAT);
	}
	else if (info.m_clsInfo.IsEdit())
	{
		HANDLE_WND_STYLE(ES_CENTER, ES_CENTER);
		HANDLE_WND_STYLE(ES_RIGHT, ES_RIGHT);
		HANDLE_WND_STYLE(ES_MULTILINE, ES_MULTILINE);
		HANDLE_WND_STYLE(ES_UPPERCASE, ES_UPPERCASE);
		HANDLE_WND_STYLE(ES_LOWERCASE, ES_LOWERCASE);
		HANDLE_WND_STYLE(ES_PASSWORD, ES_PASSWORD);
		HANDLE_WND_STYLE(ES_AUTOVSCROLL, ES_AUTOVSCROLL);
		HANDLE_WND_STYLE(ES_AUTOHSCROLL, ES_AUTOHSCROLL);
		HANDLE_WND_STYLE(ES_NOHIDESEL, ES_NOHIDESEL);
		HANDLE_WND_STYLE(ES_OEMCONVERT, ES_OEMCONVERT);
		HANDLE_WND_STYLE(ES_READONLY, ES_READONLY);
		HANDLE_WND_STYLE(ES_WANTRETURN, ES_WANTRETURN);
		HANDLE_WND_STYLE(ES_NUMBER, ES_NUMBER);
	}
	else if (info.m_clsInfo.IsListBox())
	{
		HANDLE_WND_STYLE(LBS_NOTIFY, LBS_NOTIFY);
		HANDLE_WND_STYLE(LBS_SORT, LBS_SORT);
		HANDLE_WND_STYLE(LBS_NOREDRAW, LBS_NOREDRAW);
		HANDLE_WND_STYLE(LBS_MULTIPLESEL, LBS_MULTIPLESEL);
		HANDLE_WND_STYLE(LBS_OWNERDRAWFIXED, LBS_OWNERDRAWFIXED);
		HANDLE_WND_STYLE(LBS_OWNERDRAWVARIABLE, LBS_OWNERDRAWVARIABLE);
		HANDLE_WND_STYLE(LBS_HASSTRINGS, LBS_HASSTRINGS);
		HANDLE_WND_STYLE(LBS_USETABSTOPS, LBS_USETABSTOPS);
		HANDLE_WND_STYLE(LBS_NOINTEGRALHEIGHT, LBS_NOINTEGRALHEIGHT);
		HANDLE_WND_STYLE(LBS_MULTICOLUMN, LBS_MULTICOLUMN);
		HANDLE_WND_STYLE(LBS_WANTKEYBOARDINPUT, LBS_WANTKEYBOARDINPUT);
		HANDLE_WND_STYLE(LBS_EXTENDEDSEL, LBS_EXTENDEDSEL);
		HANDLE_WND_STYLE(LBS_DISABLENOSCROLL, LBS_DISABLENOSCROLL);
		HANDLE_WND_STYLE(LBS_NODATA, LBS_NODATA);
		HANDLE_WND_STYLE(LBS_NOSEL, LBS_NOSEL);
		HANDLE_WND_STYLE(LBS_COMBOBOX, LBS_COMBOBOX);
	}
	else if (info.m_clsInfo.IsListView())
	{
		HANDLE_WND_STYLE(LVS_ICON, LVS_TYPEMASK);
		HANDLE_WND_STYLE(LVS_REPORT, LVS_TYPEMASK);
		HANDLE_WND_STYLE(LVS_SMALLICON, LVS_TYPEMASK);
		HANDLE_WND_STYLE(LVS_LIST, LVS_TYPEMASK);
		HANDLE_WND_STYLE(LVS_SINGLESEL, LVS_SINGLESEL);
		HANDLE_WND_STYLE(LVS_SHOWSELALWAYS, LVS_SHOWSELALWAYS);
		HANDLE_WND_STYLE(LVS_SORTASCENDING, LVS_SORTASCENDING);
		HANDLE_WND_STYLE(LVS_SORTDESCENDING, LVS_SORTDESCENDING);
		HANDLE_WND_STYLE(LVS_SHAREIMAGELISTS, LVS_SHAREIMAGELISTS);
		HANDLE_WND_STYLE(LVS_NOLABELWRAP, LVS_NOLABELWRAP);
		HANDLE_WND_STYLE(LVS_AUTOARRANGE, LVS_AUTOARRANGE);
		HANDLE_WND_STYLE(LVS_EDITLABELS, LVS_EDITLABELS);
		HANDLE_WND_STYLE(LVS_OWNERDATA, LVS_OWNERDATA);
		HANDLE_WND_STYLE(LVS_NOSCROLL, LVS_NOSCROLL);
		HANDLE_WND_STYLE(LVS_ALIGNTOP, LVS_ALIGNMASK);
		HANDLE_WND_STYLE(LVS_ALIGNLEFT, LVS_ALIGNMASK);
		HANDLE_WND_STYLE(LVS_OWNERDRAWFIXED, LVS_OWNERDRAWFIXED);
		HANDLE_WND_STYLE(LVS_NOCOLUMNHEADER, LVS_NOCOLUMNHEADER);
		HANDLE_WND_STYLE(LVS_NOSORTHEADER, LVS_NOSORTHEADER);
		DWORD dwExt = ListView_GetExtendedListViewStyle(info.m_hWnd);
		if (dwExt)
		{
			strText += _T("\r\n");
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_SUBITEMIMAGES, LVS_EX_SUBITEMIMAGES);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_TRACKSELECT, LVS_EX_TRACKSELECT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_HEADERDRAGDROP, LVS_EX_HEADERDRAGDROP);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_ONECLICKACTIVATE, LVS_EX_ONECLICKACTIVATE);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_TWOCLICKACTIVATE, LVS_EX_TWOCLICKACTIVATE);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_FLATSB, LVS_EX_FLATSB);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_REGIONAL, LVS_EX_REGIONAL);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_INFOTIP, LVS_EX_INFOTIP);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_UNDERLINEHOT, LVS_EX_UNDERLINEHOT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_UNDERLINECOLD, LVS_EX_UNDERLINECOLD);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_MULTIWORKAREAS, LVS_EX_MULTIWORKAREAS);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_LABELTIP, LVS_EX_LABELTIP);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_BORDERSELECT, LVS_EX_BORDERSELECT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_HIDELABELS, LVS_EX_HIDELABELS);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_SINGLEROW, LVS_EX_SINGLEROW);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_SNAPTOGRID, LVS_EX_SNAPTOGRID);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_SIMPLESELECT, LVS_EX_SIMPLESELECT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_JUSTIFYCOLUMNS, LVS_EX_JUSTIFYCOLUMNS);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_TRANSPARENTBKGND, LVS_EX_TRANSPARENTBKGND);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_TRANSPARENTSHADOWTEXT, LVS_EX_TRANSPARENTSHADOWTEXT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_AUTOAUTOARRANGE, LVS_EX_AUTOAUTOARRANGE);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_HEADERINALLVIEWS, LVS_EX_HEADERINALLVIEWS);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_AUTOCHECKSELECT, LVS_EX_AUTOCHECKSELECT);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_AUTOSIZECOLUMNS, LVS_EX_AUTOSIZECOLUMNS);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_COLUMNSNAPPOINTS, LVS_EX_COLUMNSNAPPOINTS);
			HANDLE_WND_STYLE_EXT(dwExt, LVS_EX_COLUMNOVERFLOW, LVS_EX_COLUMNOVERFLOW);
		}
	}
	else if (info.m_clsInfo.IsComboBox())
	{
		enum {CBS_TYPE_MASK = CBS_SIMPLE|CBS_DROPDOWN|CBS_DROPDOWNLIST};
		HANDLE_WND_STYLE(CBS_SIMPLE, CBS_TYPE_MASK);
		HANDLE_WND_STYLE(CBS_DROPDOWN, CBS_TYPE_MASK);
		HANDLE_WND_STYLE(CBS_DROPDOWNLIST, CBS_TYPE_MASK);
		HANDLE_WND_STYLE(CBS_OWNERDRAWFIXED, CBS_OWNERDRAWFIXED);
		HANDLE_WND_STYLE(CBS_OWNERDRAWVARIABLE, CBS_OWNERDRAWVARIABLE);
		HANDLE_WND_STYLE(CBS_AUTOHSCROLL, CBS_AUTOHSCROLL);
		HANDLE_WND_STYLE(CBS_OEMCONVERT, CBS_OEMCONVERT);
		HANDLE_WND_STYLE(CBS_SORT, CBS_SORT);
		HANDLE_WND_STYLE(CBS_HASSTRINGS, CBS_HASSTRINGS);
		HANDLE_WND_STYLE(CBS_NOINTEGRALHEIGHT, CBS_NOINTEGRALHEIGHT);
		HANDLE_WND_STYLE(CBS_DISABLENOSCROLL, CBS_DISABLENOSCROLL);
		HANDLE_WND_STYLE(CBS_UPPERCASE, CBS_UPPERCASE);
		HANDLE_WND_STYLE(CBS_LOWERCASE, CBS_LOWERCASE);
	}
	else if (info.m_clsInfo.IsHeaderCtrl())
	{
		HANDLE_WND_STYLE(HDS_BUTTONS, HDS_BUTTONS);
		HANDLE_WND_STYLE(HDS_HOTTRACK, HDS_HOTTRACK);
		HANDLE_WND_STYLE(HDS_HIDDEN, HDS_HIDDEN);
		HANDLE_WND_STYLE(HDS_DRAGDROP, HDS_DRAGDROP);
		HANDLE_WND_STYLE(HDS_FULLDRAG, HDS_FULLDRAG);
		HANDLE_WND_STYLE(HDS_FILTERBAR, HDS_FILTERBAR);
		HANDLE_WND_STYLE(HDS_FLAT, HDS_FLAT);
		HANDLE_WND_STYLE(HDS_CHECKBOXES, HDS_CHECKBOXES);
		HANDLE_WND_STYLE(HDS_NOSIZING, HDS_NOSIZING);
		HANDLE_WND_STYLE(HDS_OVERFLOW, HDS_OVERFLOW);
	}
	else if (info.m_clsInfo.IsScrollBar())
	{
		HANDLE_WND_STYLE(SBS_VERT, SBS_VERT);
		HANDLE_WND_STYLE(SBS_TOPALIGN, SBS_TOPALIGN);
		HANDLE_WND_STYLE(SBS_LEFTALIGN, SBS_LEFTALIGN);
		HANDLE_WND_STYLE(SBS_BOTTOMALIGN, SBS_BOTTOMALIGN);
		HANDLE_WND_STYLE(SBS_RIGHTALIGN, SBS_RIGHTALIGN);
		HANDLE_WND_STYLE(SBS_SIZEBOXTOPLEFTALIGN, SBS_SIZEBOXTOPLEFTALIGN);
		HANDLE_WND_STYLE(SBS_SIZEBOXBOTTOMRIGHTALIGN, SBS_SIZEBOXBOTTOMRIGHTALIGN);
		HANDLE_WND_STYLE(SBS_SIZEBOX, SBS_SIZEBOX);
		HANDLE_WND_STYLE(SBS_SIZEGRIP, SBS_SIZEGRIP);
	}
	else if (info.m_clsInfo.IsTreeView())
	{
		HANDLE_WND_STYLE(TVS_HASBUTTONS, TVS_HASBUTTONS);
		HANDLE_WND_STYLE(TVS_HASLINES, TVS_HASLINES);
		HANDLE_WND_STYLE(TVS_LINESATROOT, TVS_LINESATROOT);
		HANDLE_WND_STYLE(TVS_EDITLABELS, TVS_EDITLABELS);
		HANDLE_WND_STYLE(TVS_DISABLEDRAGDROP, TVS_DISABLEDRAGDROP);
		HANDLE_WND_STYLE(TVS_SHOWSELALWAYS, TVS_SHOWSELALWAYS);
		HANDLE_WND_STYLE(TVS_RTLREADING, TVS_RTLREADING);
		HANDLE_WND_STYLE(TVS_NOTOOLTIPS, TVS_NOTOOLTIPS);
		HANDLE_WND_STYLE(TVS_CHECKBOXES, TVS_CHECKBOXES);
		HANDLE_WND_STYLE(TVS_TRACKSELECT, TVS_TRACKSELECT);
		HANDLE_WND_STYLE(TVS_SINGLEEXPAND, TVS_SINGLEEXPAND);
		HANDLE_WND_STYLE(TVS_INFOTIP, TVS_INFOTIP);
		HANDLE_WND_STYLE(TVS_FULLROWSELECT, TVS_FULLROWSELECT);
		HANDLE_WND_STYLE(TVS_NOSCROLL, TVS_NOSCROLL);
		HANDLE_WND_STYLE(TVS_NONEVENHEIGHT, TVS_NONEVENHEIGHT);
		HANDLE_WND_STYLE(TVS_NOHSCROLL, TVS_NOHSCROLL);
		DWORD dwExt = TreeView_GetExtendedStyle(info.m_hWnd);
		if (dwExt)
		{
			strText += _T("\r\n");
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_NOSINGLECOLLAPSE, TVS_EX_NOSINGLECOLLAPSE);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_MULTISELECT, TVS_EX_MULTISELECT);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_NOINDENTSTATE, TVS_EX_NOINDENTSTATE);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_RICHTOOLTIP, TVS_EX_RICHTOOLTIP);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_AUTOHSCROLL, TVS_EX_AUTOHSCROLL);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_FADEINOUTEXPANDOS, TVS_EX_FADEINOUTEXPANDOS);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_PARTIALCHECKBOXES, TVS_EX_PARTIALCHECKBOXES);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_EXCLUSIONCHECKBOXES, TVS_EX_EXCLUSIONCHECKBOXES);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_DIMMEDCHECKBOXES, TVS_EX_DIMMEDCHECKBOXES);
			HANDLE_WND_STYLE_EXT(dwExt, TVS_EX_DRAWIMAGEASYNC, TVS_EX_DRAWIMAGEASYNC);
		}
	}
	else if (info.m_clsInfo.IsTabCtrl())
	{
		HANDLE_WND_STYLE(TCS_SCROLLOPPOSITE, TCS_SCROLLOPPOSITE);
		HANDLE_WND_STYLE(TCS_BOTTOM, TCS_BOTTOM);
		HANDLE_WND_STYLE(TCS_RIGHT, TCS_RIGHT);
		HANDLE_WND_STYLE(TCS_MULTISELECT, TCS_MULTISELECT);
		HANDLE_WND_STYLE(TCS_FLATBUTTONS, TCS_FLATBUTTONS);
		HANDLE_WND_STYLE(TCS_FORCEICONLEFT, TCS_FORCEICONLEFT);
		HANDLE_WND_STYLE(TCS_FORCELABELLEFT, TCS_FORCELABELLEFT);
		HANDLE_WND_STYLE(TCS_HOTTRACK, TCS_HOTTRACK);
		HANDLE_WND_STYLE(TCS_VERTICAL, TCS_VERTICAL);
		HANDLE_WND_STYLE(TCS_TABS, TCS_TABS);
		HANDLE_WND_STYLE(TCS_BUTTONS, TCS_BUTTONS);
		HANDLE_WND_STYLE(TCS_MULTILINE, TCS_MULTILINE);
		HANDLE_WND_STYLE(TCS_RIGHTJUSTIFY, TCS_RIGHTJUSTIFY);
		HANDLE_WND_STYLE(TCS_FIXEDWIDTH, TCS_FIXEDWIDTH);
		HANDLE_WND_STYLE(TCS_RAGGEDRIGHT, TCS_RAGGEDRIGHT);
		HANDLE_WND_STYLE(TCS_FOCUSONBUTTONDOWN, TCS_FOCUSONBUTTONDOWN);
		HANDLE_WND_STYLE(TCS_OWNERDRAWFIXED, TCS_OWNERDRAWFIXED);
		HANDLE_WND_STYLE(TCS_TOOLTIPS, TCS_TOOLTIPS);
		HANDLE_WND_STYLE(TCS_FOCUSNEVER, TCS_FOCUSNEVER);
		DWORD dwExt = TabCtrl_GetExtendedStyle(info.m_hWnd);
		if (dwExt)
		{
			strText += _T("\r\n");
			HANDLE_WND_STYLE_EXT(dwExt, TCS_EX_FLATSEPARATORS, TCS_EX_FLATSEPARATORS);
			HANDLE_WND_STYLE_EXT(dwExt, TCS_EX_REGISTERDROP, TCS_EX_REGISTERDROP);
		}
	}
	else if (info.m_clsInfo.IsStatic())
	{
		HANDLE_WND_STYLE(SS_CENTER, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_RIGHT, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_ICON, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_BLACKRECT, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_GRAYRECT, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_WHITERECT, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_BLACKFRAME, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_GRAYFRAME, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_WHITEFRAME, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_USERITEM, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_SIMPLE, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_LEFTNOWORDWRAP, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_OWNERDRAW, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_BITMAP, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_ENHMETAFILE, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_ETCHEDHORZ, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_ETCHEDVERT, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_ETCHEDFRAME, SS_TYPEMASK);
		HANDLE_WND_STYLE(SS_REALSIZECONTROL, SS_REALSIZECONTROL);
		HANDLE_WND_STYLE(SS_NOPREFIX, SS_NOPREFIX);
		HANDLE_WND_STYLE(SS_NOTIFY, SS_NOTIFY);
		HANDLE_WND_STYLE(SS_CENTERIMAGE, SS_CENTERIMAGE);
		HANDLE_WND_STYLE(SS_RIGHTJUST, SS_RIGHTJUST);
		HANDLE_WND_STYLE(SS_REALSIZEIMAGE, SS_REALSIZEIMAGE);
		HANDLE_WND_STYLE(SS_SUNKEN, SS_SUNKEN);
		HANDLE_WND_STYLE(SS_EDITCONTROL, SS_EDITCONTROL);
		HANDLE_WND_STYLE(SS_ENDELLIPSIS, SS_ELLIPSISMASK);
		HANDLE_WND_STYLE(SS_PATHELLIPSIS, SS_ELLIPSISMASK);
		HANDLE_WND_STYLE(SS_WORDELLIPSIS, SS_WORDELLIPSIS);
	}
}

static void _GetWindowInfoText(const CInspectWndInfo& info, CString& strText)
{
	CString strLine;

	strLine.Format(_T("HWND: 0x%" PRIXPTR), info.m_hWnd);
	strText += strLine;

	strText += _T("\r\nTitle: ");
	strText += info.m_strTitle;

	auto& wndClass = info.m_clsInfo;
	
	strText += _T("\r\nClass: ");
	strText += info.m_clsInfo.szClassName;
	if (info.m_clsInfo.IsMenu())
		strText += _T(" (Menu)");
	else if (info.m_clsInfo.IsDesktop())
		strText += _T(" (Desktop)");
	else if (info.m_clsInfo.IsDialog())
		strText += _T(" (Dialog)");
	else if (info.m_clsInfo.IsTaskSwitchWnd())
		strText += _T(" (Task Switch Window)");
	else if (info.m_clsInfo.IsIconTitles())
		strText += _T(" (Icon Titles)");

	strLine.Format(_T("Screen: (%d, %d) - (%d, %d)  -  %d x %d"), 
		info.m_rectWnd.left, info.m_rectWnd.top, info.m_rectWnd.right, info.m_rectWnd.bottom,
		info.m_rectWnd.Width(), info.m_rectWnd.Height());
	strText += _T("\r\n");
	strText += strLine;

	strLine.Format(_T("Client: (%d, %d) - (%d, %d)  -  %d x %d"),
		info.m_rectClient.left, info.m_rectClient.top, info.m_rectClient.right, info.m_rectClient.bottom,
		info.m_rectClient.Width(), info.m_rectClient.Height());
	strText += _T("\r\n");
	strText += strLine;

	if (info.m_dwStyle & WS_CHILD)
	{
		strLine.Format(_T("Client (To Parent): (%d, %d) - (%d, %d)  -  %d x %d"),
			info.m_rectClientToParent.left, info.m_rectClientToParent.top, info.m_rectClientToParent.right, info.m_rectClientToParent.bottom,
			info.m_rectClientToParent.Width(), info.m_rectClientToParent.Height());
		strText += _T("\r\n");
		strText += strLine;
	}

	strLine.Format(_T("HMODULE: 0x%-16" PRIXPTR "  Atom: 0x%" PRIXPTR), wndClass.hModule, wndClass.atom);
	strText += _T("\r\n");
	strText += strLine;

	strLine.Format(_T("WndProc: 0x%-16" PRIXPTR "  Menu: 0x%" PRIXPTR), wndClass.lpfnWndProc, wndClass.lpszMenuName);
	strText += _T("\r\n");
	strText += strLine;

	strLine.Format(_T("HICON:   0x%-16" PRIXPTR "  HICON(sm): 0x%" PRIXPTR), wndClass.hIcon, wndClass.hIconSm);
	strText += _T("\r\n");
	strText += strLine;

	CString strBrush;
	if (_GetBrushText((ULONG_PTR)wndClass.hbrBackground, strBrush))
		strLine.Format(_T("HCURSOR: 0x%-16" PRIXPTR "  Brush: %s"), wndClass.hCursor, (LPCTSTR)strBrush);
	else
		strLine.Format(_T("HCURSOR: 0x%-16" PRIXPTR "  Brush: 0x%" PRIXPTR), wndClass.hCursor, wndClass.hbrBackground);
	strText += _T("\r\n");
	strText += strLine;

	strLine.Format(_T("Class Styles: 0x%" PRIXPTR), wndClass.style);
	strText += _T("\r\n");
	strText += strLine;
	if (wndClass.style)
	{
		strText += _T("\r\n");
		int nItems = 0;
#define HANDLE_CLASS_STYLE(_x)	\
		if (wndClass.style & _x) \
		{\
			if (nItems)\
			{\
				strText += (nItems % 3) ? _T("\t") : _T("\r\n");\
			}\
			++nItems;\
			strText += _T(#_x);\
		}

		HANDLE_CLASS_STYLE(CS_VREDRAW);
		HANDLE_CLASS_STYLE(CS_HREDRAW);
		HANDLE_CLASS_STYLE(CS_DBLCLKS);
		HANDLE_CLASS_STYLE(CS_OWNDC);
		HANDLE_CLASS_STYLE(CS_CLASSDC);
		HANDLE_CLASS_STYLE(CS_PARENTDC);
		HANDLE_CLASS_STYLE(CS_NOCLOSE);
		HANDLE_CLASS_STYLE(CS_SAVEBITS);
		HANDLE_CLASS_STYLE(CS_BYTEALIGNCLIENT);
		HANDLE_CLASS_STYLE(CS_BYTEALIGNWINDOW);
		HANDLE_CLASS_STYLE(CS_GLOBALCLASS);
		HANDLE_CLASS_STYLE(CS_IME);
		HANDLE_CLASS_STYLE(CS_DROPSHADOW);

		strText += _T("\r\n");
	}
	strText += _T("\r\n");

	if (info.m_dwStyle & WS_CHILD)
	{
		strLine.Format(_T("Control ID: 0x%" PRIXPTR " (%d)"), info.m_nCtrlID, info.m_nCtrlID);
		strText += _T("\r\n");
		strText += strLine;
	}

	strLine.Format(_T("Style: 0x%" PRIXPTR "    ExStyle: 0x%" PRIXPTR), info.m_dwStyle, info.m_dwStyleEx);
	strText += _T("\r\n");
	strText += strLine;

	strLine.Empty();
	_GetWindowStyleText(strLine, info);
	if (!strLine.IsEmpty())
	{
		strText += _T("\r\n");
		strText += strLine;
	}
}

void CWindowInspectorDlg::UpdateWindowInfo()
{
	CPoint pos(0,0);
	GetCursorPos(&pos);
	HWND hWndInspect = ::WindowFromPoint(pos);
	if (!hWndInspect)
		return;

	m_hWndInspect = hWndInspect;

	CWnd* pWndInspect = CWnd::FromHandle(m_hWndInspect);
	
	CString strLine, strText;
	strLine.Format(_T("Pos: %d, %d"), pos.x, pos.y);
	SetDlgItemText(IDC_STATIC_CURSORPOS, strLine);

	CInspectWndInfo info;
	info.Init(m_hWndInspect);
	_GetWindowInfoText(info, strText);

	m_editWndInfo.SetWindowText(strText);

	HWND hParent = ::GetAncestor(m_hWndInspect, GA_PARENT);
	HWND hOwner = ::GetWindow(m_hWndInspect, GW_OWNER);
	CString strParentText;
	if (hParent)
	{
		info.Init(hParent);
		strParentText += _T("Parent window ");
		_GetWindowInfoText(info, strParentText);

		HWND hNextParent = ::GetAncestor(hParent, GA_PARENT);
		while (hNextParent)
		{
			info.Init(hNextParent);
			strParentText += _T("Parent window ");
			_GetWindowInfoText(info, strParentText);
			hNextParent = ::GetAncestor(hNextParent, GA_PARENT);
		}
	}
	if (hOwner && hOwner != hParent)
	{
		info.Init(hOwner);
		strParentText += _T("\r\n----------------------------------------------------------------\r\n");
		strParentText += _T("Owner window:\r\n");
		_GetWindowInfoText(info, strParentText);
	}
	if (!strParentText.IsEmpty())
	{
		m_editParentWndInfo.SetWindowText(strParentText);
	}
}


void CWindowInspectorDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_UPDATE_WIN_INFO_ID)
	{
		// need to check m_nUpdateWndInfoTimer here since 
		// KillTimer does not remove WM_TIMER messages already posted to the message queue
		if (m_nUpdateWndInfoTimer)
			UpdateWindowInfo();
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


void CWindowInspectorDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (nHotKeyId == IDH_FREEZE)
	{
		if (m_nUpdateWndInfoTimer)
		{
			KillTimer(m_nUpdateWndInfoTimer);
			m_nUpdateWndInfoTimer = 0;
			SetWindowText(m_strOriginalTitle + _T(" (Freezed)"));
		}
		else
		{
			SetWindowText(m_strOriginalTitle);
			m_nUpdateWndInfoTimer = SetTimer(TIMER_UPDATE_WIN_INFO_ID, TIMER_UPDATE_WIN_INFO_ELAPSE, nullptr);
			UpdateWindowInfo();
		}
	}
	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CWindowInspectorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (!m_editWndInfo.m_hWnd || !m_editParentWndInfo.m_hWnd)
		return;
	CSize szNew(cx, cy);
	CSize szDiff = szNew - m_szDlg;

	int nWidth = (cx - m_nEditGapToEdge * 2 - m_nEditGapToEdit) / 2;
	CRect rect;
	m_editWndInfo.GetWindowRect(rect);
	ScreenToClient(rect);
	rect.right = rect.left + nWidth;
	rect.bottom += szDiff.cy;
	m_editWndInfo.SetWindowPos(nullptr, -1, -1, rect.Width(), rect.Height(), SWP_NOMOVE);

	int nTemp = rect.right;
	m_editParentWndInfo.GetWindowRect(rect);
	ScreenToClient(rect);
	rect.left = nTemp + m_nEditGapToEdit;
	rect.right = rect.left + nWidth;
	rect.bottom += szDiff.cy;
	m_editParentWndInfo.SetWindowPos(nullptr, rect.left, rect.top, rect.Width(), rect.Height(), 0);

	m_szDlg = szNew;
}
