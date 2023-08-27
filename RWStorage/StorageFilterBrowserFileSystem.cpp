// StorageFilterWindowFileSystem.cpp : implementation of the CStorageFilterBrowserFileSystem class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StorageFilterBrowserFileSystem.h"

#include <XPGUI.h>
#include "ConfigIDsFileSystem.h"
#include "DocumentTypeWildcards.h"
#include "DocumentTypeComposed.h"
#include <PortablePath.h>
#include <IconRenderer.h>


LPWSTR MyStrRetToWStr(IMalloc* a_pMalloc, const ITEMIDLIST* a_pIDL, STRRET a_strret);
LPSTR MyStrRetToAStr(IMalloc* a_pMalloc, const ITEMIDLIST* a_pIDL, STRRET a_strret);

#ifdef _UNICODE
#define MyStrRetToStr MyStrRetToWStr
#else
#define MyStrRetToStr MyStrRetToAStr
#endif

#include "PathAnalyzer.h"


// CStorageFilterBrowserFileSystem

HRESULT CStorageFilterBrowserFileSystem::Init(LPCOLESTR a_pszInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, HWND a_hWnd, LCID a_tLocaleID)
{
	m_tLocaleID = a_tLocaleID;
	m_pCallback = a_pCallback;
	m_pListener = a_pListener;
	CComPtr<IDocumentType> pDefDocType;
	if (m_pListener)
		m_pListener->DefaultDocumentGet(&pDefDocType);
	if (a_dwFlags&EFTMultiselection)
		m_tViewSettings.fFlags &= ~FWF_SINGLESEL;

	USES_CONVERSION;
	static bool bCommonCompsInitialized = false;
	if (!bCommonCompsInitialized)
	{
		bCommonCompsInitialized = true;
		AtlInitCommonControls(ICC_BAR_CLASSES | ICC_USEREX_CLASSES);
	}

	m_pContextConfig = a_pContextConfig;

	m_nDefDocType = -1;
	if (a_pFormatFilters && pDefDocType)
	{
		CComPtr<IDocumentType> pDT;
		for (ULONG i = 0; SUCCEEDED(a_pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDT))); ++i)
		{
			if (pDT == pDefDocType)
			{
				m_nDefDocType = i;
				break;
			}
			pDT = NULL;
		}
	}

	//if (a_pFormatFilters)
	//{
	//	ULONG i;
	//	CComPtr<IDocumentType> pFilter;
	//	for (i = 0; SUCCEEDED(a_pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pFilter))); i++, pFilter=NULL)
	//	{
	//		CComBSTR bstrFilter;
	//		pFilter->FilterGet(&bstrFilter);
	//		CComPtr<ILocalizedString> pName;
	//		pFilter->FilterNameGet(&pName);
	//		CComBSTR bstrName;
	//		pName->GetLocalized(a_tLocaleID, &bstrName);
	//		pair<tstring, tstring> tTmp;
	//		tTmp.first = bstrName == NULL ? _T("") : OLE2CT(bstrName);
	//		tTmp.second = bstrFilter == NULL ? _T("") : OLE2CT(bstrFilter);
	//		tStart.aFilters.push_back(tTmp);
	//	}
	//}

	m_dwFlags = a_dwFlags;

	SHGetMalloc(&m_pShellAlloc);

//	m_pFormatFilters = a_pFormatFilters;
	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &m_cDesktopDir);
	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_cDesktop);

	if (a_pszInitial == NULL) a_pszInitial = L"";

	if (_wcsnicmp(a_pszInitial, L"file://", 7) == 0)
	{
		a_pszInitial += 7;
	}
	HRESULT hRes = a_pszInitial[0] == L'\0' ? S_OK : S_FALSE; // empty string defaults to MyDocuments folder

	// parse the input string and obtain initial file name and folder (as pidl)
	bool bBlockName = false;
	CPIDL cFolder;
	SHGetDesktopFolder(&m_pDesktopFolder);
	ULONG nEaten = 0;
	ULONG nAttribs = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER;
	if (hRes == S_FALSE && SUCCEEDED(m_pDesktopFolder->ParseDisplayName(NULL, NULL, const_cast<LPOLESTR>(a_pszInitial), &nEaten, &cFolder, &nAttribs)))
	{
		if ((nAttribs&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR) ||
			(nAttribs&(SFGAO_FILESYSANCESTOR|SFGAO_FOLDER)) == (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER) ||
			(nAttribs&(SFGAO_FILESYSTEM|SFGAO_FOLDER)) == (SFGAO_FILESYSTEM|SFGAO_FOLDER))
		{
			m_cFolder = cFolder;
			bBlockName = true;
		}
		else if ((nAttribs&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM))
		{
			m_cFolder = cFolder.GetParent();
		}
	}
	else
	{
		m_szName[0] = _T('\0');
	}
	if (m_cFolder == CPIDL())
	{
		CConfigValue cLastFolder;
		GetConfigValue(CComBSTR(CFGID_FS_LASTDIRECTORY), &cLastFolder);
		if (cLastFolder.TypeGet() == ECVTString && cLastFolder.operator BSTR() && cLastFolder.operator BSTR()[0])
		{
			CComBSTR bstrFolder;
			bstrFolder.Attach(cLastFolder.Detach().bstrVal);
			PortablePath::Portable2Full(&(bstrFolder.m_str));
			nAttribs = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER;
			nEaten = 0;
			if (FAILED(m_pDesktopFolder->ParseDisplayName(NULL, NULL, bstrFolder, &nEaten, &cFolder, &nAttribs)))
			{
				SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &m_cFolder);
			}
			else
			{
				if ((nAttribs&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR) ||
					(nAttribs&(SFGAO_FILESYSANCESTOR|SFGAO_FOLDER)) == (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER) ||
					(nAttribs&(SFGAO_FILESYSTEM|SFGAO_FOLDER)) == (SFGAO_FILESYSTEM|SFGAO_FOLDER))
				{
					m_cFolder = cFolder;
				}
				else
				{
					SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &m_cFolder);
				}
			}
		}
		else
		{
			SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &m_cFolder);
		}
	}

	int iLastNameStart = 0;
	int i;
	for (i = 0; a_pszInitial[i]; i++)
	{
		if (a_pszInitial[i] == L'\\' || a_pszInitial[i] == L'/')
			iLastNameStart = i+1;
	}
	if (iLastNameStart < 2)
	{
		hRes = S_OK; // no problems during parsing
	}

	if (!bBlockName)
		_tcscpy(m_szName, OLE2CT(a_pszInitial+iLastNameStart));
	else
		m_szName[0] = _T('\0');

	Create(a_hWnd, reinterpret_cast<LPARAM>(a_pFormatFilters));

	return hRes;
}

