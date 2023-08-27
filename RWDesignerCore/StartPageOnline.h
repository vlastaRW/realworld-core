// StartPageOnline.h : Declaration of the CStartPageOnline

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include "StartViewWnd.h"
#include <Win32LangEx.h>
#include <exdispid.h>
#include <RWLocalization.h>
#include <IconRenderer.h>
#include <math.h>

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

extern __declspec(selectany) OLECHAR const CFGID_ONLINEPROPS[] = L"OnlineProps";


// CStartPageOnline

class ATL_NO_VTABLE CStartPageOnline :
	public CStartViewPageImpl<CStartPageOnline>,
	public IDesignerViewStatusBar,
	public IDocHostUIHandler2,
	public IDocHostShowUI,
	public IDispEventImpl<0, CStartPageOnline, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>,
	public Win32LangEx::CLangIndirectDialogImpl<CStartPageOnline>
{
public:
	CStartPageOnline() : m_bOverwrite(false)
	{
	}

	enum { IDC_IECTL = 100, IDC_CMDTB, WM_REFRESHPAGE = WM_APP+391 };

	BEGIN_DIALOG_EX(0, 0, 100, 50, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T(""), IDC_IECTL, 0, 0, 100, 50, WS_VISIBLE|WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CStartPageOnline)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_REFRESHPAGE, OnRefreshPage)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	BEGIN_COM_MAP(CStartPageOnline)
		COM_INTERFACE_ENTRY(IStartViewPage)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
		COM_INTERFACE_ENTRY(IDocHostUIHandler)
		COM_INTERFACE_ENTRY(IDocHostUIHandler2)
		COM_INTERFACE_ENTRY(IDocHostShowUI)
		COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_pUnkContainer)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return CAxHostWindow::_CreatorClass::CreateInstance(this, IID_IUnknown, (void**)&m_pUnkContainer);
	}
	
	void FinalRelease() 
	{
	}

	BEGIN_SINK_MAP(CStartPageOnline)
		//SINK_ENTRY(IDC_IECTL, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
		SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2)
		SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_STATUSTEXTCHANGE, StatusTextChange)
		SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NAVIGATEERROR, NavigateError)
		//SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, NavigateComplete2)
		//SINK_ENTRY(IDC_IECTL, 0x6a, OnDownloadBegin)
	END_SINK_MAP()
 
	void STDMETHODCALLTYPE BeforeNavigate2(IDispatch* pDisp, VARIANT* pvarURL, VARIANT* pvarFlags, VARIANT* pvarTarget, VARIANT* pvarPostData, VARIANT* pvarHeaders, VARIANT_BOOL* pbCancel)
	{
		if (pvarURL && pvarURL->vt == VT_BSTR && pvarURL->bstrVal && (_wcsnicmp(pvarURL->bstrVal, L"http://", 7) == 0 || _wcsnicmp(pvarURL->bstrVal, L"https://", 8) == 0) && m_bstrURL != pvarURL->bstrVal)
		{
			ShellExecute(NULL, _T("open"), COLE2CT(pvarURL->bstrVal), NULL, NULL, SW_SHOW);
			*pbCancel = VARIANT_TRUE;
		}
	}
	void STDMETHODCALLTYPE NavigateError(IDispatch* pDisp, VARIANT* URL, VARIANT* TargetFrameName, VARIANT* StatusCode, VARIANT_BOOL* Cancel)
	{
		TCHAR szErrorURL[MAX_PATH+64] = _T("res://");
		TCHAR* p = szErrorURL+_tcslen(szErrorURL);
		GetModuleFileName(_pModule->get_m_hInst(), p, MAX_PATH);
		p = p+_tcslen(p);
		_stprintf(p, _T("/#%i/#%i"), RT_HTML, IDR_OFFLINE0409);
		*Cancel = VARIANT_TRUE;
		CComBSTR bstrErrorURL(szErrorURL);
		m_pWebBrowser->Navigate(bstrErrorURL, NULL, NULL, NULL, NULL);
	}

	//void NavigateComplete2(IDispatch* pDisp, VARIANT* pvarURL)
	//{
	//	if (pvarURL && pvarURL->vt == VT_BSTR && pvarURL->bstrVal && _wcsnicmp(pvarURL->bstrVal, L"http://", 7) != 0)
	//	{
	//		m_pWebBrowser->Navigate(L"http://localhost/RW/start/RWPaint?2009.1.0", NULL, NULL, NULL, NULL);
	//	}
	//}

	void STDMETHODCALLTYPE StatusTextChange(BSTR text)
	{
		if (m_bstrStatusText != text)
		{
			m_bstrStatusText = text;
			m_pClbk->UpdateStatusBar();
		}
	}

	void WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig)
	{
		CComObject<CExternal>* pExt = NULL;
		CComObject<CExternal>::CreateInstance(&pExt);
		m_pExternal = pExt;
		CComPtr<IConfig> pSubCfg;
		if (a_pAppConfig) a_pAppConfig->SubConfigGet(CComBSTR(CFGID_ONLINEPROPS), &pSubCfg);
		pExt->Init(m_tLocaleID, this, pSubCfg, a_pAppConfig);
		AtlAxWinInit();
		static INITCOMMONCONTROLSEX tICCEx = {sizeof tICCEx, ICC_BAR_CLASSES};
		static BOOL bDummy = InitCommonControlsEx(&tICCEx);
		m_pClbk = a_pCallback;
		m_tLocaleID = a_tLocaleID;
		Create(a_hParent);
		MoveWindow(a_prc);
		GotoDlgCtrl(GetDlgItem(IDC_IECTL));
	}
	void Refresh();

	static HRESULT InitConfig(IConfigWithDependencies* a_pRoot)
	{
		CComPtr<ISubConfig> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		a_pRoot->ItemInsSimple(CComBSTR(CFGID_ONLINEPROPS), NULL, NULL, CConfigValue(0L), pMemCfg.p, 0, NULL);

		return S_OK;
	}

	// IChildWindow methods
