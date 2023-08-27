// StorageFilterWindowFileSystem.cpp : implementation of the CStorageFilterWindowFileSystem class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StorageFilterWindowFileSystem.h"

#include <XPGUI.h>
#include "ConfigIDsFileSystem.h"
#include "DocumentTypeWildcards.h"
#include "DocumentTypeComposed.h"
#include <PortablePath.h>


LPWSTR MyStrRetToWStr(IMalloc* a_pMalloc, const ITEMIDLIST* a_pIDL, STRRET a_strret)
{
	LPWSTR pszRet = NULL;

	switch (a_strret.uType)
	{
		case STRRET_WSTR:
			{
				pszRet = new WCHAR[wcslen(a_strret.pOleStr) + 1];
				wcscpy(pszRet, a_strret.pOleStr);
				a_pMalloc->Free(a_strret.pOleStr);
			}
			break;

		case STRRET_OFFSET:
			{
				CA2W c(reinterpret_cast<const char*>(a_pIDL) + a_strret.uOffset);
				pszRet = new WCHAR[wcslen(c) + 1];
				wcscpy(pszRet, c);
			}
			break;

		case STRRET_CSTR:
			{
				CA2W c(a_strret.cStr);
				pszRet = new WCHAR[wcslen(c) + 1];
				wcscpy(pszRet, c);
			}
			break;
	}

	return pszRet;
}

LPSTR MyStrRetToAStr(IMalloc* a_pMalloc, const ITEMIDLIST* a_pIDL, STRRET a_strret)
{
	LPSTR pszRet = NULL;

	switch (a_strret.uType)
	{
		case STRRET_WSTR:
			{
				CW2A c(a_strret.pOleStr);
				pszRet = new char[strlen(c) + 1];
				strcpy(pszRet, c);
				a_pMalloc->Free(a_strret.pOleStr);
			}
			break;

		case STRRET_OFFSET:
			{
				pszRet = new char[strlen(reinterpret_cast<const char*>(a_pIDL) + a_strret.uOffset) + 1];
				strcpy(pszRet, reinterpret_cast<const char*>(a_pIDL) + a_strret.uOffset);
			}
			break;

		case STRRET_CSTR:
			{
				pszRet = new char[strlen(a_strret.cStr) + 1];
				strcpy(pszRet, a_strret.cStr);
			}
			break;
	}

	return pszRet;
}

#ifdef _UNICODE
#define MyStrRetToStr MyStrRetToWStr
#else
#define MyStrRetToStr MyStrRetToAStr
#endif

#include "PathAnalyzer.h"


// CStorageFilterWindowFileSystem

HRESULT CStorageFilterWindowFileSystem::Init(LPCOLESTR a_pszInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pContextConfig, IStorageFilterWindowCallback* a_pCallback, IStorageFilterWindowListener* a_pListener, HWND a_hWnd, LCID a_tLocaleID)
{
	m_tLocaleID = a_tLocaleID;
	m_pCallback = a_pCallback;
	m_pListener = a_pListener;
	CComPtr<IDocumentType> pDefDocType;
	if (m_pListener)
		m_pListener->DefaultDocumentGet(&pDefDocType);

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

LRESULT CStorageFilterWindowFileSystem::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
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

LRESULT CStorageFilterWindowFileSystem::OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	if (m_wndName.IsWindow())
	{
		GotoDlgCtrl(m_wndName);
	}
	return 0;
}

LRESULT CStorageFilterWindowFileSystem::OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardOK();

	return 0;
}

LRESULT CStorageFilterWindowFileSystem::OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pCallback != NULL)
		m_pCallback->ForwardCancel();

	return 0;
}


STDMETHODIMP CStorageFilterWindowFileSystem::FilterCreate(IStorageFilter** a_ppFilter)
{
	try
	{
		*a_ppFilter = NULL;

		CComPtr<IStorageFilterFactory> pFct;
		RWCoCreateInstance(pFct, __uuidof(StorageFilterFactoryFileSystem));
		tstring strOut;
		switch(GetFilter(strOut))
		{
		case 0:
			return S_FALSE;
		case 2:
			{
				CComBSTR bstrCaption;
				CMultiLanguageString::GetLocalized(L"[0409]Select Location[0405]Výběr umístění", m_tLocaleID, &bstrCaption);
				CComBSTR bstrMessage;
				CMultiLanguageString::GetLocalized(L"[0409]The file already exists. Do you wish to overwrite it?[0405]Soubor již existuje. Přejete si ho přepsat?", m_tLocaleID, &bstrMessage);
				if (MessageBox(bstrMessage, bstrCaption, MB_YESNO) == IDNO)
					return S_FALSE;
			}
		case 1:
			break;
		}

		HRESULT hRes = pFct->FilterCreate(CComBSTR(strOut.c_str()), 0, a_ppFilter);
		if (SUCCEEDED(hRes) && strOut.length() != 0 && m_pContextConfig != NULL)
		{
			CConfigValue cVal;
			CComBSTR cCFGID_FS_RECENTFILES(CFGID_FS_RECENTFILES);
			m_pContextConfig->ItemValueGet(cCFGID_FS_RECENTFILES, &cVal);
			std::vector<std::wstring> cRecentFiles;
			LPCOLESTR psz = cVal;
			LPCOLESTR pszStart = psz;
			while (*psz)
			{
				if (L'|' == *psz)
				{
					if (psz > pszStart)
					{
						cRecentFiles.push_back(std::wstring(pszStart, psz));
					}
					pszStart = psz+1;
				}
				++psz;
			}
			if (psz > pszStart)
			{
				cRecentFiles.push_back(std::wstring(pszStart, psz));
			}
			CComBSTR bstrNew;
			(*a_ppFilter)->ToText(NULL, &bstrNew);
			PortablePath::Full2Portable(&(bstrNew.m_str));
			for (std::vector<std::wstring>::iterator i = cRecentFiles.begin(); i != cRecentFiles.end(); ++i)
			{
				if (i->compare(bstrNew.m_str) == 0)
				{
					cRecentFiles.erase(i);
					break;
				}
			}
			cRecentFiles.insert(cRecentFiles.begin(), bstrNew.m_str);
			if (cRecentFiles.size() > 16)
				cRecentFiles.resize(16);
			wstring strFinal;
			for (std::vector<std::wstring>::const_iterator i = cRecentFiles.begin(); i != cRecentFiles.end(); ++i)
			{
				if (strFinal.length() != 0)
					strFinal.append(L"|");
				strFinal.append(*i);
			}
			m_pContextConfig->ItemValuesSet(1, &cCFGID_FS_RECENTFILES.m_str, CConfigValue(strFinal.c_str()));
		}
		return hRes;
	}
	catch (...)
	{
		return a_ppFilter == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CStorageFilterWindowFileSystem::FiltersCreate(IEnumUnknowns** a_ppFilters)
{
	try
	{
		*a_ppFilters = NULL;
		CComPtr<IStorageFilter> pFlt;
		HRESULT hRes = FilterCreate(&pFlt);
		if (pFlt == NULL)
			return hRes;
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		p->Insert(pFlt);
		*a_ppFilters = p.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFilters ? E_UNEXPECTED : E_NOTIMPL;
	}
}

LRESULT CStorageFilterWindowFileSystem::OnNameChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pListener != NULL)
	{
		TCHAR szName[MAX_PATH] = _T("");
		m_wndName.GetWindowText(szName, itemsof(szName));

		std::tstring strDefExt;
		CComBSTR bstrDefExt;
		if (m_pActiveDocType != NULL && (m_dwFlags&(EFTOpenExisting|EFTCreateNew)) == EFTCreateNew)
			m_pActiveDocType->DefaultExtensionGet(&bstrDefExt);
		if (bstrDefExt != NULL && bstrDefExt[0] != L'\0')
		{
			strDefExt = CW2CT(bstrDefExt);
		}
		std::tstring strLocator(szName);
		switch (AnalyzePath(m_pShellAlloc, m_pDesktopFolder, m_cFolder, strLocator, strDefExt.empty() ? NULL : strDefExt.c_str()))
		{
		case EPARExistingFile:
		case EPARExistingFolder:
		case EPARNewFileInExistingFolder:
		case EPARNonExistingPath:
			m_pListener->LocatorTextChanged(CComBSTR(strLocator.c_str()));
		}
	}
	return 0;
}