LRESULT CStorageFilterBrowserFileSystem::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	m_pCallback = NULL;
	m_pListener = NULL;

	m_bDestroying = true;
	a_bHandled = FALSE;

	LookInClear();

	if (m_pActiveShellView != NULL)
	{
		m_pActiveShellView->UIActivate(SVUIA_DEACTIVATE);
		m_pActiveShellView->DestroyViewWindow();
		m_pActiveShellView = NULL;
	}

	return 0;
}

LRESULT CStorageFilterBrowserFileSystem::OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	if (m_pActiveShellView)
	{
		m_pActiveShellView->UIActivate(SVUIA_ACTIVATE_FOCUS);
	}
	return 0;
}

LRESULT CStorageFilterBrowserFileSystem::OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardOK();

	return 0;
}

LRESULT CStorageFilterBrowserFileSystem::OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardCancel();

	return 0;
}


STDMETHODIMP CStorageFilterBrowserFileSystem::FilterCreate(IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;

		if (m_pActiveShellView == NULL)
			return E_FAIL;

		CComPtr<IStorageFilterFactory> pFct;
		RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));

		CComPtr<IDataObject> pDataObj;
		m_pActiveShellView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**)&pDataObj);
		if (pDataObj == NULL)
			return E_FAIL;

		FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg;
		stg.tymed = TYMED_HGLOBAL;

		if (SUCCEEDED(pDataObj->GetData(&fmt, &stg)))
		{
			HDROP hDrop = reinterpret_cast<HDROP>(GlobalLock(stg.hGlobal));

			UINT uNumFiles = DragQueryFile (hDrop, 0xFFFFFFFF, NULL, 0);
			HRESULT hr = S_OK;

			for(UINT i = 0; i < uNumFiles; i++)
			{
				TCHAR szPath[_MAX_PATH];
				szPath[0] = 0;
				DragQueryFile(hDrop, i, szPath, MAX_PATH);
				if (szPath[0] != 0)
				{
					GlobalUnlock(stg.hGlobal);
					ReleaseStgMedium(&stg);
					return pFct->FilterCreate(CComBSTR(szPath), EFTOpenExisting|EFTShareRead|EFTShareWrite|EFTHintNoStream, a_ppFilter);
				}
			}
		    
			GlobalUnlock(stg.hGlobal);
			ReleaseStgMedium(&stg);
		}
		return E_FAIL;
	}
	catch (...)
	{
		return a_ppFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::FiltersCreate(IEnumUnknowns** a_ppFilters)
{
	try
	{
		*a_ppFilters = NULL;

		if (m_pActiveShellView == NULL)
			return E_FAIL;

		CComPtr<IStorageFilterFactory> pFct;
		RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));

		CComPtr<IDataObject> pDataObj;
		m_pActiveShellView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**)&pDataObj);
		if (pDataObj == NULL)
			return E_FAIL;

		FORMATETC fmt = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stg;
		stg.tymed = TYMED_HGLOBAL;

		if (SUCCEEDED(pDataObj->GetData(&fmt, &stg)))
		{
			HDROP hDrop = reinterpret_cast<HDROP>(GlobalLock(stg.hGlobal));

			UINT uNumFiles = DragQueryFile (hDrop, 0xFFFFFFFF, NULL, 0);
			HRESULT hr = S_OK;

			for(UINT i = 0; i < uNumFiles; i++)
			{
				TCHAR szPath[_MAX_PATH];
				szPath[0] = 0;
				DragQueryFile(hDrop, i, szPath, MAX_PATH);
				if (szPath[0] != 0)
				{
					CComPtr<IStorageFilter> pFlt;
					pFct->FilterCreate(CComBSTR(szPath), EFTOpenExisting|EFTShareRead|EFTShareWrite|EFTHintNoStream, &pFlt);
					if (pFlt) p->Insert(pFlt);
				}
			}
		    
			GlobalUnlock(stg.hGlobal);
			ReleaseStgMedium(&stg);
		}
		*a_ppFilters = p.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilters ? E_UNEXPECTED : E_NOTIMPL;
	}
}

