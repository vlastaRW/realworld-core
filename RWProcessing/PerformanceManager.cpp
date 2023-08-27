
#include "stdafx.h"
#include <RWBase.h>
#include <RWConfig.h>
#include <RWConceptDesignerExtension.h>
#include <WeakSingleton.h>
#include <MultiLanguageString.h>
#include <SimpleLocalizedString.h>
#include <ConfigCustomGUIImpl.h>
#include <ObserverImpl.h>
#include "RWProcessing.h"
#include <IconRenderer.h>


class ATL_NO_VTABLE CConfigGUIPerformanceDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIPerformanceDlg>,
	public CDialogResize<CConfigGUIPerformanceDlg>
{
public:
	enum
	{
		IDC_MAXCORES = 100,
		IDC_MAXCORES_SPIN,
		IDC_MAXCORES_LABEL,
		IDC_COREINFO,
		IDC_PERFMON,
	};

	BEGIN_DIALOG_EX(0, 0, 219, 38, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Maximum cores:[0405]Maximum jader:"), IDC_MAXCORES_LABEL, 0, 2, 77, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_MAXCORES, 80, 0, 139, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_MAXCORES_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 209, 0, 10, 12, 0)
		CONTROL_LTEXT(_T("[0409]CPU info: %i cores, %i execution threads.[0405]CPU info: %i jader, %i výkonných vláken."), IDC_COREINFO, 0, 16, 219, 8, WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]Monitor performance[0405]Sledovat výkon"), IDC_PERFMON, 0, 28, 219, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIPerformanceDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIPerformanceDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIPerformanceDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIFormatsDlg)
		DLGRESIZE_CONTROL(IDC_MAXCORES, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_MAXCORES_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_COREINFO, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_PERFMON, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIFormatsDlg)
		CONFIGITEM_EDITBOX(IDC_MAXCORES, L"MaxCores")
		CONFIGITEM_CHECKBOX(IDC_PERFMON, L"PerfMon")
		//CONFIGITEM_CONTEXTHELP_IDS(IDC_APPOPT_UNDOMODE_GRP, IDS_CFGID_UNDOMODE_DESC)
		//CONFIGITEM_RADIO(IDC_APPOPT_UNDODEFAULT, CFGID_UNDOMODE, EUMDefault)
	END_CONFIGITEM_MAP()


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		CComPtr<IThreadPoolControl> pThPool;
		RWCoCreateInstance(pThPool, __uuidof(ThreadPool));
		if (pThPool)
		{
			ULONG nCores = 0;
			ULONG nThreads = 0;
			pThPool->ProcessorInfoGet(&nCores, &nThreads);
			CUpDownCtrl wnd(GetDlgItem(IDC_MAXCORES_SPIN));
			wnd.SetRange(0, nCores);
			wchar_t szTempl[256];
			GetDlgItemText(IDC_COREINFO, szTempl, 256);
			szTempl[255] = L'\0';
			wchar_t sz[272];
			swprintf(sz, szTempl, nCores, nThreads);
			SetDlgItemText(IDC_COREINFO, sz);
		}

		return 1;
	}
};

// {BCD365B1-63E7-43CC-BAF9-9AFA920B3AC6}
//extern GUID const CLSID_PerformanceManager = {0xbcd365b1, 0x63e7, 0x43cc, {0xba, 0xf9, 0x9a, 0xfa, 0x92, 0xb, 0x3a, 0xc6}};

class ATL_NO_VTABLE CPerformanceManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CPerformanceManager>,
	public IGlobalConfigFactory,
	public IPerformanceMonitor,
	public IStatusBarPane,
	public CObserverImpl<CPerformanceManager, IConfigObserver, IUnknown*>
{
public:
	CPerformanceManager() : m_bEnabled(true), m_nCurrent(0xffffffff), m_nTimeStamp(1)
	{
	}
	~CPerformanceManager()
	{
		for (CCounters::iterator i = m_cCounters.begin(); i != m_cCounters.end(); ++i)
		{
			if (i->bstrID) SysFreeString(i->bstrID);
			if (i->pName) i->pName->Release();
		}
		if (m_pConfig)
			m_pConfig->ObserverDel(ObserverGet(), 0);
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CPerformanceManager)

BEGIN_CATEGORY_MAP(CPerformanceManager)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
	IMPLEMENTED_CATEGORY(CATID_StatusBarPane)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CPerformanceManager)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
	COM_INTERFACE_ENTRY(IStatusBarPane)
	COM_INTERFACE_ENTRY(IPerformanceMonitor)