bool CStorageFilterWindowFileSystem::MessagePumpPretranslateMessage(LPMSG a_pMsg)
{
	if (!IsChild(a_pMsg->hwnd))
		return false;
	if (m_pActiveShellView == NULL)
		return false;
	HWND hShellWnd = NULL;
	m_pActiveShellView->GetWindow(&hShellWnd);
	if (hShellWnd && ::IsChild(hShellWnd, a_pMsg->hwnd))
		return S_OK == m_pActiveShellView->TranslateAccelerator(a_pMsg);
	return false;
}

STDMETHODIMP CStorageFilterWindowFileSystem::GetWindow(HWND* a_phWnd)
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

STDMETHODIMP CStorageFilterWindowFileSystem::ContextSensitiveHelp(BOOL a_fEnterMode)
{
	return E_NOTIMPL;
}


STDMETHODIMP CStorageFilterWindowFileSystem::InsertMenusSB(HMENU a_hMenuShared, LPOLEMENUGROUPWIDTHS a_pMenuWidths)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowFileSystem::SetMenuSB(HMENU a_hMenuShared, HOLEMENU a_hOleMenuReserved, HWND a_hWndActiveObject)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowFileSystem::RemoveMenusSB(HMENU a_hMenuShared)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowFileSystem::SetStatusTextSB(LPCOLESTR a_pszStatusText)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowFileSystem::EnableModelessSB(BOOL a_fEnable)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowFileSystem::BrowseObject(LPCITEMIDLIST a_pIDL, UINT a_wFlags)
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

STDMETHODIMP CStorageFilterWindowFileSystem::GetViewStateStream(DWORD a_dwGrfMode, LPSTREAM *a_ppStrm)
{
	return E_NOTIMPL;
}

STDMETHODIMP CStorageFilterWindowFileSystem::OnViewWindowActive(IShellView* a_pShellView)
{
	return S_OK;
}

STDMETHODIMP CStorageFilterWindowFileSystem::SetToolbarItems(LPTBBUTTON a_pButtons, UINT a_nButtons, UINT a_uFlags)
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

STDMETHODIMP CStorageFilterWindowFileSystem::TranslateAcceleratorSB(LPMSG a_pMsg, WORD a_wID)
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

STDMETHODIMP CStorageFilterWindowFileSystem::QueryActiveShellView(IShellView** a_ppShellView)
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

STDMETHODIMP CStorageFilterWindowFileSystem::GetControlWindow(UINT a_uID, HWND* a_phWnd)
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

STDMETHODIMP CStorageFilterWindowFileSystem::SendControlMsg(UINT a_uID, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, LRESULT* a_pResult)
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

STDMETHODIMP CStorageFilterWindowFileSystem::OnDefaultCommand(IShellView* a_pShellView)
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
						LPCTSTR pszName = MyStrRetToStr(m_pShellAlloc, pIDL, tName);
						m_wndName.SetWindowText(pszName);
						delete[] pszName;
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