public:
	STDMETHOD(Destroy)();
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IStartViewWnd methods
public:
	STDMETHOD(Activate)();
	STDMETHOD(Deactivate)();
	STDMETHOD(ClickedDefault)();
	STDMETHOD(HasChoices)() { return S_FALSE; }
	STDMETHOD(GetChoiceProps)(ULONG a_nIndex, ILocalizedString** UNREF(a_ppName), ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon)) { return E_NOTIMPL; }
	STDMETHOD(ClickedChoice)(ULONG UNREF(a_nIndex)) { return E_NOTIMPL; }

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		if (m_bstrStatusTextOverride && m_bstrStatusTextOverride[0])
			a_pStatusBar->SimpleModeSet(m_bstrStatusTextOverride);
		else if (m_bstrStatusText && m_bstrStatusText[0])
			a_pStatusBar->SimpleModeSet(m_bstrStatusText);
		return S_OK;
	}

	// IDocHostUIHandler methods
public:
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved)
	{
#ifdef _DEBUG
		return S_FALSE;
#else
		return S_OK;
#endif
	}
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO* pInfo);
	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc) { return S_FALSE; }
	STDMETHOD(HideUI)() { return S_FALSE; }
	STDMETHOD(UpdateUI)() { return S_FALSE; }
	STDMETHOD(EnableModeless)(BOOL fEnable) { return S_OK; }
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate) { return S_FALSE; }
	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate) { return S_FALSE; }
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fRameWindow) { return S_FALSE; }
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, GUID const* pguidCmdGroup, DWORD nCmdID) { return S_FALSE; }
	STDMETHOD(GetOptionKeyPath)(LPOLESTR* pchKey, DWORD dw);
	STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget) { return E_NOTIMPL; }
	STDMETHOD(GetExternal)(IDispatch** ppDispatch) { (*ppDispatch = m_pExternal)->AddRef(); return S_OK; }
	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut) { *ppchURLOut = NULL; return S_FALSE; }
	STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet) { if (ppDORet) *ppDORet = NULL; return S_FALSE; }

	// IDocHostUIHandler2 methods
public:
	STDMETHOD(GetOverrideKeyPath)(LPOLESTR* pchKey, DWORD dw) { return S_FALSE; }

	//IDocHostShowUI methods