bool CStorageFilterBrowserFileSystem::MessagePumpPretranslateMessage(LPMSG a_pMsg)
{
	if (m_pActiveShellView == NULL)
		return false;
	HWND hMessageWnd = a_pMsg->hwnd;
	HWND hShellWnd = NULL;
	m_pActiveShellView->GetWindow(&hShellWnd);
	while (hMessageWnd != NULL)
	{
		if (hMessageWnd == hShellWnd)
			return S_OK == m_pActiveShellView->TranslateAccelerator(a_pMsg);
		else if (hMessageWnd == m_hWnd)
			return false;
		hMessageWnd = ::GetParent(hMessageWnd);
	}
	return false;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::GetWindow(HWND* a_phWnd)
{
	try
	{
		*a_phWnd = m_hWnd;
		return S_OK; 
	}
	catch (...)
	{
		return a_phWnd ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::ContextSensitiveHelp(BOOL a_fEnterMode)
{
	return E_NOTIMPL;
}


STDMETHODIMP CStorageFilterBrowserFileSystem::InsertMenusSB(HMENU a_hMenuShared, LPOLEMENUGROUPWIDTHS a_pMenuWidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::SetMenuSB(HMENU a_hMenuShared, HOLEMENU a_hOleMenuReserved, HWND a_hWndActiveObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::RemoveMenusSB(HMENU a_hMenuShared)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::SetStatusTextSB(LPCOLESTR a_pszStatusText)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::EnableModelessSB(BOOL a_fEnable)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::BrowseObject(LPCITEMIDLIST a_pIDL, UINT a_wFlags)
{
	try
	{
		if ((a_wFlags & (SBSP_SAMEBROWSER|SBSP_DEFBROWSER)) == 0)
			return E_NOTIMPL;
		if ((a_wFlags & SBSP_PARENT) == SBSP_PARENT)
		{
			BOOL b;
			OnFolderUp(0, 0, 0, b);
			return S_OK;
		}
		else if ((a_wFlags & SBSP_NAVIGATEBACK) == SBSP_NAVIGATEBACK)
		{
			BOOL b;
			OnFolderBack(0, 0, 0, b);
			return S_OK;
		}
		else if ((a_wFlags & SBSP_NAVIGATEFORWARD) == SBSP_NAVIGATEFORWARD)
		{
			return E_NOTIMPL;
		}
		else if ((a_wFlags & SBSP_RELATIVE) == SBSP_RELATIVE)
		{
			CPIDL cFolder(m_cFolder + a_pIDL);
			if (cFolder.GetIDCount())
			{
				CComPtr<IShellFolder> pFolder;
				m_pDesktopFolder->BindToObject(cFolder, NULL, IID_IShellFolder, (void**)&pFolder);
				if (pFolder)
				{
					m_cFolder = cFolder;
					ShowFolder(pFolder);
				}
			}
			else
			{
				m_cFolder = cFolder;
				ShowFolder(m_pDesktopFolder);
			}
			return S_OK;
		}
		else if ((a_wFlags & SBSP_ABSOLUTE) == SBSP_ABSOLUTE)
		{
			CPIDL cFolder(a_pIDL);
			if (cFolder.GetIDCount())
			{
				CComPtr<IShellFolder> pFolder;
				m_pDesktopFolder->BindToObject(cFolder, NULL, IID_IShellFolder, (void**)&pFolder);
				if (pFolder)
				{
					m_cFolder = cFolder;
					ShowFolder(pFolder);
				}
			}
			else
			{
				m_cFolder = cFolder;
				ShowFolder(m_pDesktopFolder);
			}
			return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::GetViewStateStream(DWORD a_dwGrfMode, LPSTREAM *a_ppStrm)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::OnViewWindowActive(IShellView* a_pShellView)
{
	return S_OK;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::SetToolbarItems(LPTBBUTTON a_pButtons, UINT a_nButtons, UINT a_uFlags)
{
	try
	{
		//while (m_wndToolBar.GetButtonCount())
		//	m_wndToolBar.DeleteButton(0);

		//m_wndToolBar.AddButtons(a_nButtons, a_pButtons);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::TranslateAcceleratorSB(LPMSG a_pMsg, WORD a_wID)
{
	try
	{
		if (a_pMsg->message == WM_KEYDOWN && a_pMsg->wParam == VK_BACK)
		{
			BOOL b;
			OnFolderUp(0, 0, 0, b);
			return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::QueryActiveShellView(IShellView** a_ppShellView)
{
	try
	{
		if (m_pActiveShellView != NULL)
			(*a_ppShellView = m_pActiveShellView)->AddRef();
		else
			*a_ppShellView = NULL;
		return S_OK; 
	}
	catch (...)
	{
		return a_ppShellView ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::GetControlWindow(UINT a_uID, HWND* a_phWnd)
{
	try
	{
		*a_phWnd = NULL;

		if (a_uID == FCW_TOOLBAR)
			*a_phWnd = m_wndToolBar;

		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::SendControlMsg(UINT a_uID, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, LRESULT* a_pResult)
{
	try
	{
		return E_NOTIMPL;
		if (a_uID == FCW_TOOLBAR)
		{
			*a_pResult = m_wndToolBar.SendMessage(a_uMsg, a_wParam, a_lParam);
		}
		return S_OK;
	}
	catch (...)
	{
		return a_pResult == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#define GetPIDLFolder(pida) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define GetPIDLItem(pida, i) (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

STDMETHODIMP CStorageFilterBrowserFileSystem::OnDefaultCommand(IShellView* a_pShellView)
{
	if (m_pLastFolder == NULL)
		return S_FALSE;

	CComPtr<IDataObject> pDataObj;
	a_pShellView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**)&pDataObj);
	if (pDataObj == NULL)
		return S_FALSE;

	FORMATETC fEtc;
	fEtc.cfFormat = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	fEtc.ptd = NULL;
	fEtc.tymed = TYMED_HGLOBAL;
	fEtc.dwAspect = DVASPECT_CONTENT;
	fEtc.lindex = -1;

	STGMEDIUM sMedium;
	pDataObj->GetData(&fEtc, &sMedium);
	LPIDA pCida = reinterpret_cast<LPIDA>(GlobalLock(sMedium.hGlobal));
	if (pCida->cidl > 0)
	{
		CPIDL cParent(GetPIDLFolder(pCida));
		CPIDL cItem(GetPIDLItem(pCida, 0));
		CPIDL cFull(cParent+cItem);
		CComPtr<IShellFolder> pLastFolder = m_pLastFolder;

		bool bRun = true;
		while (bRun)
		{
			LPCITEMIDLIST pIDL = cItem;
			bRun = false;

			SFGAOF tRGF;
			tRGF = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER|SFGAO_STREAM|SFGAO_LINK;
			if (SUCCEEDED(pLastFolder->GetAttributesOf(1, &pIDL, &tRGF)))
			{
				if (tRGF&SFGAO_LINK)
				{
					CComPtr<IShellLink> pLink;
					pLastFolder->BindToObject(pIDL, NULL, IID_IShellLink, (void**)&pLink);
					if (pLink == NULL)
						break; // error
					if (FAILED(pLink->GetIDList(&cFull)))
						break;
					cParent = cFull.GetParent();
					cItem = cFull.GetLastItem();
					if (cFull.GetIDCount() == 1)
					{
						pLastFolder = m_pDesktopFolder;
					}
					else
					{
						pLastFolder = NULL;
						m_pDesktopFolder->BindToObject(cParent, NULL, IID_IShellFolder, reinterpret_cast<void**>(&pLastFolder));
					}
					if (pLastFolder)
						bRun = true;
					else
						break;
				}
				else if (((tRGF&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR) ||
					(tRGF&(SFGAO_FILESYSANCESTOR|SFGAO_FOLDER)) == (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER) ||
					(tRGF&(SFGAO_FILESYSTEM|SFGAO_FOLDER)) == (SFGAO_FILESYSTEM|SFGAO_FOLDER)) && (!XPGUI::IsXP() || (tRGF&SFGAO_STREAM) == 0))
				{
					CComPtr<IShellFolder> pNewFolder;
					pLastFolder->BindToObject(pIDL, NULL, IID_IShellFolder, (void**)&pNewFolder);
					if (pNewFolder)
					{
						m_cFolder = cFull;
						ShowFolder(pNewFolder);
					}
				}
				else if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM))
				{
					STRRET tName;
					if (SUCCEEDED(pLastFolder->GetDisplayNameOf(pIDL, /*SHGDN_INFOLDER|*/SHGDN_FORPARSING, &tName)))
					{
						delete[] MyStrRetToStr(m_pShellAlloc, pIDL, tName);
						if (m_pCallback != NULL)
							m_pCallback->ForwardOK();
					}
				}
			}
		}
	}

	GlobalUnlock(sMedium.hGlobal);
	ReleaseStgMedium(&sMedium);

	return S_OK;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::OnStateChange(IShellView* a_pShellView, ULONG a_uChange)
{
	try
	{
		if (m_pLastFolder == NULL)
			return S_OK;

		switch (a_uChange)
		{
		case CDBOSC_SETFOCUS: // the focus has been set to the view
			break;
		case CDBOSC_KILLFOCUS: // the view has lost the focus
			break;
		case CDBOSC_SELCHANGE: // the selected item has changed
			if (m_pListener)
				m_pListener->LocatorTextChanged(NULL);
			break;
		case CDBOSC_RENAME: // an item has been renamed
			break;
		case CDBOSC_STATECHANGE: // an item has been checked or unchecked
			break;
		default:
			return E_FAIL;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::IncludeObject(IShellView* a_pShellView, LPCITEMIDLIST a_pIDL)
{
	try
	{
		//STRRET tName2;
		//m_pDesktopFolder->GetDisplayNameOf(a_pIDL, SHGDN_NORMAL, &tName2);
		//LPCTSTR pszName2 = MyStrRetToStr(m_pShellAlloc, a_pIDL, tName2);

		if (m_pLastFolder == NULL)
			return S_OK;

		SFGAOF tRGF;
		tRGF = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER|SFGAO_LINK;
		if (FAILED(m_pLastFolder->GetAttributesOf(1, &a_pIDL, &tRGF)))
			return S_FALSE;
		if ((tRGF&SFGAO_LINK) == SFGAO_LINK)
			return S_OK; // link (could be link to unsupported invalid file, but let's be optimistic)
		if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR))
			return S_OK; // folder (shell extension)
		if ((tRGF&(SFGAO_FILESYSANCESTOR|SFGAO_FOLDER)) == (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER))
			return S_OK; // folder (std shell extension - this computer, network)
		if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FOLDER)) == (SFGAO_FILESYSTEM|SFGAO_FOLDER))
			return S_OK; // folder (real)
		if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) != (SFGAO_FILESYSTEM))
			return S_FALSE; // something strange - not a file

		// filter match
		if (m_pActiveDocType == NULL)
			return S_OK;
		STRRET tName;
		if (FAILED(m_pLastFolder->GetDisplayNameOf(a_pIDL, SHGDN_INFOLDER|SHGDN_FORPARSING, &tName)))
			return S_OK;
		LPCTSTR pszName = MyStrRetToStr(m_pShellAlloc, a_pIDL, tName);
		HRESULT hRet = m_pActiveDocType->MatchFilename(CComBSTR(pszName));
		delete[] pszName;
		return hRet;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CStorageFilterBrowserFileSystem::GetDefaultMenuText(IShellView* a_pShellView, WCHAR *a_pszText, int a_cchMax)
{
	if (m_pCallback == NULL) return S_FALSE;
	CComPtr<ILocalizedString> pName;
	m_pCallback->DefaultCommand(&pName, NULL, NULL);
	if (pName == NULL) return S_FALSE;
	CComBSTR bstrName;
	pName->GetLocalized(m_tLocaleID, &bstrName);
	if (bstrName.Length() == 0) return S_FALSE;
	wcsncpy(a_pszText, bstrName.m_str, a_cchMax);
	a_pszText[a_cchMax-1] = L'\0';
	return S_OK;
}

STDMETHODIMP CStorageFilterBrowserFileSystem::GetViewFlags(DWORD* a_pdwFlags)
{
	try
	{
		*a_pdwFlags = 0; // or CDB2GVF_SHOWALLFILES ?
		return S_OK;
	}
	catch (...)
	{
		return a_pdwFlags ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CStorageFilterBrowserFileSystem::Notify(IShellView* a_pShellView, DWORD a_dwNotifyType)
{
	if (a_dwNotifyType == CDB2N_CONTEXTMENU_DONE)
	{
		if (m_pActiveShellView != NULL)
		{
			m_pActiveShellView->GetCurrentInfo(&m_tViewSettings);
		}
	}

	return S_OK;
}


HRESULT CStorageFilterBrowserFileSystem::ShowFolder(IShellFolder* a_pNewFolder)
{
	try
	{
		CComPtr<IShellView> pNewView;
		a_pNewFolder->CreateViewObject(m_hWnd, IID_IShellView, (void**)&pNewView);
		if (pNewView == NULL)
		{
			return E_FAIL;
		}

		m_pLastFolder = a_pNewFolder;

//		AddRef(); // ????????
		bool bSetFocus = false;
		if (m_pActiveShellView)
		{
			HWND hFoc = GetFocus();
			if (hFoc)
				bSetFocus = m_hActiveShellWnd == hFoc || ::IsChild(m_hActiveShellWnd, hFoc);
		}
		CWindow wnd = GetDlgItem(IDC_FWFILE_SHELLVIEW);
		RECT rcX = GetShellWndRectangle();
		if (SUCCEEDED(pNewView->CreateViewWindow(m_pActiveShellView, &m_tViewSettings, this, &GetShellWndRectangle(), &m_hActiveShellWnd)))
		{
			::SetWindowLong(m_hActiveShellWnd, GWL_ID, IDC_FWFILE_SHELLVIEW);

			if (m_pActiveShellView != NULL)
			{
				m_pActiveShellView->UIActivate(SVUIA_DEACTIVATE);
				m_pActiveShellView->DestroyViewWindow();
			}
			else
			{
				if (wnd.m_hWnd)
					wnd.DestroyWindow();
			}
			m_pActiveShellView = pNewView;
			m_pActiveShellView->UIActivate(bSetFocus ? SVUIA_ACTIVATE_FOCUS : SVUIA_ACTIVATE_NOFOCUS);

			SHFILEINFO sfi;
			ZeroMemory(&sfi,sizeof(sfi));
			sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
			SHGetFileInfo((LPCTSTR)m_cFolder.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ATTRIBUTES);

			LookInInit(m_cFolder);

			if (m_cFolderHistory.size() == 0 || !(m_cFolderHistory.top() == m_cFolder))
			{
				m_cFolderHistory.push(m_cFolder);
			}
			m_wndToolBar.EnableButton(ID_FWFILE_BACK, m_cFolderHistory.size() > 1);
			m_wndToolBar.EnableButton(ID_FWFILE_UP, m_cFolder.GetIDCount() > 0);

			// save last active folder in config
			CComBSTR cCFGID_FS_LASTDIRECTORY(CFGID_FS_LASTDIRECTORY);
			tstring strFolder;
			GetFolder(strFolder);
			if (!strFolder.empty())
			{
				CComBSTR bstr(strFolder.c_str());
				PortablePath::Full2Portable(&(bstr.m_str));
				TConfigValue tVal;
				tVal.eTypeID = ECVTString;
				tVal.bstrVal = bstr;
				SetConfigValue(cCFGID_FS_LASTDIRECTORY, tVal);
			}

			if (m_pListener)
				m_pListener->LocatorTextChanged(NULL);

			return S_OK;
		}
		else
		{
			bool bCreate = false;
			if (m_pActiveShellView != NULL)
			{
				m_pActiveShellView->UIActivate(SVUIA_DEACTIVATE);
				m_pActiveShellView->DestroyViewWindow();
				m_pActiveShellView = NULL;
				bCreate = true;
			}
			if (bCreate || GetDlgItem(IDC_FWFILE_SHELLVIEW).m_hWnd == NULL)
			{
				RECT rc = GetShellWndRectangle();
				CComBSTR bstrMessage;
				CMultiLanguageString::GetLocalized(L"[0409]\nCannot display folder contents. Please select another folder.[0405]\nNelze zobrazit obsah složky. Prosím, zvolte jinou složku.", m_tLocaleID, &bstrMessage);
				CStatic wnd;
				wnd.Create(m_hWnd, rc, bstrMessage, WS_CHILD|SS_CENTER, 0, IDC_FWFILE_SHELLVIEW);
				wnd.SetFont(GetFont());
				wnd.ShowWindow(SW_SHOW);
			}
			LookInInit(m_cFolder);
			m_wndToolBar.EnableButton(ID_FWFILE_BACK, m_cFolderHistory.size() > 1);
			m_wndToolBar.EnableButton(ID_FWFILE_UP, m_cFolder.GetIDCount() > 0);

			if (m_pListener)
				m_pListener->LocatorTextChanged(NULL);

			return S_FALSE;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

RECT CStorageFilterBrowserFileSystem::GetShellWndRectangle() const
{
	RECT rcSep;
	m_wndSeparator.GetWindowRect(&rcSep);
	ScreenToClient(&rcSep);
	RECT rc;
	GetClientRect(&rc);
	rc.top = rcSep.bottom;
	return rc;
}

int InitToolbarIcons(CImageList& imageList);

LRESULT CStorageFilterBrowserFileSystem::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	AddRef();

	m_wndSeparator = GetDlgItem(IDC_FWFILE_HORSEP);
	RECT rcSep;
	m_wndSeparator.GetWindowRect(&rcSep);
	ScreenToClient(&rcSep);
	rcSep.bottom = rcSep.top+1;
	RECT rcWnd;
	GetClientRect(&rcWnd);
	rcSep.left = 0;
	rcSep.right = rcWnd.right;
	m_wndSeparator.MoveWindow(&rcSep);

	int nIconSize = InitToolbarIcons(m_hTBList);

	CComPtr<IEnumUnknowns> pFormatFilters = reinterpret_cast<IEnumUnknowns*>(a_lParam);
	ULONG nSize = 0;
	if (pFormatFilters) pFormatFilters->Size(&nSize);
	if (nSize == 1)
	{
		pFormatFilters->Get(0, &m_pActiveDocType);
	}
	else if (nSize > 1)
	{
		CComObject<CDocumentTypeComposed>* pAllSup = NULL;
		CComObject<CDocumentTypeComposed>::CreateInstance(&pAllSup);
		m_pActiveDocType = pAllSup;
		pAllSup->InitAsAllSupportedFiles();
		IDocumentType* pDefType = NULL;
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<IDocumentType> pDT;
			pFormatFilters->Get(i, &pDT);
			if (pDT != NULL)
				pAllSup->DocTypesAddFromList(1, &(pDT.p));
		}
	}

	CComBSTR bstrFileBack;
	CMultiLanguageString::GetLocalized(L"[0409]Go to last folder visited[0405]Zpět do poslední navštívené složky", m_tLocaleID, &bstrFileBack);
	CComBSTR bstrFileUp;
	CMultiLanguageString::GetLocalized(L"[0409]Go up one level[0405]O úroveň výš", m_tLocaleID, &bstrFileUp);
	//CComBSTR bstrFileNewFolder;
	//CMultiLanguageString::GetLocalized(L"[0409]Add folder to favorites[0405]Přidat složku k oblíbeným", m_tLocaleID, &bstrFileNewFolder);
	CComBSTR bstrFileViewType;
	CMultiLanguageString::GetLocalized(L"[0409]View menu[0405]Menu náhledů", m_tLocaleID, &bstrFileViewType);
	//CComBSTR bstrFileRemoveFolder;
	//CMultiLanguageString::GetLocalized(L"[0409]Remove folder from favorites[0405]Odstranit složku z oblíbených", m_tLocaleID, &bstrFileRemoveFolder);

	TBBUTTON atButtons[] =
	{
		{0, ID_FWFILE_BACK, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileBack.m_str)},
		{1, ID_FWFILE_UP, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileUp.m_str)},
		//{2, ID_FWFILE_NEWFOLDER, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
		{3, ID_FWFILE_VIEWTYPE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileViewType.m_str)}
	};
	m_wndToolBar = GetDlgItem(IDC_FWFILE_TOOLBAR);
	m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndToolBar.SetImageList(m_hTBList);
	m_wndToolBar.AddButtons(itemsof(atButtons), atButtons);
	m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

	SHFILEINFO sfi;
	HIMAGELIST hIL = (HIMAGELIST)SHGetFileInfo((LPCTSTR)_T("C:\\"), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	m_wndFolder = GetDlgItem(IDC_FWFILE_FOLDER);
	m_wndFolder.SetImageList(hIL);

	CConfigValue cViewMode;
	GetConfigValue(CComBSTR(CFGID_FS_VIEWMODE), &cViewMode);
	if (cViewMode.TypeGet() == ECVTInteger)
		m_tViewSettings.ViewMode = cViewMode.operator LONG();

	CComPtr<IShellFolder> pFolder;
	m_pDesktopFolder->BindToObject(m_cFolder, NULL, IID_IShellFolder, (void**)&pFolder);
	if (pFolder)
		ShowFolder(pFolder);

	{
		RECT rcFolder;
		m_wndFolder.GetWindowRect(&rcFolder);
		ScreenToClient(&rcFolder);

		RECT rcActual;
		RECT rcDesired;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcDesired);

		// align the toolbar to the right and folder combo
		m_wndToolBar.MoveWindow(rcActual.right-rcDesired.right, rcFolder.top, rcDesired.right, rcDesired.bottom-rcDesired.top, FALSE);
		m_wndFolder.MoveWindow(rcFolder.left, rcFolder.top, rcFolder.right-rcFolder.left+rcActual.right-rcActual.left-rcDesired.right, rcFolder.bottom-rcFolder.top, FALSE);
	}

	DlgResize_Init(false, false, 0);

	return TRUE;
}

void CStorageFilterBrowserFileSystem::AddFavFolder(CPIDL const& a_cPIDL)
{
	SHFILEINFO sfi;
	ZeroMemory(&sfi,sizeof(sfi));
	//sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
	SHGetFileInfo((LPCTSTR)a_cPIDL.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_DISPLAYNAME);
	m_cFavFolders.push_back(a_cPIDL);
}

void CStorageFilterBrowserFileSystem::LookInClear()
{
	int nCount = m_wndFolder.GetCount();
	int i;
	for (i = 0; i < nCount; i++)
	{
		delete reinterpret_cast<CPIDL*>(m_wndFolder.GetItemDataPtr(i));
	}
	m_wndFolder.ResetContent();
}

void CStorageFilterBrowserFileSystem::LookInInit(const CPIDL& a_cActual)
{
	int iSelection = 0;

	LookInClear();
	{
		ITEMIDLIST const* pIDL = a_cActual.GetIDCount() ? a_cActual.operator const ITEMIDLIST*() : m_cDesktop.operator const ITEMIDLIST*();
		COMBOBOXEXITEM tItem;
		tItem.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM;// | CBEIF_INDENT;
		tItem.iItem = 0;
		//tItem.iIndent = a_iIndent;
		//const CPIDL& cAct(cItems.top());
		tItem.lParam = reinterpret_cast<LPARAM>(new CPIDL(a_cActual));
		SHFILEINFO sfi;
		tItem.pszText = sfi.szDisplayName;
		ZeroMemory(&sfi,sizeof(sfi));
		SHGetFileInfo((LPCTSTR)pIDL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL);
		tItem.iImage = sfi.iIcon;
		ZeroMemory(&sfi,sizeof(sfi));
		sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
		SHGetFileInfo((LPCTSTR)pIDL, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SELECTED | SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME);
		tItem.iSelectedImage = sfi.iIcon;
		m_wndFolder.InsertItem(&tItem);
		m_wndFolder.SetCurSel(0);
	}
}

LRESULT CStorageFilterBrowserFileSystem::OnFolderDropDown(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_wndFolder.GetCount() != 1)
		return 0;

	//CWaitCursor c;

	const CPIDL cActual(*reinterpret_cast<const CPIDL*>(m_wndFolder.GetItemDataPtr(0)));

	int iSelection = 0;

	LookInClear();

	CPIDL cParent = cActual.GetParent();
	while (cParent.GetIDCount() > 1) cParent = cParent.GetParent();
	CPIDL cDesktop;
	SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &cDesktop);
	CPIDL cMyComputer;
	SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &cMyComputer);
//	CPIDL cMyDocuments;
//	SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &cMyDocuments); 

	COMBOBOXEXITEM tItem;
	tItem.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM | CBEIF_INDENT;
	tItem.iItem = 0;
	tItem.lParam = reinterpret_cast<LPARAM>(new CPIDL());
	tItem.iIndent = 0;

	SHFILEINFO sfi;
    tItem.pszText = sfi.szDisplayName;
	ZeroMemory(&sfi,sizeof(sfi));
	SHGetFileInfo((LPCTSTR)cDesktop.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL);
    tItem.iImage = sfi.iIcon;
	ZeroMemory(&sfi,sizeof(sfi));
	sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
	SHGetFileInfo((LPCTSTR)cDesktop.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SELECTED | SHGFI_DISPLAYNAME);
	tItem.iSelectedImage = sfi.iIcon;
	m_wndFolder.InsertItem(&tItem);

	CComPtr<IEnumIDList> pDesktopItems;
	HRESULT hRes = m_pDesktopFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pDesktopItems);
	if (S_OK == hRes)
	{
		CPIDL cSubItem;
		while (pDesktopItems->Next(1, &cSubItem, NULL) == S_OK)
		{
			// expand Desktop folder
			CPIDL* pFull = new CPIDL(cDesktop+cSubItem);
			tItem.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM | CBEIF_INDENT;
			tItem.iItem++;
			tItem.lParam = reinterpret_cast<LPARAM>(pFull);
			tItem.iIndent = 1;
		    tItem.pszText = sfi.szDisplayName;
			if (cActual == *pFull)
				iSelection = tItem.iItem;

			ZeroMemory(&sfi,sizeof(sfi));
			SHGetFileInfo((LPCTSTR)pFull->operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL);
		    tItem.iImage = sfi.iIcon;
			ZeroMemory(&sfi,sizeof(sfi));
			sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
			SHGetFileInfo((LPCTSTR)pFull->operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SELECTED | SHGFI_DISPLAYNAME);
			tItem.iSelectedImage = sfi.iIcon;
			m_wndFolder.InsertItem(&tItem);
			if (cMyComputer == *pFull || cParent == *pFull)
			{
				// expand MyComputer folder
				CComPtr<IShellFolder> pMyComputer;
				m_pDesktopFolder->BindToObject(*pFull, NULL, IID_IShellFolder, (void**)&pMyComputer);
				CComPtr<IEnumIDList> pMyComputerItems;
				if (pMyComputer && S_OK == pMyComputer->EnumObjects(NULL, SHCONTF_FOLDERS, &pMyComputerItems))
				{
					CPIDL cSubItem;
					while (pMyComputerItems->Next(1, &cSubItem, NULL) == S_OK)
					{
						// expand Desktop folder
						CPIDL* pFull2 = new CPIDL(*pFull+cSubItem);
						tItem.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM | CBEIF_INDENT;
						tItem.iItem++;
						tItem.lParam = reinterpret_cast<LPARAM>(pFull2);
						tItem.iIndent = 2;
						tItem.pszText = sfi.szDisplayName;
						if (cActual == *pFull2)
							iSelection = tItem.iItem;

						ZeroMemory(&sfi,sizeof(sfi));
						SHGetFileInfo((LPCTSTR)pFull2->operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL);
						tItem.iImage = sfi.iIcon;
						ZeroMemory(&sfi,sizeof(sfi));
						sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
						SHGetFileInfo((LPCTSTR)pFull2->operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SELECTED | SHGFI_DISPLAYNAME);
						tItem.iSelectedImage = sfi.iIcon;
						m_wndFolder.InsertItem(&tItem);
						if (*pFull2 < cActual)
						{
							// actual is located under selected item
							iSelection = tItem.iItem = LookInInsert(*pFull2, cActual, tItem.iIndent, tItem.iItem);
						}
					}
				}
			}
		}
	}

	m_wndFolder.SetCurSel(iSelection);

	return 0;
}

int CStorageFilterBrowserFileSystem::LookInInsert(const CPIDL& a_cRoot, const CPIDL& a_cActual, int a_iIndent, int a_iItem)
{
	stack<CPIDL> cItems;
	CPIDL cTmp;
	for (cTmp = a_cActual; a_cRoot < cTmp; cTmp = cTmp.GetParent())
	{
		cItems.push(cTmp);
	}
	COMBOBOXEXITEM tItem;
	tItem.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_LPARAM | CBEIF_INDENT;
	tItem.iItem = a_iItem;
	tItem.iIndent = a_iIndent;

	while (!cItems.empty())
	{
		tItem.iItem++;
		tItem.iIndent++;
		tItem.lParam = reinterpret_cast<LPARAM>(new CPIDL(cItems.top()));

		const CPIDL& cAct(cItems.top());
		SHFILEINFO sfi;
		tItem.pszText = sfi.szDisplayName;
		ZeroMemory(&sfi,sizeof(sfi));
		SHGetFileInfo((LPCTSTR)cAct.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL);
		tItem.iImage = sfi.iIcon;
		ZeroMemory(&sfi,sizeof(sfi));
		sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
		SHGetFileInfo((LPCTSTR)cAct.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SELECTED | SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME);
		tItem.iSelectedImage = sfi.iIcon;
		m_wndFolder.InsertItem(&tItem);

		cItems.pop();
	}

	return tItem.iItem;
}

LRESULT CStorageFilterBrowserFileSystem::OnGetIShellBrowser(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	//AddRef(); //not addrefed
	SetWindowLongPtr(DWLP_MSGRESULT, (LONG_PTR)(IShellBrowser*)this); //use this if dialog
	return (LRESULT)(IShellBrowser*)this;
}


LRESULT CStorageFilterBrowserFileSystem::OnFolderBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_cFolderHistory.size() > 1)
	{
		m_cFolderHistory.pop();
		if (m_cFolderHistory.top().GetIDCount())
		{
			CComPtr<IShellFolder> pFolder;
			m_pDesktopFolder->BindToObject(m_cFolderHistory.top(), NULL, IID_IShellFolder, (void**)&pFolder);
			if (pFolder)
			{
				m_cFolder = m_cFolderHistory.top();
				ShowFolder(pFolder);
			}
			else
			{
				m_cFolderHistory.push(m_cFolder); // something failed - put it back
			}
		}
		else
		{
			m_cFolder = m_cFolderHistory.top();
			ShowFolder(m_pDesktopFolder);
		}
	}

	return 0;
}

LRESULT CStorageFilterBrowserFileSystem::OnFolderUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_cFolder.GetIDCount())
	{
		CPIDL cParent(m_cFolder.GetParent());
		if (cParent.GetIDCount())
		{
			CComPtr<IShellFolder> pFolder;
			m_pDesktopFolder->BindToObject(cParent, NULL, IID_IShellFolder, (void**)&pFolder);
			if (pFolder)
			{
				m_cFolder = cParent;
				ShowFolder(pFolder);
			}
		}
		else
		{
			m_cFolder = cParent;
			ShowFolder(m_pDesktopFolder);
		}
	}

	return 0;
}

enum EViewModeBySystem
{
	EVMBSXP = 1,
	EVMBSOld = 4,
};

struct TViewType
{
	FOLDERVIEWMODE first;
	wchar_t const* second;
	int flags;
};

LRESULT CStorageFilterBrowserFileSystem::OnViewTypeMenu(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
	if (pTB->iItem == ID_FWFILE_VIEWTYPE)
	{
		CMenu cMenu;
		cMenu.CreatePopupMenu();

		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		static BOOL bDummy = GetVersionEx(&tVersion);
		static int eSystem = (tVersion.dwMajorVersion > 5 || (tVersion.dwMajorVersion == 5 && tVersion.dwMinorVersion >= 1)) ? EVMBSXP : EVMBSOld;

		static TViewType const aTypes[] =
		{
			{FVM_ICON, L"[0409]Icons[0405]Ikony", EVMBSXP | EVMBSOld},
			{FVM_SMALLICON, L"[0409]Small icons[0405]Malé ikony", EVMBSOld},
			{FVM_LIST, L"[0409]List[0405]Seznam", EVMBSXP|EVMBSOld},
			{FVM_DETAILS, L"[0409]Details[0405]Podrobnosti", EVMBSXP|EVMBSOld},
			{FVM_THUMBNAIL, L"[0409]Thumbnails[0405]Miniatury", EVMBSXP},
			{FVM_TILE, L"[0409]Tiles[0405]Vedle sebe", EVMBSXP},
			{FVM_THUMBSTRIP, L"[0409]Filmstrip[0405]Filmový pás", EVMBSXP},
		};
		for (size_t i = 0; i < itemsof(aTypes); i++)
		{
			if ((aTypes[i].flags&eSystem) == eSystem)
			{
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(aTypes[i].second, m_tLocaleID, &bstr);
				cMenu.AppendMenu(aTypes[i].first == m_tViewSettings.ViewMode ? (MF_STRING | MFT_RADIOCHECK | MFS_CHECKED) : MF_STRING, ID_VIEWTYPE_BASE+aTypes[i].first, bstr.m_str);
			}
		}

		POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
		m_wndToolBar.ClientToScreen(&ptBtn);

		cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ptBtn.x, ptBtn.y, m_hWnd, NULL);
	}
	return TBDDRET_DEFAULT;
}

LRESULT CStorageFilterBrowserFileSystem::OnViewTypeChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	a_wID -= ID_VIEWTYPE_BASE;
	if (m_tViewSettings.ViewMode != a_wID)
	{
		if (m_cFolder.GetIDCount())
		{
			CComPtr<IShellFolder> pFolder;
			m_pDesktopFolder->BindToObject(m_cFolder, NULL, IID_IShellFolder, (void**)&pFolder);
			if (pFolder)
			{
				m_tViewSettings.ViewMode = a_wID;
				ShowFolder(pFolder);
			}
		}
		else
		{
			m_tViewSettings.ViewMode = a_wID;
			ShowFolder(m_pDesktopFolder);
		}
		SetConfigValue(CComBSTR(CFGID_FS_VIEWMODE), CConfigValue(LONG(m_tViewSettings.ViewMode)));
	}

	return 0;
}

LRESULT CStorageFilterBrowserFileSystem::OnFolderChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	const CPIDL* pIDL = reinterpret_cast<const CPIDL*>(m_wndFolder.GetItemDataPtr(m_wndFolder.GetCurSel()));
	if (pIDL->GetIDCount())
	{
		CComPtr<IShellFolder> pFolder;
		m_pDesktopFolder->BindToObject(*pIDL, NULL, IID_IShellFolder, (void**)&pFolder);
		if (pFolder)
		{
			m_cFolder = *pIDL;
			ShowFolder(pFolder);
		}
	}
	else
	{
		m_cFolder = *pIDL;
		ShowFolder(m_pDesktopFolder);
	}

	return 0;
}

void CStorageFilterBrowserFileSystem::GetFolder(tstring& a_strOut) const
{
	STRRET str;
	m_pDesktopFolder->GetDisplayNameOf(m_cFolder, SHGDN_FORPARSING, &str);
	LPTSTR pFld = MyStrRetToStr(m_pShellAlloc, NULL, str);

	if (pFld != NULL)
		a_strOut = pFld;

	delete[] pFld;
}