STDMETHODIMP CStorageFilterWindowFileSystem::OnStateChange(IShellView* a_pShellView, ULONG a_uChange)
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
		{
			CComPtr<IDataObject> pDataObj;
			a_pShellView->GetItemObject(SVGIO_SELECTION, IID_IDataObject, (void**)&pDataObj);
			if (pDataObj == NULL)
				break;

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
				LPCITEMIDLIST pIDL = cItem;

				SFGAOF tRGF;
				tRGF = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER|SFGAO_LINK;
				if (SUCCEEDED(m_pLastFolder->GetAttributesOf(1, &pIDL, &tRGF)))
				{
					if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR|SFGAO_FOLDER|SFGAO_LINK)) == (SFGAO_FILESYSTEM))
					{
						STRRET tName;
						if (SUCCEEDED(m_pLastFolder->GetDisplayNameOf(pIDL, SHGDN_INFOLDER|SHGDN_FORPARSING, &tName)))
						{
							LPCTSTR pszName = MyStrRetToStr(m_pShellAlloc, pIDL, tName);
							m_wndName.SetWindowText(pszName);
							delete[] pszName;
						}
					}
				}
			}

			GlobalUnlock(sMedium.hGlobal);
			ReleaseStgMedium(&sMedium);
		}
		break;
	case CDBOSC_RENAME: // an item has been renamed
		// TODO: update name combobox
		break;
	case CDBOSC_STATECHANGE: // an item has been checked or unchecked
		break;
	default:
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP CStorageFilterWindowFileSystem::IncludeObject(IShellView* a_pShellView, LPCITEMIDLIST a_pIDL)
{
	try
	{
		//STRRET tName2;
		//m_pDesktopFolder->GetDisplayNameOf(a_pIDL, SHGDN_NORMAL, &tName2);
		//LPCTSTR pszName2 = MyStrRetToStr(m_pShellAlloc, a_pIDL, tName2);

		if (m_pLastFolder == NULL)
			return S_OK;

		SFGAOF tRGF;
		tRGF = SFGAO_FILESYSANCESTOR|SFGAO_FILESYSTEM|SFGAO_FOLDER|SFGAO_STREAM|SFGAO_LINK;
		if (FAILED(m_pLastFolder->GetAttributesOf(1, &a_pIDL, &tRGF)))
			return S_FALSE;
		if ((tRGF&SFGAO_LINK) == SFGAO_LINK)
			return S_OK; // link (could be link to unsupported invalid file, but let's be optimistic)
		if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR)) == (SFGAO_FILESYSTEM|SFGAO_FILESYSANCESTOR))
			return S_OK; // folder (shell extension)
		if ((tRGF&(SFGAO_FILESYSANCESTOR|SFGAO_FOLDER)) == (SFGAO_FILESYSANCESTOR|SFGAO_FOLDER))
			return S_OK; // folder (std shell extension - this computer, network)
		if ((tRGF&(SFGAO_FILESYSTEM|SFGAO_FOLDER)) == (SFGAO_FILESYSTEM|SFGAO_FOLDER) && 0 == (tRGF&SFGAO_STREAM))
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


STDMETHODIMP CStorageFilterWindowFileSystem::GetDefaultMenuText(IShellView* a_pShellView, WCHAR *a_pszText, int a_cchMax)
{
	return S_FALSE;
}

STDMETHODIMP CStorageFilterWindowFileSystem::GetViewFlags(DWORD* a_pdwFlags)
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

STDMETHODIMP CStorageFilterWindowFileSystem::Notify(IShellView* a_pShellView, DWORD a_dwNotifyType)
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


HRESULT CStorageFilterWindowFileSystem::ShowFolder(IShellFolder* a_pNewFolder)
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

			bool bInFavs = false;

			// favorite folders
			for (std::vector<CPIDL>::const_iterator i = m_cFavFolders.begin(); i != m_cFavFolders.end(); ++i)
			{
				bool bMatch = *i == m_cFolder ||
					((m_cFolder == m_cDesktopDir || m_cFolder.GetIDCount() == 0) && *i == m_cDesktop); // desktop is special case
				m_wndFavFolders.CheckButton(IDC_FWFILE_FAVFOLDER1+(i-m_cFavFolders.begin()), bMatch);
				if (bMatch) bInFavs = true;
			}

			if (bInFavs)
			{
				CComBSTR bstrFileRemoveFolder;
				CMultiLanguageString::GetLocalized(L"[0409]Remove folder from favorites[0405]Odstranit složku z oblíbených", m_tLocaleID, &bstrFileRemoveFolder);
				TBBUTTONINFO tInfo;
				ZeroMemory(&tInfo, sizeof tInfo);
				tInfo.cbSize = sizeof tInfo;
				tInfo.dwMask = TBIF_IMAGE | TBIF_TEXT;
				tInfo.iImage = 4;
				tInfo.pszText = bstrFileRemoveFolder;
				m_wndToolBar.SetButtonInfo(ID_FWFILE_NEWFOLDER, &tInfo);
				m_wndToolBar.EnableButton(ID_FWFILE_NEWFOLDER, !(m_cFolder == m_cDesktopDir || m_cFolder.GetIDCount() == 0));
			}
			else
			{
				CComBSTR bstrFileNewFolder;
				CMultiLanguageString::GetLocalized(L"[0409]Add folder to favorites[0405]Přidat složku k oblíbeným", m_tLocaleID, &bstrFileNewFolder);
				TBBUTTONINFO tInfo;
				ZeroMemory(&tInfo, sizeof tInfo);
				tInfo.cbSize = sizeof tInfo;
				tInfo.dwMask = TBIF_IMAGE | TBIF_TEXT;
				tInfo.iImage = 2;
				tInfo.pszText = bstrFileNewFolder;
				m_wndToolBar.SetButtonInfo(ID_FWFILE_NEWFOLDER, &tInfo);
				m_wndToolBar.EnableButton(ID_FWFILE_NEWFOLDER, TRUE);
			}
			UpdateAutoFolders();

			// AutoComplete
			CComQIPtr<IPersistFolder> pACFolder(m_pAutoComplete);
			if (pACFolder != NULL)
			{
				pACFolder->Initialize(m_cFolder);
			}

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
			// AutoComplete
			CComQIPtr<IPersistFolder> pACFolder(m_pAutoComplete);
			if (pACFolder != NULL)
			{
				pACFolder->Initialize(m_cFolder);
			}
			return S_FALSE;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

RECT CStorageFilterWindowFileSystem::GetShellWndRectangle() const
{
	RECT rcFolder;
	GetDlgItem(IDC_FWFILE_FOLDER).GetWindowRect(&rcFolder);
	ScreenToClient(&rcFolder);
	RECT rcName;
	GetDlgItem(IDC_FWFILE_NAME).GetWindowRect(&rcName);
	ScreenToClient(&rcName);
	RECT rcTmp;
	int nGapY = MulDiv(4, HIWORD(GetDialogBaseUnits()), 8);
	rcTmp.top = rcFolder.bottom+nGapY;
	rcTmp.bottom = rcName.top-nGapY;
	rcTmp.left = rcFolder.left;
	rcTmp.right = rcName.right;
	return rcTmp;
}