public:
	STDMETHOD(ShowMessage)(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult) { return S_FALSE; }
	STDMETHOD(ShowHelp)(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit) { return S_FALSE; }

	// handlers
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRefreshPage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	class ATL_NO_VTABLE CExternal : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDispatchImpl<IOnlinePageExternal, &IID_IOnlinePageExternal, &LIBID_RWDesignerCoreLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
	{
	public:
		CExternal() : m_tLocaleID(0), m_pOwner(NULL)
		{
		}
		void Init(LCID a_tLocaleID, CStartPageOnline* a_pOwner, CComPtr<IConfig>& a_pContext, IConfig* a_pAppConfig)
		{
			m_tLocaleID = a_tLocaleID;
			m_pOwner = a_pOwner;
			if (a_pContext)
				a_pContext->QueryInterface(&m_pContext);
			m_pMainConfig = a_pAppConfig;
		}


	DECLARE_NOT_AGGREGATABLE(CExternal)

	BEGIN_COM_MAP(CExternal)
		COM_INTERFACE_ENTRY(IOnlinePageExternal)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()


		DECLARE_PROTECT_FINAL_CONSTRUCT()

		HRESULT FinalConstruct()
		{
			return S_OK;
		}
		
		void FinalRelease() 
		{
		}

		// IOnlinePageExternal methods
	public:
		STDMETHOD(IsInstalled)(BSTR plugInID, VARIANT_BOOL* res);
		STDMETHOD(InstallPlugIn)(BSTR plugInID, BSTR version, VARIANT_BOOL* res);
		STDMETHOD(RemovePlugIn)(BSTR plugInID, VARIANT_BOOL* res);
		STDMETHOD(GetPlugInVersion)(BSTR plugInID, BSTR* ver);
		STDMETHOD(get_AppLangID)(ULONG* pnLangID);
		STDMETHOD(put_AppLangID)(ULONG nLangID);
		STDMETHOD(get_AppLangStamp)(ULONG* pnLangStamp);
		STDMETHOD(get_OSLangID)(ULONG* pnLangID);
		STDMETHOD(UpdateLanguage)(VARIANT_BOOL* res);
		STDMETHOD(get_EvalMax)(ULONG* pnVal);
		STDMETHOD(get_EvalDay)(ULONG* pnVal);
		STDMETHOD(get_LicName)(BSTR* pVal);
		STDMETHOD(get_LicOrg)(BSTR* pVal);
		STDMETHOD(get_LicCode)(BSTR* pVal);
		STDMETHOD(SetValue)(BSTR key, BSTR val);
		STDMETHOD(GetValue)(BSTR key, BSTR* pVal);
		STDMETHOD(Translate)(BSTR eng, BSTR* pVal);
		STDMETHOD(ChangeAccount)(BSTR email, BSTR password);
		STDMETHOD(ChangeLicense)(BSTR name, BSTR organization, BSTR serial);
		STDMETHOD(OpenFile)(BSTR path);
		STDMETHOD(RunCommand)(BSTR commandCode);
		STDMETHOD(get_status)(BSTR* pVal);
		STDMETHOD(put_status)(BSTR val);
		STDMETHOD(get_TagLib)(VARIANT_BOOL* res);
		STDMETHOD(TagLibCheck)(BSTR fileID, VARIANT_BOOL* res);
		STDMETHOD(TagLibAdd)(BSTR fileID, BSTR tags, VARIANT_BOOL* res);
		STDMETHOD(get_BatchOp)(VARIANT_BOOL* res);
		STDMETHOD(BatchOpCheck)(BSTR opName, VARIANT_BOOL* res);
		STDMETHOD(BatchOpAdd)(BSTR fileName, VARIANT_BOOL* res);
		STDMETHOD(LayoutAdd)(BSTR fileName, VARIANT_BOOL* res);
		STDMETHOD(get_autoUpdates)(LONG* pVal);
		STDMETHOD(put_autoUpdates)(LONG val);
		STDMETHOD(UpdateNow)();
		STDMETHOD(get_lastUpdateCheck)(LONG* pVal);
		STDMETHOD(ReloadPage)();

	private:
		LCID m_tLocaleID;
		CStartPageOnline* m_pOwner;
		CComPtr<IConfigInMemory> m_pContext;
		CComPtr<IConfig> m_pMainConfig;
	};