END_COM_MAP()


public:
	void OwnerNotify(TCookie, IUnknown*)
	{
		CConfigValue cVal;
		m_pConfig->ItemValueGet(CComBSTR(L"MaxCores"), &cVal);
		m_pThPool->MaxThreadsSet(cVal.operator LONG());
		m_pConfig->ItemValueGet(CComBSTR(L"CounterID"), &cVal);
		ObjectLock lock(this);
		m_bstrCurrentID.Attach(cVal.Detach().bstrVal);
		m_pConfig->ItemValueGet(CComBSTR(L"PerfMon"), &cVal);
		bool bEnabled = cVal;
		if (m_bEnabled != bEnabled)
		{
			m_bEnabled = bEnabled;
			++m_nTimeStamp;
		}
		for (CCounters::const_iterator i = m_cCounters.begin(); i != m_cCounters.end(); ++i)
		{
			if (m_bstrCurrentID == i->bstrID)
			{
				if (m_nCurrent != i-m_cCounters.begin())
				{
					m_nCurrent = i-m_cCounters.begin();
					++m_nTimeStamp;
				}
				break;
			}
		}
	}

	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE* a_pPriority) { if (a_pPriority) *a_pPriority = 85; return S_OK; }
	STDMETHOD(IGlobalConfigFactory::Name)(ILocalizedString** a_ppName)
	{
		ATLASSERT(a_ppName != NULL);
		try
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Performance[0405]Výkon");
			return S_OK;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppDesc)
	{
		ATLASSERT(a_ppDesc != NULL);
		try
		{
			*a_ppDesc = new CMultiLanguageString(L"[0409]Set maximum number of processor cores to use and monitor application performance.[0405]Nastavte maximální počet používaných procesorových jader a monitorujte výkon aplikace.");
			return S_OK;
		}
		catch (...)
		{
			return a_ppDesc ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Config)(IConfig** a_ppConfig)
	{
		ATLASSERT(a_ppConfig != NULL);
		try
		{
			InitConfig();
			(*a_ppConfig = m_pConfig)->AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppConfig ? E_UNEXPECTED : E_POINTER;
		}
	}

	// IPerformanceMonitor methods
public:
	STDMETHOD(IsEnabled)()
	{
		return m_bEnabled ? S_OK : S_FALSE;
	}
	STDMETHOD(RegisterCounter)(BSTR a_bstrCounterID, ILocalizedString* a_pCounterName, ULONG* a_pCounterID)
	{
		if (a_bstrCounterID == NULL) return E_RW_INVALIDPARAM;
		ObjectLock lock(this);
		for (CCounters::const_iterator i = m_cCounters.begin(); i != m_cCounters.end(); ++i)
		{
			if (wcscmp(a_bstrCounterID, i->bstrID) == 0)
			{
				*a_pCounterID = i-m_cCounters.begin();
				return S_FALSE;
			}
		}
		m_cCounters.reserve(m_cCounters.size()+1);
		*a_pCounterID = m_cCounters.size();
		SCounter s;
		s.bstrID = SysAllocString(a_bstrCounterID);
		s.pName = a_pCounterName;
		if (s.pName) s.pName->AddRef();
		s.nPointer = 0;
		s.nWritten = 0;
		m_cCounters.push_back(s);
		if (m_bstrCurrentID == a_bstrCounterID || m_bstrCurrentID.Length() == 0)
		{
			m_nCurrent = *a_pCounterID;
			++m_nTimeStamp;
		}
		return S_OK;
	}
	STDMETHOD(AddMeasurement)(ULONG a_nCounterID, float a_fValue)
	{
		ObjectLock lock(this);
		if (a_nCounterID >= m_cCounters.size())
			return E_RW_INDEXOUTOFRANGE;
		SCounter& s = m_cCounters[a_nCounterID];
		s.aValues[s.nPointer] = a_fValue;
		s.nPointer = (s.nPointer+1)%EHistoryLength;
		++s.nWritten;
		if (a_nCounterID == m_nCurrent)
			++m_nTimeStamp;
		return S_OK;
	}

	// IToolStatusBarControl methods
public:
	STDMETHOD(IsVisible)(IDocument* a_pCurrentDoc, ULONG* a_pnTimeStamp)
	{
		if (m_bEnabled)
		{
			if (a_pnTimeStamp)
				*a_pnTimeStamp = m_nTimeStamp;
			return S_OK;
		}
		if (a_pnTimeStamp)
			*a_pnTimeStamp = 0;
		return E_FAIL;
	}

	STDMETHOD(Text)(ILocalizedString** a_ppText)
	{
		ObjectLock lock(this);
		if (!m_bEnabled)
			return E_RW_INDEXOUTOFRANGE;
		if (m_nCurrent >= m_cCounters.size())
		{
			*a_ppText = new CSimpleLocalizedString(SysAllocString(L"<no data>"));
			return S_OK;
		}
		SCounter const& s = m_cCounters[m_nCurrent];
		float fLast = 0.0f;
		float fAverage = 0.0f;
		if (s.nWritten > 0)
		{
			fLast = s.aValues[(s.nPointer+EHistoryLength-1)%EHistoryLength];
			int nValid = min(EHistoryLength, s.nWritten);
			for (int i = 0; i < nValid; ++i)
				fAverage += s.aValues[i];
			fAverage /= nValid;
		}
		wchar_t sz[256];
		swprintf(sz, L"#%i %gms (%gms)", s.nWritten%100, int(fLast*10.0f+0.5f)/10.0f, int(fAverage*10.0f+0.5f)/10.0f);
		*a_ppText = new CSimpleLocalizedString(SysAllocString(sz));
		return S_OK;
	}
	STDMETHOD(Tooltip)(ILocalizedString** a_ppText)
	{
		ObjectLock lock(this);
		if (!m_bEnabled)
			return E_RW_INDEXOUTOFRANGE;
		if (m_nCurrent >= m_cCounters.size())
			return E_NOTIMPL;
		SCounter const& s = m_cCounters[m_nCurrent];
		*a_ppText = s.pName;
		if (s.pName) s.pName->AddRef();
		return S_OK;
	}
	STDMETHOD(MaxWidth)(ULONG* a_pnPixels)
	{
		if (!m_bEnabled)
			return E_RW_INDEXOUTOFRANGE;
		*a_pnPixels = 200;
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;

		IRPathPoint const scale0[] =
		{
			{169, 62, 0, 0, -11.984, -4.45876},
			{126, 101, -7.03387, -2.39162, 0, 0},
			{104, 97, -35.2647, 0, 7.8644, 0},
			{35, 157, 0, 0, 4.85236, -33.9229},
			{21, 157, 4.09954, -57.0151, 0, 0},
			{131, 55, 13.5141, 0, -58.0599, 0},
		};
		IRPathPoint const scale1[] =
		{
			{241, 157, 0, 0, -2.71326, -37.7353},
			{173, 157, -1.68018, -11.7462, 0, 0},
			{160, 125, 0, 0, 6.70985, 9.10793},
			{185, 69, 31.3155, 17.6417, 0, 0},
		};
		IRPath const scale[] = { {itemsof(scale0), scale0}, {itemsof(scale1), scale1} };
		IRPathPoint const needle[] =
		{
			{133, 153, 4.86826, 10.2413, 0, 0},
			{130, 186, -10.388, 14.2978, 7.13431, -9.81953},
			{85, 193, -14.2978, -10.388, 14.2978, 10.388},
			{78, 148, 7.13431, -9.81953, -10.388, 14.2978},
			{108, 136, 0, 0, -11.2444, -1.46526},
			{175, 70, 0, 0, 0, 0},
		};
		IRGridItem grid = {0, 157};
		IRCanvas const canvas = {21, 21, 241, 241, 0, 1, NULL, &grid};
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(scale), scale, pSI->GetMaterial(ESMInterior));
		cRenderer(&canvas, itemsof(needle), needle, pSI->GetMaterial(ESMContrast));
		*a_phIcon = cRenderer.get();
		return (*a_phIcon) ? S_OK : E_FAIL;


		//static float const f = 1.0f/256.0f;
		////static TPolyCoords const aVertices[] = {{f*24, f*232}, {f*128, f*24}, {f*232, f*232}};
		////*a_phIcon = IconFromPolygon(itemsof(aVertices), aVertices, a_nSize, false);
		//static TPolyCoords const aVertices1a[] =
		//{
		//	{f*169, f*62},
		//	{0, 0}, {0, 0}, {f*126, f*101},
		//	{-f*7.03387, -f*2.39162}, {f*7.8644, 0}, {f*104, f*97},
		//	{-f*35.2647, 0}, {f*4.85236, -f*33.9229}, {f*35, f*157},
		//	{0, 0}, {0, 0}, {f*21, f*157},
		//	{f*4.09954, -f*57.0151}, {-f*58.0599, 0}, {f*131, f*55},
		//	{f*13.5141, 0}, {-f*11.984, -f*4.45876}, {f*169, f*62},
		//};
		//static TPolyCoords const aVertices1b[] =
		//{
		//	{f*241, f*157},
		//	{0, 0}, {0, 0}, {f*173, f*157},
		//	{-f*1.68018, -f*11.7462}, {f*6.70985, f*9.10793}, {f*160, f*125},
		//	{0, 0}, {0, 0}, {f*185, f*69},
		//	{f*31.3155, f*17.6417}, {-f*2.71326, -f*37.7353}, {f*241, f*157},
		//};
		//static TPolyCoords const aVertices2[] =
		//{
		//	{f*133, f*153},
		//	{f*4.86826, f*10.2413}, {f*7.13431, -f*9.81953}, {f*130, f*186},
		//	{-f*10.388, f*14.2978}, {f*14.2978, f*10.388}, {f*85, f*193},
		//	{-f*14.2978, -f*10.388}, {-f*10.388, f*14.2978}, {f*78, f*148},
		//	{f*7.13431, -f*9.81953}, {-f*11.2444, -f*1.46526}, {f*108, f*136},
		//	{0, 0}, {0, 0}, {f*175, f*70},
		//	{0, 0}, {0, 0}, {f*133, f*153},

		//};
		//TIconPolySpec tPolySpec[3];
		//tPolySpec[0].nVertices = itemsof(aVertices1a);
		//tPolySpec[0].pVertices = aVertices1a;
		//tPolySpec[0].interior = GetIconFillColor();
		//tPolySpec[0].outline = agg::rgba8(0, 0, 0, 255);
		//tPolySpec[1].nVertices = itemsof(aVertices1b);
		//tPolySpec[1].pVertices = aVertices1b;
		//tPolySpec[1].interior = GetIconFillColor();
		//tPolySpec[1].outline = agg::rgba8(0, 0, 0, 255);
		//tPolySpec[2].nVertices = itemsof(aVertices2);
		//tPolySpec[2].pVertices = aVertices2;
		//tPolySpec[2].interior = agg::rgba8(0, 0, 0, 255);//GetIconFillColor();
		//tPolySpec[2].outline = agg::rgba8(0, 0, 0, 0);
		//*a_phIcon = IconFromPath(itemsof(tPolySpec), tPolySpec, a_nSize, false);
		//return S_OK;
	}

	STDMETHOD(OnClick)(RWHWND a_hFrameWnd, LCID a_tLocaleID, RECT const* a_pPaneRect, POINT a_ptClick, EStatusBarClickType a_eClickType)
	{
		if (a_eClickType == ESBCTRightSingle)
		{
			CMenu cMenu;
			//StartContextMenu(cMenu);
			//if (m_cMenuImages.m_hImageList == NULL)
			//{
			//	int nIconSize = XPGUI::GetSmallIconSize();
			//	m_cMenuImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 1, 0);
			//}
			//Reset(m_cMenuImages);
			cMenu.CreatePopupMenu();

			//RECT rc;
			//if (!m_bClickedStop || !GetStopRect(m_nClickedStop, &rc))
			//{
			//	GetClientRect(&rc);
			//}
			//else
			//{
			//	AddStopCommands(cMenu, m_nClickedStop);
			//}
			//AddGlobalCommands(cMenu);

			ObjectLock lock(this);
			for (CCounters::const_iterator i = m_cCounters.begin(); i != m_cCounters.end(); ++i)
			{
				CComBSTR bstr;
				if (i->pName)
					i->pName->GetLocalized(a_tLocaleID, &bstr);
				cMenu.AppendMenu((i-m_cCounters.begin()) != m_nCurrent ? MF_STRING : MF_STRING|MF_CHECKED, 1+(i-m_cCounters.begin()), bstr.Length() ? bstr.m_str : i->bstrID);
			}

			RECT rcWin;
			GetWindowRect(a_hFrameWnd, &rcWin);
			//ClientToScreen(&rc);
			TPMPARAMS tTPMP = { sizeof(TPMPARAMS), *a_pPaneRect };
			//cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, m_tClickedPoint.x, m_tClickedPoint.y, m_hWnd, m_bClickedStop ? &tTPMP : NULL);
			ULONG res = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, a_ptClick.x, a_ptClick.y, a_hFrameWnd, &tTPMP);
			if (res != 0 && res <= m_cCounters.size())
			{
				m_nCurrent = res-1;
				CComBSTR bstr(L"CounterID");
				m_pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(m_cCounters[m_nCurrent].bstrID));
			}
			return S_OK;
		}
		return E_NOTIMPL;
	}