namespace std
{
	template<typename T>
	void swap(CComPtr<T>& p1, CComPtr<T>& p2)
	{
		T* p = p2;
		p2.p = p1.p;
		p1.p = p;
	}
}

int InitToolbarIcons(CImageList& imageList);

LRESULT CStorageFilterWindowFileSystem::OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	AddRef();

	//SetWindowTheme(L"Explorer", NULL);

	int nIconSize = InitToolbarIcons(m_hTBList);

	CComBSTR bstrFileBack;
	CMultiLanguageString::GetLocalized(L"[0409]Go to last folder visited[0405]Zpět do poslední navštívené složky", m_tLocaleID, &bstrFileBack);
	CComBSTR bstrFileUp;
	CMultiLanguageString::GetLocalized(L"[0409]Go up one level[0405]O úroveň výš", m_tLocaleID, &bstrFileUp);
	CComBSTR bstrFileNewFolder;
	CMultiLanguageString::GetLocalized(L"[0409]Add folder to favorites[0405]Přidat složku k oblíbeným", m_tLocaleID, &bstrFileNewFolder);
	CComBSTR bstrFileViewType;
	CMultiLanguageString::GetLocalized(L"[0409]View menu[0405]Menu náhledů", m_tLocaleID, &bstrFileViewType);
	//CComBSTR bstrFileRemoveFolder;
	//CMultiLanguageString::GetLocalized(L"[0409]Remove folder from favorites[0405]Odstranit složku z oblíbených", m_tLocaleID, &bstrFileRemoveFolder);

	TBBUTTON atButtons[] =
	{
		{0, ID_FWFILE_BACK, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileBack.m_str)},
		{1, ID_FWFILE_UP, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileUp.m_str)},
		{2, ID_FWFILE_NEWFOLDER, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileNewFolder.m_str)},
		{3, ID_FWFILE_VIEWTYPE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrFileViewType.m_str)}
	};
	m_wndToolBar = GetDlgItem(IDC_FWFILE_TOOLBAR);
	m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndToolBar.SetImageList(m_hTBList);
	m_wndToolBar.AddButtons(itemsof(atButtons), atButtons);
	m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

	SHFILEINFO sfi;
	HIMAGELIST hIL = (HIMAGELIST)SHGetFileInfo((LPCTSTR)_T("C:\\"), 
                                           0,
                                           &sfi, 
                                           sizeof(SHFILEINFO), 
                                           SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	m_wndFolder = GetDlgItem(IDC_FWFILE_FOLDER);
	m_wndFolder.SetImageList(hIL);


	// filter combo
	USES_CONVERSION;
	m_wndName = GetDlgItem(IDC_FWFILE_NAME);
	//m_wndName.GetComboCtrl().ModifyStyle(0, CBS_AUTOHSCROLL);
	//m_wndName.GetEditCtrl().ModifyStyle(0, ES_AUTOHSCROLL);
	m_wndName.SetWindowText(m_szName);
	m_pAutoComplete.CoCreateInstance(CLSID_ACListISF);
	CComPtr<IAutoComplete> pACInit;
	pACInit.CoCreateInstance(CLSID_AutoComplete);
	if (pACInit != NULL && m_pAutoComplete != NULL)
	{
		pACInit->Init(m_wndName.GetEditCtrl(), m_pAutoComplete, NULL, NULL);
	}
	CComQIPtr<IAutoComplete2> pACInit2(pACInit);
	if (pACInit2 != NULL)
	{
		pACInit2->SetOptions(ACO_AUTOSUGGEST|ACO_AUTOAPPEND);
	}
	IACList2* pList = NULL;
	m_pAutoComplete->QueryInterface(IID_IACList2, reinterpret_cast<void**>(&pList));
	if (pList != NULL)
	{
		pList->SetOptions(ACLO_FILESYSONLY|ACLO_CURRENTDIR);
		pList->Release();
	}


	m_wndFilter = GetDlgItem(IDC_FWFILE_FILTER);
	CComPtr<IEnumUnknowns> pFormatFilters = reinterpret_cast<IEnumUnknowns*>(a_lParam);
	CComObject<CDocumentTypeComposed>* pAllSup = NULL;
	CComObject<CDocumentTypeComposed>::CreateInstance(&pAllSup);
	CComPtr<IDocumentType> pAllSup2 = pAllSup;
	pAllSup->InitAsAllSupportedFiles();
	IDocumentType* pDefType = NULL;
	if (pFormatFilters != NULL)
	{
		ULONG nSize = 0;
		pFormatFilters->Size(&nSize);
		for (ULONG i = 0; i < nSize; ++i)
		{
			CComPtr<IDocumentType> pDT;
			pFormatFilters->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pDT));
			if (pDT != NULL)
			{
				m_cDocTypes.push_back(pDT);
				pAllSup->DocTypesAddFromList(1, &(pDT.p));
				if (m_nDefDocType == i)
					pDefType = pDT;
			}
		}
	}
	if (m_cDocTypes.size() > 1)
	{
		std::sort(m_cDocTypes.begin(), m_cDocTypes.end(), localized_doctype_compare(m_tLocaleID));
		m_cDocTypes.insert(m_cDocTypes.begin(), pAllSup2);
	}
	{
		CComObject<CDocumentTypeComposed>* pAll = NULL;
		CComObject<CDocumentTypeComposed>::CreateInstance(&pAll);
		m_pActiveDocType = pAll;
		pAll->InitAsAllFiles();
		m_cDocTypes.push_back(m_pActiveDocType);
	}
	// fill filters combo box
	nIconSize = XPGUI::GetSmallIconSize();
	m_hFilterImages.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 4, 4);
	m_wndFilter.SetImageList(m_hFilterImages);
	COMBOBOXEXITEM tItem;
	tItem.mask = CBEIF_TEXT|CBEIF_IMAGE|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	tItem.iItem = 0;
	for (CDocTypes::const_iterator i = m_cDocTypes.begin(); i != m_cDocTypes.end(); ++i)
	{
		CComPtr<ILocalizedString> pLocName;
		(*i)->FilterNameGet(&pLocName);
		CComBSTR bstrName;
		if (pLocName != NULL)
			pLocName->GetLocalized(m_tLocaleID, &bstrName);
		if (bstrName == NULL)
			bstrName = L"";
		CW2T strName(bstrName);
		tItem.pszText = strName;

		HICON hIcon = NULL;
		(*i)->IconGet(0, nIconSize, &hIcon);
		if (hIcon)
		{
			m_hFilterImages.AddIcon(hIcon);
			DestroyIcon(hIcon);
			tItem.iImage = tItem.iSelectedImage = m_hFilterImages.GetImageCount()-1;
		}
		else
		{
			tItem.iImage = tItem.iSelectedImage = -1;
		}

		tItem.lParam = reinterpret_cast<INT_PTR>(i->p);
		m_wndFilter.InsertItem(&tItem);
		++tItem.iItem;
		//m_wndFilter.SetItemDataPtr(m_wndFilter.AddString(CW2T(bstrName)), i->p);
	}
	m_wndFilter.EnableWindow(m_cDocTypes.size() > 1);

	// set active filter
	if (m_wndFilter.GetCount() > 0)
	{
		// priority: default DocType in listener, (TODO: extension in name), last used filter
		if (pDefType)
		{
			int i = 0;
			while (i < m_wndFilter.GetCount())
			{
				IDocumentType* pType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
				if (pType == pDefType)
				{
					m_pActiveDocType = pType;
					break;
				}
				++i;
			}
			i %= m_wndFilter.GetCount();
			m_wndFilter.SetCurSel(i);
			m_pActiveDocType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
			NotifyListenerDocumentChange();
		}
		else
		{
			int i = 0;
			CConfigValue cLastFilter;
			GetConfigValue(CComBSTR(CFGID_FS_LASTFILTER), &cLastFilter);
			if (cLastFilter.TypeGet() == ECVTString)
			{
				while (i < m_wndFilter.GetCount())
				{
					IDocumentType* pType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
					CComBSTR bstrUID;
					pType->UniqueIDGet(&bstrUID);
					if (bstrUID != NULL && cLastFilter.operator BSTR() != NULL && wcscmp(bstrUID, cLastFilter) == 0)
					{
						m_pActiveDocType = pType;
						break;
					}
					++i;
				}
			}
			i %= m_wndFilter.GetCount();
			m_wndFilter.SetCurSel(i);
			m_pActiveDocType = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(i));
			NotifyListenerDocumentChange();
		}
	}

	// init favorite folders toolbar
	{
		CConfigValue cFavFolders;
		GetConfigValue(CComBSTR(CFGID_FS_FAVFOLDERS), &cFavFolders);
		m_wndFavFolders = GetDlgItem(IDC_FWFILE_FOLDERS);
		SHFILEINFO sfi;
		HIMAGELIST hIL = (HIMAGELIST)SHGetFileInfo((LPCTSTR)_T("C:\\"), 
											0,
											&sfi, 
											sizeof(SHFILEINFO), 
											SHGFI_SYSICONINDEX);
		m_wndFavFolders.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndFavFolders.SetImageList(hIL);

		if (cFavFolders.operator BSTR())
		{
			m_strFavFolders = cFavFolders;
			LPCOLESTR const pszFavFolders = cFavFolders;
			LPCOLESTR p = pszFavFolders;
			while (true)
			{
				LPCOLESTR const p1 = p;
				while (*p && *p != L'|') ++p;
				if (*p1)
				{
					if (p1[0] == L'0' && (p1[1] == L'x' || p1[1] == L'X'))
					{
						// CSIDL
						int nCSIDL = 0;
						if (1 == swscanf(p1+2, L"%x", &nCSIDL))
						{
							CPIDL cFF;
							SHGetSpecialFolderLocation(NULL, nCSIDL, &cFF);
							AddFavFolder(cFF);
						}
					}
					else
					{
						// path
						CComBSTR bstr(p-p1, p1);
						PortablePath::Portable2Full(&(bstr.m_str));
						CPIDL cFF;
						ULONG nEaten = 0;
						ULONG nAttribs = SFGAO_FOLDER;
						if (SUCCEEDED(m_pDesktopFolder->ParseDisplayName(NULL, NULL, bstr, &nEaten, &cFF, &nAttribs)))
						{
							AddFavFolder(cFF);
						}
					}
				}
				if (*p)
				{
					++p;
				}
				else
					break;
			}
		}
		RECT rc;
		m_wndFavFolders.GetClientRect(&rc);
		m_wndFavFolders.SetButtonWidth(rc.right-rc.left, rc.right-rc.left);
	}

	CConfigValue cViewMode;
	GetConfigValue(CComBSTR(CFGID_FS_VIEWMODE), &cViewMode);
	if (cViewMode.TypeGet() == ECVTInteger)
		m_tViewSettings.ViewMode = cViewMode.operator LONG();

	CComPtr<IShellFolder> pFolder;
	m_pDesktopFolder->BindToObject(m_cFolder, NULL, IID_IShellFolder, (void**)&pFolder);
	if (pFolder)
		ShowFolder(pFolder);

	// init last files combo
	CConfigValue cVal;
	GetConfigValue(CComBSTR(CFGID_FS_RECENTFILES), &cVal);
	if (cVal.TypeGet() == ECVTString)
	{
		LPCOLESTR psz = cVal;
		LPCOLESTR pszStart = psz;
		while (*psz)
		{
			if (L'|' == *psz)
			{
				if (psz > pszStart)
				{
					CComBSTR bstr(psz-pszStart, pszStart);
					PortablePath::Portable2Full(&(bstr.m_str));
					m_cRecentFiles.push_back(std::wstring(bstr.m_str));
				}
				pszStart = psz+1;
			}
			++psz;
		}
		if (psz > pszStart)
		{
			CComBSTR bstr(psz-pszStart, pszStart);
			PortablePath::Portable2Full(&(bstr.m_str));
			m_cRecentFiles.push_back(std::wstring(bstr.m_str));
		}
	}
	UpdateAutoFolders();

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
		m_wndToolBar.MoveWindow(rcActual.right-rcDesired.right, rcFolder.top+((rcFolder.bottom-rcFolder.top-rcDesired.bottom)>>1), rcDesired.right, rcDesired.bottom-rcDesired.top, FALSE);
		m_wndFolder.MoveWindow(rcFolder.left, rcFolder.top, rcFolder.right-rcFolder.left+rcActual.right-rcActual.left-rcDesired.right, rcFolder.bottom-rcFolder.top, FALSE);
	}

	DlgResize_Init(false, false, 0);

	return TRUE;
}