private:
	CComPtr<IStartViewCallback> m_pClbk;
	CComBSTR m_bstrStatusText;
	CComBSTR m_bstrStatusTextOverride;
	CComPtr<IWebBrowser2> m_pWebBrowser;
	CComPtr<IOleInPlaceActiveObject> m_pWBObject;
	CComPtr<IUnknown> m_pUnkContainer;
	CComPtr<IDispatch> m_pExternal;
	CComBSTR m_bstrURL;
	CComBSTR m_bstrPost;
	bool m_bOverwrite;
};


class CStartViewPageFactoryOnline : public CStartViewPageFactory<CStartPageOnline, &CLSID_StartPageOnline, IDS_STARTPAGEONLINE_NAME, IDS_STARTPAGEONLINE_DESC, 0, &CStartPageOnline::InitConfig>, public IAnimatedIcon
{
public:
	DECLARE_CLASSFACTORY_SINGLETON(CStartViewPageFactoryOnline)

BEGIN_COM_MAP(CStartViewPageFactoryOnline)
	COM_INTERFACE_ENTRY(IStartViewPageFactory)
	COM_INTERFACE_ENTRY(IAnimatedIcon)
END_COM_MAP()

	// IAnimatedIcon methods
public:
	enum
	{
		DURATION = 128,
		PHASES = 16,
	};
	STDMETHOD_(ULONG, Phase)(ULONG a_nMSElapsed, ULONG* a_pNextPhase)
	{
		ULONG phase = a_nMSElapsed/DURATION;
		if (a_pNextPhase)
		{
			ULONG delta = a_nMSElapsed-phase*DURATION;
			*a_pNextPhase = DURATION-delta+(DURATION/4); // extra (DURATION/4) is a margin to fall within the interval more reliably
		}
		return phase%PHASES;
	}
	STDMETHOD_(HICON, Icon)(ULONG a_nPhase, ULONG a_nSize)
	{
		try
		{
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPolyPoint const mid[] =
			{
				{112, 208}, {144, 208}, {152, 245}, {104, 245},
			};
			static IRPathPoint const bot[] =
			{
				{64, 256, 0, -16, 0, 0},
				{128, 232, 64, 0, -64, 0},
				{192, 256, 0, 0, 0, -16},
			};
			static IRPathPoint const half[] =
			{
				{88, 23, -47.0508, 21.9401, 0, 0},
				{43, 148, 21.9401, 47.0508, -21.9401, -47.0508},
				{168, 193, 0, 0, -47.0508, 21.9401},
				{175, 210, -56.0605, 26.1414, 0, 0},
				{26, 155, -26.1414, -56.0605, 26.1414, 56.0605},
				{81, 6, 0, 0, -56.0605, 26.1414},
			};
			static IRPathPoint const axis[] =
			{
				{73.2942, 14.3449, -2.33405, -5.0054, 0, 0},
				{78.1311, 1.05567, 5.0054, -2.33405, -5.0054, 2.33405},
				{91.4203, 5.89256, 0, 0, -2.33405, -5.0054},
				{182.706, 201.655, 2.33405, 5.0054, 0, 0},
				{177.869, 214.944, -5.0054, 2.33405, 5.0054, -2.33405},
				{164.58, 210.107, 0, 0, 2.33405, 5.0054},
			};
			static IRPathPoint const globe[] =
			{
				{212, 108, 0, -46.3919, 0, 46.3919},
				{128, 24, -46.3919, 0, 46.3919, 0},
				{44, 108, 0, 46.3919, 0, -46.3919},
				{128, 192, 46.3919, 0, -46.3919, 0},
			};
			IRFill matStandFill(0xffa38863);
			IRFill matGlassFill(0xffd5ebf1);
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			IROutlinedFill matStand(&matStandFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IROutlinedFill matGlass(&matGlassFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			//IRGridItem gridX[] = {{0, 28}, {0, 228}};
			//IRGridItem gridY[] = {{0, 0}, {0, 32}, {0, 224}, {0, 256}};
			//IRCanvas canvasStand = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
			//IRTarget target(0.92f);

			cRenderer(&canvas, itemsof(mid), mid, &matStand);
			cRenderer(&canvas, itemsof(bot), bot, &matStand);
			cRenderer(&canvas, itemsof(half), half, &matStand);
			cRenderer(&canvas, itemsof(axis), axis, &matStand);
			cRenderer(&canvas, itemsof(globe), globe, &matGlass);

			IRStroke matLine(/*0xff8ba3a9*/pSI->GetSRGBColor(ESMContrast), 1.0/40.0f);
			int const pts = 10;
			IRPolyPoint line[10];
			float centerX = 128;
			float centerY = 108;
			float radius = 84*0.96f;

			matLine.color = (matLine.color&0xffffff)|0xbf000000;
			GetLinePoints(10, line, centerX, centerY, radius, 25, true);
			cRenderer(&canvas, 10, line, &matLine);
			GetLinePoints(10, line, centerX, centerY, radius, 0, true);
			cRenderer(&canvas, 10, line, &matLine);
			GetLinePoints(10, line, centerX, centerY, radius, -25, true);
			cRenderer(&canvas, 10, line, &matLine);

			float angle = a_nPhase > PHASES/2 ? (a_nPhase*25.0f)/PHASES-25.0f : (a_nPhase*25.0f)/PHASES;
			if (angle+50 < 53)
			{
				matLine.color = (matLine.color&0xffffff)|(angle+50 < 45 ? 0xbf000000 : DWORD(0xbf-(angle+50-45)*0x17)<<24);
				GetLinePoints(10, line, centerX, centerY, radius, angle+50, false);
				cRenderer(&canvas, 10, line, &matLine);
			}
			matLine.color = (matLine.color&0xffffff)|(angle+25 < 45 ? 0xbf000000 : DWORD(0xbf-(angle+25-45)*0x17)<<24);
			GetLinePoints(10, line, centerX, centerY, radius, angle+25, false);
			cRenderer(&canvas, 10, line, &matLine);
			matLine.color = (matLine.color&0xffffff)|0xbf000000;
			GetLinePoints(10, line, centerX, centerY, radius, angle, false);
			cRenderer(&canvas, 10, line, &matLine);
			matLine.color = (matLine.color&0xffffff)|(angle-25 > -45 ? 0xbf000000 : DWORD(0xbf-(-45-angle+25)*0x17)<<24);
			GetLinePoints(10, line, centerX, centerY, radius, angle-25, false);
			cRenderer(&canvas, 10, line, &matLine);
			if (angle-50 > -53)
			{
				matLine.color = (matLine.color&0xffffff)|(angle-50 > -45 ? 0xbf000000 : DWORD(0xbf-(-45-angle+50)*0x17)<<24);
				GetLinePoints(10, line, centerX, centerY, radius, angle-50, false);
				cRenderer(&canvas, 10, line, &matLine);
			}

			return cRenderer.get();
		}
		catch (...)
		{
			return NULL;
		}
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		*a_phIcon = Icon(0, a_nSize);
		return S_OK;
	}
	void GetLinePoints(int pts, IRPolyPoint* line, float centerX, float centerY, float radius, float angle, bool latitude)
	{
		float sa = sinf(-0.436332f);
		float ca = cosf(-0.436332f);
		for (int i = 0; i < pts; ++i)
		{
			float lat = latitude ? angle*3.14159265359f/180.0f : (i-pts*0.5f+0.5f)*0.83f/(pts*0.5f-0.5f)*1.5707963f;
			float lon = latitude ? (i-pts*0.5f+0.5f)*0.7f/(pts*0.5f-0.5f)*1.5707963f : angle*3.14159265359f/180.0f;
			float y = sinf(lat)*radius;
			float x = cosf(lat)*sinf(lon)*radius;
			float z = cosf(lat)*cosf(lon);
			x *= 4/(4-z);
			y *= 4/(4-z);
			line[i].x = centerX + ca*x - sa*y;
			line[i].y = centerY + ca*y + sa*x;
		}
	}
};
OBJECT_ENTRY_AUTO(__uuidof(StartPageOnline), CStartViewPageFactoryOnline)
