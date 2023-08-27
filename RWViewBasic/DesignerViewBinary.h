// DesignerViewBinary.h : Declaration of the CDesignerViewBinary

#pragma once
#include "resource.h"       // main symbols
#include "RWViewBasic.h"
#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include <RWDocumentBasic.h>
#include <atlgdix.h>


// CDesignerViewBinary

class ATL_NO_VTABLE CDesignerViewBinary : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewBinary>,
	public COffscreenDraw<CDesignerViewBinary>,
	public CObserverImpl<CDesignerViewBinary, IBinaryObserver, TBinaryChange>,
	public CDesignerViewWndImpl<CDesignerViewBinary, IDesignerView>
{
public:
	CDesignerViewBinary() :
		m_dwPos(0), 
		m_bShowAddress(true),
		m_bShowData(true),
		m_bShowAscii(true),
		m_bInDataPane(true),
		m_iDataSize(1),
		m_dwDigitOffset(0),
		m_cInvalidAscii('.'),
		m_dwSelStart((DWORD)-1),
		m_dwSelEnd((DWORD)-1),
		m_bReadOnly(FALSE)
	{
		::ZeroMemory(&m_rcData, sizeof(m_rcData));
		::ZeroMemory(&m_rcAscii, sizeof(m_rcAscii));
		m_szMargin.cx = 3;
		m_szMargin.cy = 2;
	}
	void Init(ISharedStateManager* a_pStateMgr, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID);

   DECLARE_WND_CLASS_EX(_T("DesignerViewBinary"), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW + 1)

BEGIN_COM_MAP(CDesignerViewBinary)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewBinary)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	MESSAGE_HANDLER(WM_CHAR, OnChar)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
	COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnEditUndo)
	COMMAND_ID_HANDLER(ID_NEXT_PANE, OnNextPane)
	CHAIN_MSG_MAP( COffscreenDraw<CDesignerViewBinary> )
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pBinDoc)
			m_pBinDoc->ObserverDel(CObserverImpl<CDesignerViewBinary, IBinaryObserver, TBinaryChange>::ObserverGet(), 0);
	}

public:
	void OwnerNotify(TCookie a_tCookie, TBinaryChange a_tChange)
	{
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);      
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);   
	LRESULT OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditUndo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNextPane(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void RecalcCaret();
	void RecalcPosition(DWORD dwPos);
	void AssignDigitValue(DWORD& dwPos, DWORD& dwCursorPos, BYTE bValue);
	void AssignCharValue(DWORD& dwPos, DWORD& dwCursorPos, BYTE bValue);
	BOOL GetPosFromPoint(POINT pt, DWORD& dwPos, bool& bDataActive);

	ATLINLINE TCHAR toupper(WPARAM c) const { return (TCHAR)( c - 'a' + 'A' ); };
	ATLINLINE bool isdigit(WPARAM c) const { return c >= '0' && c <= '9'; };
	ATLINLINE bool isprint(char c) const { WORD w; ::GetStringTypeA(LOCALE_USER_DEFAULT, CT_CTYPE1, &c, 1, &w); return w != C1_CNTRL; };

	void DoPaint(CDCHandle dc);

	int GetLinesPrPage() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		RECT rcClient;
		GetClientRect(&rcClient);
		return (rcClient.bottom - rcClient.top) / GetLineHeight();
	}
	BOOL SetSel(DWORD dwStart, DWORD dwEnd, BOOL bNoScroll = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if( dwEnd == (DWORD) -1 ) dwEnd = M_DataLen();
		if( dwStart == dwEnd && dwEnd >= M_DataLen() ) dwEnd = M_DataLen() - 1;
		if( dwStart != dwEnd && dwEnd > M_DataLen() ) dwEnd = M_DataLen();
		if( dwStart >= M_DataLen() ) dwStart = M_DataLen() - 1;
		dwStart = dwStart / m_iDataSize * m_iDataSize;
		dwEnd = dwEnd / m_iDataSize * m_iDataSize;
		if( dwEnd == dwStart && m_dwSelStart != m_dwSelEnd ) ShowCaret();
		if( dwEnd != dwStart && m_dwSelStart == m_dwSelEnd ) HideCaret();
		m_dwSelStart = dwStart;
		m_dwSelEnd = dwEnd;
		m_dwDigitOffset = 0;
		if( !bNoScroll ) RecalcPosition(dwEnd);
		if( dwStart == dwEnd ) RecalcCaret();
		return Invalidate();
	}
	void GetSel(DWORD& dwStart, DWORD& dwEnd) const
	{
		dwStart = m_dwSelStart;
		dwEnd = m_dwSelEnd;
		if( dwStart > dwEnd) {        // Return values in normalized form
			DWORD dwTemp = dwStart;
			dwStart = dwEnd;
			dwEnd = dwTemp;
		}
	}
	int GetLineHeight() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return m_tmEditor.tmHeight;
	}

private:
   enum { BYTES_PR_LINE = 16 };

   ULONG M_DataLen()
   {
	   ULONG n = 0;
	   if (m_pBinDoc) m_pBinDoc->Size(&n);
	   return n;
   }

private:
	CComPtr<IDocumentBinary> m_pBinDoc;
	EDesignerViewWndStyle m_nStyle;

	DWORD m_dwPos;                          // File Position of top of view
	CFont m_fontEditor;                     // Font used in Editor
	TEXTMETRIC m_tmEditor;                  // Font text metrics
	bool  m_bShowAddress;                   // Show address dump?
	bool  m_bShowData;                      // Show Hex dump?
	bool  m_bShowAscii;                     // Show ASCII dump?
	TCHAR m_cInvalidAscii;                  // Character to use when char is unprintable
	BYTE  m_iDataSize;                      // Size of address (1, 2, 4)
	RECT  m_rcData;                         // Rectangle of data (hex) area
	RECT  m_rcAscii;                        // Rectangle of ASCII area
	DWORD m_dwSelStart;                     // Selection start
	DWORD m_dwSelEnd;                       // Selection end
	DWORD m_dwDigitOffset;                  // Cursor digit-position inside value
	bool  m_bInDataPane;                    // Selection is in data pane!
	SIZE  m_szMargin;                       // Left/top margin
	BOOL  m_bReadOnly;                      // Is control read-only?
};