void CStorageFilterWindowFileSystem::UpdateAutoFolders()
{
	size_t count = m_wndFavFolders.GetButtonCount();
	bool sep = count == m_cFavFolders.size();
	while (count > m_cFavFolders.size()+1)
		m_wndFavFolders.DeleteButton(--count);
	m_cAutoFolders.clear();
	for (std::vector<std::wstring>::const_iterator i = m_cRecentFiles.begin(); i != m_cRecentFiles.end(); ++i)
	{
		CW2T cTxt(i->c_str());
		COMBOBOXEXITEM tItem;
		ZeroMemory(&tItem, sizeof(tItem));
		tItem.mask = CBEIF_TEXT;
		tItem.iItem = i-m_cRecentFiles.begin();
		tItem.pszText = cTxt;
		m_wndName.InsertItem(&tItem);

		CPIDL cAF;
		ULONG nEaten = 0;
		ULONG nAttribs = SFGAO_FOLDER;
		if (SUCCEEDED(m_pDesktopFolder->ParseDisplayName(NULL, NULL, const_cast<LPWSTR>(i->c_str()), &nEaten, &cAF, &nAttribs)))
		{
			CPIDL folder(cAF.GetParent());
			bool unique = true;
			for (std::vector<CPIDL>::const_iterator i = m_cAutoFolders.begin(); unique && i != m_cAutoFolders.end(); ++i)
				if (folder == *i || ((folder == m_cDesktopDir || folder.GetIDCount() == 0) && *i == m_cDesktop))
					unique = false;
			for (std::vector<CPIDL>::const_iterator i = m_cFavFolders.begin(); unique && i != m_cFavFolders.end(); ++i)
				if (folder == *i || ((folder == m_cDesktopDir || folder.GetIDCount() == 0) && *i == m_cDesktop))
					unique = false;
			if (unique)
			{
				if (sep)
				{
					m_wndFavFolders.AddButton(0, TBSTYLE_SEP, 0, 0, LPCTSTR(0), 0);
					sep = false;
				}
				SHFILEINFO sfi;
				ZeroMemory(&sfi,sizeof(sfi));
				SHGetFileInfo((LPCTSTR)folder.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_DISPLAYNAME);
				bool bMatch = folder == m_cFolder ||
					((m_cFolder == m_cDesktopDir || m_cFolder.GetIDCount() == 0) && folder == m_cDesktop); // desktop is special case
				TBBUTTON const tButton =
					{ sfi.iIcon, IDC_FWFILE_FAVFOLDER1+m_cFavFolders.size()+m_cAutoFolders.size(), bMatch ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED, BTNS_BUTTON|BTNS_NOPREFIX|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(sfi.szDisplayName)};
				m_wndFavFolders.AddButtons(1, const_cast<LPTBBUTTON>(&tButton));
				m_cAutoFolders.push_back(folder);
			}
		}

	}
}
void CStorageFilterWindowFileSystem::AddFavFolder(CPIDL const& a_cPIDL)
{
	SHFILEINFO sfi;
	ZeroMemory(&sfi,sizeof(sfi));
	//sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER;
	SHGetFileInfo((LPCTSTR)a_cPIDL.operator const ITEMIDLIST*(), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_DISPLAYNAME);
	TBBUTTON const tButton =
		{ sfi.iIcon, IDC_FWFILE_FAVFOLDER1+m_cFavFolders.size(), TBSTATE_ENABLED, BTNS_BUTTON|BTNS_NOPREFIX, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(sfi.szDisplayName)};
	m_wndFavFolders.AddButtons(1, const_cast<LPTBBUTTON>(&tButton));
	m_cFavFolders.push_back(a_cPIDL);
}

