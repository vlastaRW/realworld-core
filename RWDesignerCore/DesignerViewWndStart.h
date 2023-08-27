// DesignerViewWndStart.h : Declaration of the CDesignerViewWndStart

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <Win32LangEx.h>
#include "ThreadCommand.h" // TODO: remove
#include "StartViewWnd.h"


class IDocumentControl
{
public:
	virtual void SetNewDocument(IDocument* a_pNewDoc) = 0;
};

// CDesignerViewWndStart

class ATL_NO_VTABLE CDesignerViewWndStart : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDesignerViewWndImpl<CDesignerViewWndStart, IDesignerView>,
	public Win32LangEx::CLangIndirectDialogImpl<CDesignerViewWndStart>,
	public IDesignerViewStatusBar
{
public:
	CDesignerViewWndStart() : m_pDocCtrl(NULL), m_hToolBarImages(NULL),
		m_pInternObserver(NULL), m_bShowButtonDesc(false),
		m_pStartPage(NULL), m_nTBGaps(6), m_nLastHotButton(0),
		iconSize(48)
	{
		CComObject<CInternObserver>::CreateInstance(&m_pInternObserver);
		m_pInternObserver->AddRefAndLink(this);
	}
	~CDesignerViewWndStart()
	{
		if (m_hToolBarImages)
			ImageList_Destroy(m_hToolBarImages);
		ATLVERIFY(m_pInternObserver->ReleaseAndUnlink() == 0);
		ClearAnimationCache();
	}
	void Init(IConfig* a_pMainCfg, IDocumentControl* a_pDocCtrl, IStatusBarObserver* a_pStatusBar, HWND a_hParentWnd, RECT const* a_prcArea, LCID a_tLocaleID, size_t a_nPages, CLSID const* a_pPages, CLSID const& a_tConfigStartPage, CLSID const& a_tExplicitStartPage, IStorageManager* a_pStorageManager, ILocalizedString* a_pCaption);
	virtual void OnFinalMessage(HWND a_hWnd);

	enum
	{
		IDC_TOOLBAR = 1021,
		ID_PAGE1 = 666,
		ANIMATIONTIMERID = 71,
	};

	BEGIN_DIALOG_EX(0, 0, 333, 231, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_TOOLBAR, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_CUSTOMERASE | TBSTYLE_FLAT | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | CCS_TOP | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 0, 0, 63, 231, 0)
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(CDesignerViewWndStart)
		COM_INTERFACE_ENTRY(IDesignerView)
		COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	END_COM_MAP()

	BEGIN_MSG_MAP(CDesignerViewWndStart)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		//MESSAGE_HANDLER(WM_USER_SELECTED, OnSelected)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		//COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_RANGE_HANDLER(ID_PAGE1, ID_PAGE1+10, OnSelectPage)
		NOTIFY_HANDLER(IDC_TOOLBAR, TBN_HOTITEMCHANGE, OnHotItemChange)
		NOTIFY_HANDLER(IDC_TOOLBAR, NM_CUSTOMDRAW, OnTBCustomDraw)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()


	// message handlers
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelectPage(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnHotItemChange(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnTBCustomDraw(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// IStartViewCallback methods
public:
	void ReportError(ILocalizedString* a_pMessage);
	void OpenDocument(IDocument* a_pDoc);
	void OnOKEx();
	void UpdateStatusBar() { if (m_pStatusBar) m_pStatusBar->Notify(0, 0); }

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView methods
public:
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
	{
		try
		{
			if (a_eFilter != EQIFActive)
			{
				CComPtr<IUnknown> pMy;
				QueryInterface(a_iid, reinterpret_cast<void**>(&pMy));
				if (pMy) a_pInterfaces->Insert(pMy);
			}

			IStartViewPage* p = NULL;
			for (CStartPages::const_iterator i = m_aWnds.begin(); i != m_aWnds.end(); ++i)
			{
				if (IsEqualCLSID(i->tClsID, m_tStartPage) && i->pPage)
				{
					CComPtr<IUnknown> p;
					i->pPage->QueryInterface(a_iid, reinterpret_cast<void**>(&p));
					if (p == NULL)
						return S_FALSE;
					return a_pInterfaces->Insert(p);
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(OnIdle)();

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		if (m_bShowButtonDesc)
		{
			a_pStatusBar->SimpleModeSet(m_bstrButtonDesc);
		}
		else
		{
			CComQIPtr<IDesignerViewStatusBar> pPageSB(m_pStartPage);
			if (pPageSB)
				return pPageSB->Update(a_pStatusBar);
		}
		return S_OK;
	}

private:
	class CInternObserver : public CComObjectRootEx<CComMultiThreadModel>, public IStartViewCallback
	{
	public:
		CInternObserver() : m_pOwner(NULL)
		{
		}

	BEGIN_COM_MAP(CInternObserver)
		COM_INTERFACE_ENTRY(IStartViewCallback)
	END_COM_MAP()

		ULONG AddRefAndLink(CDesignerViewWndStart* a_pOwner)
		{
			m_pOwner = a_pOwner;
			return AddRef();
		}
		ULONG ReleaseAndUnlink()
		{
			{
				ObjectLock cLock(this);
				m_pOwner = NULL;
			}
			return Release();
		}
		STDMETHOD(ReportError)(ILocalizedString* a_pMessage)
		{
			CDesignerViewWndStart* pOwner;
			{
				ObjectLock cLock(this);
				pOwner = m_pOwner;
			}
			if (pOwner)
				pOwner->ReportError(a_pMessage);
			return S_OK;
		}
		STDMETHOD(OpenDocument)(IDocument* a_pDoc)
		{
			CDesignerViewWndStart* pOwner;
			{
				ObjectLock cLock(this);
				pOwner = m_pOwner;
			}
			if (pOwner)
			{
				CLSID t = GUID_NULL;
				pOwner->OpenDocument(a_pDoc->BuilderID(&t) == S_OK && !IsEqualCLSID(t, GUID_NULL) ? a_pDoc : NULL);
			}
			return S_OK;
		}
		STDMETHOD(OnOKEx)()
		{
			CDesignerViewWndStart* pOwner;
			{
				ObjectLock cLock(this);
				pOwner = m_pOwner;
			}
			if (pOwner)
				pOwner->OnOKEx();
			return S_OK;
		}
		STDMETHOD(UpdateStatusBar)()
		{
			CDesignerViewWndStart* pOwner;
			{
				ObjectLock cLock(this);
				pOwner = m_pOwner;
			}
			if (pOwner)
				pOwner->UpdateStatusBar();
			return S_OK;
		}

	private:
		CDesignerViewWndStart* m_pOwner;
	};

	struct SStartPageInfo
	{
		CLSID tClsID;
		CComPtr<IStartViewPageFactory> pFactory;
		CComPtr<IStartViewPage> pPage;
	};
	typedef std::vector<SStartPageInfo> CStartPages;

	typedef std::map<ULONG, HICON> CAnimationCache;

private:
	void ShowHidePages();
	RECT GetPageRectangle(int a_nSizeX, int a_nSizeY) const;
	void ClearAnimationCache();

private:
	CComObject<CInternObserver>* m_pInternObserver;
	IDocumentControl* m_pDocCtrl;
	CComPtr<IConfig> m_pMainCfg;
	CComPtr<IStorageManager> m_pStorageManager;
	CComPtr<ILocalizedString> m_pCaption;

	GUID m_tStartPage;
	IStartViewPage* m_pStartPage;
	CComPtr<IAnimatedIcon> m_pAnimation;
	ULONG m_dwAnimationStart;
	CAnimationCache m_cAnimationCache;
	RECT m_rcToolBar;
	CToolBarCtrl m_wndToolBar;
	HIMAGELIST m_hToolBarImages;
	ULONG m_nTBGaps;
	ULONG iconSize;
	//CButtonWithPopup m_wndOKButton;

	CStartPages m_aWnds;
	ULONG m_timestamp;

	bool m_bShowButtonDesc;
	UINT m_nLastHotButton;
	CComBSTR m_bstrButtonDesc;
	CComPtr<IStatusBarObserver> m_pStatusBar;

	CComPtr<ILocalizedString> m_pSubmenuTemplate;
};
