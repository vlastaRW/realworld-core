// DesignerViewFactoryDockingWindows.cpp : Implementation of CDesignerViewFactoryDockingWindows

#include "stdafx.h"
#include "DesignerViewFactoryDockingWindows.h"

#include "ConfigIDsDockingWindows.h"
#include "DesignerViewDockingWindows.h"
//#include "ConfigGUIDockingWindows.h"
#include <SharedStringTable.h>


// CDesignerViewFactoryDockingWindows

STDMETHODIMP CDesignerViewFactoryDockingWindows::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_DOCKWINS_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDockingWindows::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		// create config for one docking window
		CComPtr<IConfigWithDependencies> pWindowPattern;
		RWCoCreateInstance(pWindowPattern, __uuidof(ConfigWithDependencies));

		a_pManager->InsertIntoConfigAs(a_pManager, pWindowPattern, CComBSTR(CFGID_DOCK_WINDOWVIEW), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_WINDOWVIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_WINDOWVIEW_DESC), 0, NULL);
		pWindowPattern->ItemInsSimple(CComBSTR(CFGID_DOCK_DOCKED), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_DOCKED_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_DOCKED_DESC), CConfigValue(false), NULL, 0, NULL);
		pWindowPattern->ItemInsSimple(CComBSTR(CFGID_DOCK_FLOATPOSX), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATPOSX_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATPOSX_DESC), CConfigValue(50L), NULL, 0, NULL);
		pWindowPattern->ItemInsSimple(CComBSTR(CFGID_DOCK_FLOATPOSY), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATPOSY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATPOSY_DESC), CConfigValue(50L), NULL, 0, NULL);
		pWindowPattern->ItemInsSimple(CComBSTR(CFGID_DOCK_FLOATSIZEX), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATSIZEX_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATSIZEX_DESC), CConfigValue(150L), NULL, 0, NULL);
		pWindowPattern->ItemInsSimple(CComBSTR(CFGID_DOCK_FLOATSIZEY), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATSIZEY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_FLOATSIZEY_DESC), CConfigValue(250L), NULL, 0, NULL);
		CComBSTR cCFGID_DOCK_DOCKSIDE(CFGID_DOCK_DOCKSIDE);
		pWindowPattern->ItemIns1ofN(cCFGID_DOCK_DOCKSIDE, _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_DOCKSIDE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_DOCKSIDE_DESC), CConfigValue(CFGVAL_DOCKSIDE_LEFT), NULL);
		pWindowPattern->ItemOptionAdd(cCFGID_DOCK_DOCKSIDE, CConfigValue(CFGVAL_DOCKSIDE_LEFT), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DOCKSIDE_LEFT), 0, NULL);
		pWindowPattern->ItemOptionAdd(cCFGID_DOCK_DOCKSIDE, CConfigValue(CFGVAL_DOCKSIDE_TOP), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DOCKSIDE_TOP), 0, NULL);
		pWindowPattern->ItemOptionAdd(cCFGID_DOCK_DOCKSIDE, CConfigValue(CFGVAL_DOCKSIDE_RIGHT), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DOCKSIDE_RIGHT), 0, NULL);
		pWindowPattern->ItemOptionAdd(cCFGID_DOCK_DOCKSIDE, CConfigValue(CFGVAL_DOCKSIDE_BOTTOM), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DOCKSIDE_BOTTOM), 0, NULL);

		if (FAILED(pWindowPattern->Finalize(NULL)))
			return E_UNEXPECTED; // TODO: error code

		// insert pattern to the vector
		CComPtr<ISubConfigVector> pWindowVector;
		RWCoCreateInstance(pWindowVector, __uuidof(SubConfigVector));
		if (FAILED(pWindowVector->Init(TRUE, pWindowPattern)))
			return E_UNEXPECTED; // TODO: error code

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_DOCK_SUBVIEW), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_SUBVIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_SUBVIEW_DESC), 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_DOCK_WINDOWS), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_WINDOWS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DOCK_WINDOWS_DESC), CConfigValue(1L), pWindowVector, CConfigValue(0L), CConfigValue(16L), CConfigValue(1L), 0, NULL);

		//CComObject<CConfigGUISplitter>* pGUI = NULL;
		//CComObject<CConfigGUISplitter>::CreateInstance(&pGUI);
		CComPtr<IConfigCustomGUI> pTmp = NULL;//pGUI;

		if (FAILED(pCfgInit->Finalize(pTmp)))
			return E_FAIL;

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDockingWindows::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewDockingWindows>* pWnd = NULL;
		CComObject<CDesignerViewDockingWindows>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		pWnd->Init(a_pFrame, a_pStatusBar, a_pManager, a_pConfig, a_hParent, a_prcWindow, a_pDoc, a_nStyle, a_tLocaleID);

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDockingWindows::CheckSuitability(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	try
	{
		{
			CConfigValue cViewType;
			CComPtr<IConfig> pSubCfg;
			CComBSTR bstrID(CFGID_DOCK_SUBVIEW);
			a_pConfig->ItemValueGet(bstrID, &cViewType);
			a_pConfig->SubConfigGet(bstrID, &pSubCfg);
			a_pManager->CheckSuitability(a_pManager, cViewType, pSubCfg, a_pDocument, a_pCallback);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDockingWindows::NameGet(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_DOCKWINS_COMMANDS);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDockingWindows::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	return E_NOTIMPL;
}

class ATL_NO_VTABLE CMenuItemDockingWindow :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand,
	public ILocalizedString
{
public:
	void Init(IDesignerViewDockingWindows* a_pDVDW, ULONG a_nIndex)
	{
		m_pDVDW = a_pDVDW;
		m_nIndex = a_nIndex;
	}

BEGIN_COM_MAP(CMenuItemDockingWindow)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	COM_INTERFACE_ENTRY(ILocalizedString)
END_COM_MAP()

	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		return m_pDVDW->NameGet(m_nIndex, a_ppText);
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			(*a_ppText = this)->AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		return m_pDVDW->IconIDGet(m_nIndex, a_pIconID);
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		return m_pDVDW->IconGet(m_nIndex, a_nSize, a_phIcon);
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel) { return E_NOTIMPL; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands) { return E_NOTIMPL; }
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = m_pDVDW->IsVisible(m_nIndex) == S_OK ? EMCSChecked : EMCSNormal;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pDVDW->SetVisible(m_nIndex, m_pDVDW->IsVisible(m_nIndex) != S_OK);
	}

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR *a_pbstrString) { return GetLocalized(GetThreadLocale(), a_pbstrString); }
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR *a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;
			CComPtr<ILocalizedString> pName;
			m_pDVDW->NameGet(m_nIndex, &pName);
			CComBSTR bstrName;
			pName->GetLocalized(a_tLCID, &bstrName);
			OLECHAR szText[256] = L"";
			OLECHAR szTempl[128] = L"";
			Win32LangEx::LoadStringW(_pModule->get_m_hInst(), m_pDVDW->IsVisible(m_nIndex) == S_OK ? IDS_DOCKWINS_HIDE : IDS_DOCKWINS_SHOW, szTempl, itemsof(szTempl), LANGIDFROMLCID(a_tLCID));
			_swprintf(szText, szTempl, bstrName);
			CComBSTR bstrText(szText);
			*a_pbstrString = bstrText.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	CComPtr<IDesignerViewDockingWindows> m_pDVDW;
	ULONG m_nIndex;
};

STDMETHODIMP CDesignerViewFactoryDockingWindows::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pViews;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IDesignerViewDockingWindows), EQIFVisible, pViews);
		CComPtr<IDesignerViewDockingWindows> pView;
		pViews->Get(0, __uuidof(IDesignerViewDockingWindows), reinterpret_cast<void**>(&pView));
		if (pView == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		ULONG nItems = 0;
		pView->ItemCount(&nItems);
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComObject<CMenuItemDockingWindow>* p = NULL;
			CComObject<CMenuItemDockingWindow>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(pView, i);
			pItems->Insert(pTmp);
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