LRESULT CStorageFilterWindowFileSystem::OnFavoriteFolder(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	size_t i = a_wID-IDC_FWFILE_FAVFOLDER1;
	std::vector<CPIDL>::const_iterator pF = m_cFavFolders.end();
	bool ok = false;
	if (i < m_cFavFolders.size())
	{
		pF = m_cFavFolders.begin()+i;
		ok = true;
	}
	else if (i < m_cFavFolders.size()+m_cAutoFolders.size())
	{
		pF = m_cAutoFolders.begin()+(i-m_cFavFolders.size());
		ok = true;
	}
	if (ok)
	{
		if (pF->GetIDCount() && *pF != m_cDesktopDir && *pF != m_cDesktop)
		{
			CComPtr<IShellFolder> pFolder;
			m_pDesktopFolder->BindToObject(*pF, NULL, IID_IShellFolder, (void**)&pFolder);
			if (pFolder)
			{
				m_cFolder = *pF;
				ShowFolder(pFolder);
			}
		}
		else
		{
			m_cFolder = CPIDL();
			ShowFolder(m_pDesktopFolder);
		}
	}
	return 0;
}

LRESULT CStorageFilterWindowFileSystem::OnFoldersCustomDraw(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	NMTBCUSTOMDRAW* pCustDraw = reinterpret_cast<NMTBCUSTOMDRAW*>(a_pNMHdr);
	if (pCustDraw->nmcd.dwDrawStage == CDDS_PREERASE)
	{
		RECT rc = pCustDraw->nmcd.rc;
		if (rc.left == rc.right || rc.top == rc.bottom)
			m_wndFavFolders.GetClientRect(&rc);
		FillRect(pCustDraw->nmcd.hdc, &rc, m_hFavFoldersBackground);
		return CDRF_SKIPDEFAULT;
	}
	return CDRF_DODEFAULT;
}

void CStorageFilterWindowFileSystem::LookInClear()
{
	int nCount = m_wndFolder.GetCount();
	int i;
	for (i = 0; i < nCount; i++)
	{
		delete reinterpret_cast<CPIDL*>(m_wndFolder.GetItemDataPtr(i));
	}
	m_wndFolder.ResetContent();
}

void CStorageFilterWindowFileSystem::LookInInit(const CPIDL& a_cActual)
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

