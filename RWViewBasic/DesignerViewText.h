// DesignerViewText.h : Declaration of the CDesignerViewText

#pragma once
#include "resource.h"       // main symbols
#include "RWViewBasic.h"
#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include <RWDocumentBasic.h>
#include <Platform.h>
#include <Scintilla.h>
#include <SciLexer.h>


// CDesignerViewText

class ATL_NO_VTABLE CDesignerViewText : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewText>,
	public CObserverImpl<CDesignerViewText, ITextObserver, LONG>,
	public CDesignerViewWndImpl<CDesignerViewText, IDesignerView>,
	public IDesignerViewClipboardHandler
{
public:
	CDesignerViewText() : m_bModifying(false)
	{
	}
	~CDesignerViewText()
	{
		if (m_pScintilla)
			m_pScintilla->UnregisterClasses();
	}
	void Init(ISharedStateManager* a_pStateMgr, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID, IScintillaFactory* a_pScintilla);

   DECLARE_WND_CLASS_EX(_T("DesignerViewText"), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW + 1)

BEGIN_COM_MAP(CDesignerViewText)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewClipboardHandler)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewText)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	NOTIFY_HANDLER(IDC_TEXT_TEXT, SCN_MODIFIED, OnTextModified)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pTextDoc)
			m_pTextDoc->ObserverDel(CObserverImpl<CDesignerViewText, ITextObserver, LONG>::ObserverGet(), 0);
	}

public:
	void OwnerNotify(TCookie a_tCookie, LONG a_nLineIndex);

	// IDesignerViewClipboardHandler
public:
	STDMETHOD(Priority)(BYTE* a_pPrio) { return E_NOTIMPL; }
	STDMETHOD(ObjectName)(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName);
	STDMETHOD(ObjectIconID)(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID) { return E_NOTIMPL; }
	STDMETHOD(ObjectIcon)(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay) { return E_NOTIMPL; }
	STDMETHOD(Check)(EDesignerViewClipboardAction a_eAction);
	STDMETHOD(Exec)(EDesignerViewClipboardAction a_eAction);

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnTextModified(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);

private:
	enum {IDC_TEXT_TEXT = 100};

private:
	CComPtr<IDocumentText> m_pTextDoc;
	CComPtr<IScintillaFactory> m_pScintilla;

	LCID m_tLocaleID;
	CFont m_cFont;
	EDesignerViewWndStyle m_nStyle;
	CWindow m_wndClient;
	bool m_bModifying;
};