private:
	void InitConfig()
	{
		if (m_pConfig)
			return;
		ObjectLock cLock(this);
		if (m_pConfig == NULL)
		{
			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));
			pCfgInit->ItemInsSimple(CComBSTR(L"MaxCores"), CMultiLanguageString::GetAuto(L"[0409]Maximum threads[0405]Maximum vláken"), CMultiLanguageString::GetAuto(L"[0409]Set to 0 use all available cores.[0405]Nastavete na 0 pro použití všech dostupných jader."), CConfigValue(0L), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(L"PerfMon"), CMultiLanguageString::GetAuto(L"[0409]Monitor performance[0405]Sledovat výkon"), CMultiLanguageString::GetAuto(L"[0409]Gather information about application performance.[0405]Shromažďovat informace o výkonu aplikace."), CConfigValue(
#ifdef DEBUG
				true
#else
				false
#endif
				), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(L"CounterID"), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);
			//pCfgInit->Finalize(NULL);
			CConfigCustomGUI<&CLSID_PerformanceManager, CConfigGUIPerformanceDlg>::FinalizeConfig(pCfgInit);
			m_pConfig.Attach(pCfgInit.Detach());
			m_pConfig->ObserverIns(ObserverGet(), 0);

			RWCoCreateInstance(m_pThPool, __uuidof(ThreadPool));
		}
	}

private:
	enum { EHistoryLength = 10 };
	struct SCounter
	{
		BSTR bstrID;
		ILocalizedString* pName;
		float aValues[EHistoryLength];
		ULONG nWritten;
		BYTE nPointer;
	};
	typedef std::vector<SCounter> CCounters;

private:
	CComPtr<IConfig> m_pConfig;
	CComPtr<IThreadPoolControl> m_pThPool;

	bool m_bEnabled;
	CComBSTR m_bstrCurrentID;
	ULONG m_nCurrent;
	ULONG m_nTimeStamp;
	CCounters m_cCounters;
};

OBJECT_ENTRY_AUTO(CLSID_PerformanceManager, CPerformanceManager)