LRESULT CStorageFilterWindowFileSystem::OnFolderDropDown(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
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

int CStorageFilterWindowFileSystem::LookInInsert(const CPIDL& a_cRoot, const CPIDL& a_cActual, int a_iIndent, int a_iItem)
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

LRESULT CStorageFilterWindowFileSystem::OnGetIShellBrowser(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
{
	//AddRef(); //not addrefed
	SetWindowLongPtr(DWLP_MSGRESULT, (LONG_PTR)(IShellBrowser*)this); //use this if dialog
	return (LRESULT)(IShellBrowser*)this;
}


LRESULT CStorageFilterWindowFileSystem::OnFolderBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT CStorageFilterWindowFileSystem::OnFolderAddOrRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// favorite folders
	for (std::vector<CPIDL>::iterator i = m_cFavFolders.begin(); i != m_cFavFolders.end(); ++i)
	{
		if (*i == m_cFolder || ((m_cFolder == m_cDesktopDir || m_cFolder.GetIDCount() == 0) && *i == m_cDesktop))
		{
			TBBUTTONINFO tInfo;
			ZeroMemory(&tInfo, sizeof tInfo);
			tInfo.cbSize = sizeof tInfo;

			// remove it
			size_t pos = i-m_cFavFolders.begin();
			m_wndFavFolders.DeleteButton(pos);
			for (size_t j = pos; j < size_t(m_wndFavFolders.GetButtonCount()); ++j)
			{
				tInfo.dwMask = TBIF_COMMAND;
				tInfo.idCommand = IDC_FWFILE_FAVFOLDER1+j;
				m_wndFavFolders.SetButtonInfo(IDC_FWFILE_FAVFOLDER1+j+1, &tInfo);
			}
			m_cFavFolders.erase(i);

			CComBSTR bstrFileNewFolder;
			CMultiLanguageString::GetLocalized(L"[0409]Add folder to favorites[0405]Přidat složku k oblíbeným", m_tLocaleID, &bstrFileNewFolder);
			tInfo.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tInfo.iImage = 2;
			tInfo.pszText = bstrFileNewFolder;
			m_wndToolBar.SetButtonInfo(ID_FWFILE_NEWFOLDER, &tInfo);
			m_wndToolBar.EnableButton(ID_FWFILE_NEWFOLDER, TRUE);

			std::wstring::size_type pos1 = 0;
			for (size_t j = 0; pos1 != std::wstring::npos && j < pos; ++j)
				pos1 = m_strFavFolders.find(L'|', pos1+1);
			if (pos1 != std::wstring::npos)
			{
				std::wstring::size_type pos2 = m_strFavFolders.find(L'|', pos1+1);
				m_strFavFolders.erase(pos1, pos2 != std::wstring::npos ? pos2-pos1 : m_strFavFolders.size()-pos1);
				SetConfigValue(CComBSTR(CFGID_FS_FAVFOLDERS), CConfigValue(m_strFavFolders.c_str()));
			}
			UpdateAutoFolders();
			return 0;
		}
	}

	// add 
	size_t count = m_wndFavFolders.GetButtonCount();
	while (count-- > m_cFavFolders.size())
		m_wndFavFolders.DeleteButton(count);
	m_cAutoFolders.clear();
	AddFavFolder(m_cFolder);
	m_wndFavFolders.CheckButton(IDC_FWFILE_FAVFOLDER1+m_cFavFolders.size()-1, TRUE);
	UpdateAutoFolders();

	CComBSTR bstrFileRemoveFolder;
	CMultiLanguageString::GetLocalized(L"[0409]Remove folder from favorites[0405]Odstranit složku z oblíbených", m_tLocaleID, &bstrFileRemoveFolder);
	TBBUTTONINFO tInfo;
	ZeroMemory(&tInfo, sizeof tInfo);
	tInfo.cbSize = sizeof tInfo;
	tInfo.dwMask = TBIF_IMAGE | TBIF_TEXT;
	tInfo.iImage = 4;
	tInfo.pszText = bstrFileRemoveFolder;
	m_wndToolBar.SetButtonInfo(ID_FWFILE_NEWFOLDER, &tInfo);
	m_wndToolBar.EnableButton(ID_FWFILE_NEWFOLDER, TRUE);

	STRRET str;
	if (m_cFolder.GetIDCount())
	{
		m_pDesktopFolder->GetDisplayNameOf(m_cFolder, SHGDN_FORPARSING, &str);
	}
	else
	{
		CPIDL cFolder;
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &cFolder);
		m_pDesktopFolder->GetDisplayNameOf(m_cFolder, SHGDN_FORPARSING, &str);
	}
	LPWSTR pFld = MyStrRetToWStr(m_pShellAlloc, m_cFolder, str);
	if (pFld)
	{
		m_strFavFolders += L'|';
		CComBSTR bstr(pFld);
		PortablePath::Full2Portable(&(bstr.m_str));
		m_strFavFolders += bstr.m_str;
		SetConfigValue(CComBSTR(CFGID_FS_FAVFOLDERS), CConfigValue(m_strFavFolders.c_str()));
	}
	delete[] pFld;

	return 0;
}


LRESULT CStorageFilterWindowFileSystem::OnFolderUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT CStorageFilterWindowFileSystem::OnViewTypeMenu(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
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
			{FVM_LIST, L"[0409]List[0405]Seznam", EVMBSXP | EVMBSOld},
			{FVM_DETAILS, L"[0409]Details[0405]Podrobnosti", EVMBSXP | EVMBSOld},
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

LRESULT CStorageFilterWindowFileSystem::OnViewTypeChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
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

LRESULT CStorageFilterWindowFileSystem::OnFilterChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	int nCurFilter = m_wndFilter.GetCurSel();
	if (nCurFilter < 0 || nCurFilter >= m_wndFilter.GetCount())
		return 0;
	IDocumentType* pActiveDocType  = reinterpret_cast<IDocumentType*>(m_wndFilter.GetItemDataPtr(nCurFilter));
	if (m_pActiveDocType == pActiveDocType)
		return 0;
	if (m_pActiveDocType && pActiveDocType && (m_dwFlags&(EFTOpenExisting|EFTCreateNew)) == EFTCreateNew)
	{
		CComPtr<IEnumStrings> pDefExts;
		m_pActiveDocType->SupportedExtensionsGet(&pDefExts);
		TCHAR szName[MAX_PATH] = _T("");
		m_wndName.GetWindowText(szName, itemsof(szName));
		size_t nNameLen = _tcslen(szName);
		bool bMatch = false;
		size_t nExtLen = 0;
		ULONG i = 0;
		if (pDefExts)
		{
			CComBSTR bstrDefExt;
			for (; !bMatch && SUCCEEDED(pDefExts->Get(i, &bstrDefExt)); ++i)
			{
				if (bstrDefExt)
				{
					COLE2CT strDefExt(bstrDefExt);
					nExtLen = _tcslen(strDefExt);
					bMatch = nNameLen > nExtLen && szName[nNameLen-nExtLen-1] == _T('.') && 0 == _tcsicmp(strDefExt, szName+nNameLen-nExtLen);
					bstrDefExt.Empty();
				}
			}
		}
		if (!pDefExts || i == 0)
		{
			nExtLen = -1;
			bMatch = _tcsrchr(szName, _T('.')) == NULL;
		}
		if (bMatch)
		{
			CComBSTR bstrDefExt2;
			pActiveDocType->DefaultExtensionGet(&bstrDefExt2);
			COLE2CT strDefExt2(bstrDefExt2);
			if (bstrDefExt2)
			{
				szName[nNameLen-nExtLen-1] = _T('.');
				_tcscpy(szName+nNameLen-nExtLen, strDefExt2);
			}
			else
			{
				szName[nNameLen-nExtLen-1] = _T('\0');
			}
			m_wndName.SetWindowText(szName);
		}
	}

	m_pActiveDocType = pActiveDocType;
	NotifyListenerDocumentChange();

	if (m_pActiveDocType != NULL)
	{
		CComBSTR bstrUID;
		m_pActiveDocType->UniqueIDGet(&bstrUID);
		SetConfigValue(CComBSTR(CFGID_FS_LASTFILTER), CConfigValue(bstrUID));
	}

	m_pActiveShellView->Refresh();

	return 0;
}

LRESULT CStorageFilterWindowFileSystem::OnFolderChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
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

void CStorageFilterWindowFileSystem::GetFolder(tstring& a_strOut) const
{
	STRRET str;
	m_pDesktopFolder->GetDisplayNameOf(m_cFolder, SHGDN_FORPARSING, &str);
	LPTSTR pFld = MyStrRetToStr(m_pShellAlloc, NULL, str);

	if (pFld != NULL)
		a_strOut = pFld;

	delete[] pFld;
}

LPARAM CStorageFilterWindowFileSystem::GetFilter(tstring& a_strOut, bool a_bNoBeep)
{
	TCHAR szName[MAX_PATH] = _T("");
	m_wndName.GetWindowText(szName, itemsof(szName));

	// EFTOpenExisting or EFTCreateNew but not EFTHintNoStream (folder is not valid)
	// text in edit box could be :
	// - existing relative path
	//   existing semi-relative path (beginning with \)
	//   existing absolute path
	//     -> change active folder, clear name textbox
	// - existing relative filename
	//   existing semi-relative filename (beginning with \)
	//   existing absolute filename
	//     -> return full path
	// - custom filter
	//     -> activate custom filter, clear name textbox
	// - existing path + non-existing filename
	//     -> EFTOpenExisting -> beep 
	//     -> EFTCreateNew -> return full path
	// - another combination
	//     -> beep

	std::tstring strDefExt;
	CComBSTR bstrDefExt;
	if (m_pActiveDocType != NULL && (m_dwFlags&(EFTOpenExisting|EFTCreateNew)) == EFTCreateNew)
		m_pActiveDocType->DefaultExtensionGet(&bstrDefExt);
	if (bstrDefExt != NULL && bstrDefExt[0] != L'\0')
	{
		strDefExt = CW2CT(bstrDefExt);
	}
	std::tstring strLocator(szName);
	switch (AnalyzePath(m_pShellAlloc, m_pDesktopFolder, m_cFolder, strLocator, strDefExt.empty() ? NULL : strDefExt.c_str()))
	{
	case EPARExistingFile:
		a_strOut = strLocator;
		return (m_dwFlags&EFTCreateNew) == EFTCreateNew ? 2 : 1;
	case EPARExistingFolder:
		{
			CPIDL cFolder;
			ULONG nEaten = 0;
			ULONG nAttribs = SFGAO_FOLDER;
			if (SUCCEEDED(m_pDesktopFolder->ParseDisplayName(NULL, NULL, CT2W(strLocator.c_str()), &nEaten, &cFolder, &nAttribs)))
			{
				CComPtr<IShellFolder> pFolder;
				m_pDesktopFolder->BindToObject(cFolder, NULL, IID_IShellFolder, (void**)&pFolder);
				if (pFolder != NULL)
				{
//					ReplyMessage(0);
					m_cFolder = cFolder;
					ShowFolder(pFolder);
					m_wndName.SetWindowText(_T(""));
					return 0;
				}
			}
		}
		if (!a_bNoBeep)
			MessageBeep(MB_ICONHAND);
		return 0;
	case EPARNewFileInExistingFolder:
		if ((m_dwFlags&EFTOpenExisting) == 0)
		{
			a_strOut = strLocator;
			return 1;
		}
		else
		{
			if (!a_bNoBeep)
				MessageBeep(MB_ICONHAND);
			return 0;
		}
	case EPARNonExistingPath:
		if (!a_bNoBeep)
			MessageBeep(MB_ICONHAND);
		return 0;
	case EPARCustomFilter:
		{
			CComObject<CDocumentTypeWildcards>* p = NULL;
			CComObject<CDocumentTypeWildcards>::CreateInstance(&p);
			if (p != NULL)
			{
				p->Init(CMultiLanguageString::GetAuto(L"[0409]Custom Filter[0405]Vlastní filter"), CComBSTR(strLocator.c_str()));
				m_pActiveDocType = p;
				m_pActiveShellView->Refresh();
				m_wndFilter.SetWindowText(_T(""));
			}
		}
		return 0;
	case EPARParsingError:
		if (!a_bNoBeep)
			MessageBeep(MB_ICONHAND);
		return 0;
	default:
		return 0;
	}
}

