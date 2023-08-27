
#pragma once

#include <htmlhelp.h>
#include <Win32LangEx.h>
#include <ApplicationFileName.h>

enum EHelpLocation
{
	EHLAbsolute = 0,
	EHLExeDir,
	EHLDllDir,
	EHLOnline
};
inline LPCTSTR GetHelpFilePath(LPTSTR a_pszBuffer, size_t a_nBuffer, LCID a_tLocaleID, EHelpLocation a_eLocation = EHLExeDir, LPCTSTR a_pszName = GetAppFileName())
{
	LPTSTR pszName = NULL;
	switch (a_eLocation)
	{
	case EHLExeDir:
		::GetModuleFileName(NULL, a_pszBuffer, a_nBuffer);
		pszName = _tcsrchr(a_pszBuffer, _T('\\'))+1;
		_tcscpy(pszName, a_pszName);
		pszName += _tcslen(a_pszName);
		break;
	case EHLDllDir:
		::GetModuleFileName(_pModule->get_m_hInst(), a_pszBuffer, a_nBuffer);
		pszName = _tcsrchr(a_pszBuffer, _T('\\'))+1;
		_tcscpy(pszName, a_pszName);
		pszName += _tcslen(a_pszName);
		break;
	default:
		_tcscpy(a_pszBuffer, a_pszName);
		pszName = a_pszBuffer+_tcslen(a_pszName);
		break;
	}
	_stprintf(pszName, _T("%04x.chm"), LANGIDFROMLCID(a_tLocaleID));
	HANDLE hFile = ::CreateFile(a_pszBuffer, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		_tcscpy(pszName, _T(".chm"));
	}
	else
	{
		CloseHandle(hFile);
	}
	return a_pszBuffer;
}

struct SCtxHelpMap
{
	UINT nControlID;
	UINT nHelpStringID;
	LPCTSTR pszHelpString;
	bool bNoToolTips;
};

#define BEGIN_CTXHELP_MAP(thisClass) \
	static const SCtxHelpMap* GetCtxHelpMap() \
	{ \
		static const SCtxHelpMap theMap[] = \
		{

#define END_CTXHELP_MAP() \
			{ 0xffffffff, 0, NULL, false }, \
		}; \
		return theMap; \
	}

#define CTXHELP_CONTROL_RESOURCE(id, ids) \
		{ id, ids, NULL, false },
#define CTXHELP_CONTROL_RESOURCE_NOTIP(id, ids) \
		{ id, ids, NULL, true },
#define CTXHELP_CONTROL_CUSTOM(id) \
		{ id, 0, NULL, false },
#ifdef MULTIMANGUAGE_STRING_INCLUDED
#define CTXHELP_CONTROL_STRING(id, str) \
		{ id, 0, str, false },
#define CTXHELP_CONTROL_STRING_NOTIP(id, str) \
		{ id, 0, str, true },
#endif

template<class T>
class CContextHelpDlg
{
public:
	CContextHelpDlg() : m_pszHelpName(NULL), m_pszTopic(NULL)
	{
	}
    CContextHelpDlg(LPCTSTR a_pszHelpTopic, LPCTSTR a_pszHelpWindow = NULL, EHelpLocation a_eLocation = EHLExeDir, LPCTSTR a_pszHelpName = GetAppFileName()) :
		m_pszHelpName(NULL), m_pszTopic(NULL), m_eHelpLocation(a_eLocation)
	{
		if (a_pszHelpTopic && _tcsncmp(a_pszHelpTopic, _T("http://"), 7) == 0)
		{
			m_eHelpLocation = EHLOnline;
			m_pszTopic = new TCHAR[_tcslen(a_pszHelpTopic)+1];
			_tcscpy(m_pszTopic, a_pszHelpTopic);
			m_pszHelpName = NULL;
		}
		else
		{
			if (a_pszHelpName)
			{
				m_pszHelpName = new TCHAR[_tcslen(a_pszHelpName)+1];
				_tcscpy(m_pszHelpName, a_pszHelpName);
			}
			m_pszTopic = new TCHAR[_tcslen(a_pszHelpTopic)+4+(a_pszHelpWindow ? _tcslen(a_pszHelpWindow)+1 : 0)];
			_tcscpy(m_pszTopic, _T("::/"));
			_tcscat(m_pszTopic, a_pszHelpTopic);
			if (a_pszHelpWindow)
			{
				_tcscat(m_pszTopic, _T(">"));
				_tcscat(m_pszTopic, a_pszHelpWindow);
			}
		}
	}
	~CContextHelpDlg()
	{
		delete[] m_pszHelpName;
		delete[] m_pszTopic;
	}

	BEGIN_MSG_MAP(CContextHelpDlg)
		MESSAGE_HANDLER(WM_HELP, OnHelp)
		COMMAND_HANDLER(IDHELP, BN_CLICKED, OnClickedHelp)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, OnGetDispInfo)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		bool bContxtTips = true;
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr)
		{
			CComPtr<IConfig> pCfg;
			// hacks: copied CLSID and CFGVAL
			static CLSID const tID = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}}; // CLSID_GlobalConfigMainFrame
			pMgr->Config(tID, &pCfg);
			CConfigValue cVal;
			if (pCfg) pCfg->ItemValueGet(CComBSTR(L"CtxHelpTips"), &cVal);
			if (cVal.TypeGet() == ECVTBool) bContxtTips = cVal;
		}
		if (!bContxtTips)
			return 0;

		for (SCtxHelpMap const* pMap = static_cast<T const*>(this)->GetCtxHelpMap(); pMap->nControlID != 0xffffffff; ++pMap)
		{
			if (pMap->bNoToolTips)
				continue;
			HWND hCtl = ::GetDlgItem(static_cast<T const*>(this)->m_hWnd, pMap->nControlID);
			if (hCtl == NULL)
				continue;
			TCHAR szClass[128] = _T("");
			::GetClassName(hCtl, szClass, itemsof(szClass));
			if (_tcscmp(szClass, WC_COMBOBOXEX) == 0)
			{
				hCtl = reinterpret_cast<HWND>(::SendMessage(hCtl, CBEM_GETCOMBOCONTROL, 0, 0));
				if (hCtl == NULL)
					continue;
			}
			//if (_tcscmp(szClass, TOOLBARCLASSNAME) == 0)
			//	continue; // no tooltip for toolbars (they have their own)
			if (m_wndToolTip.m_hWnd == NULL)
			{
				m_wndToolTip.Create(static_cast<T const*>(this)->m_hWnd);
				HDC hDC = ::GetDC(static_cast<T const*>(this)->m_hWnd);
				int nWidth = 420 * GetDeviceCaps(hDC, LOGPIXELSX) / 96;
				::ReleaseDC(static_cast<T const*>(this)->m_hWnd, hDC);
				m_wndToolTip.SetMaxTipWidth(nWidth);
			}
			TOOLINFO tTI;
			ZeroMemory(&tTI, sizeof tTI);
			tTI.cbSize = TTTOOLINFOA_V2_SIZE;
			tTI.hwnd = static_cast<T const*>(this)->m_hWnd;
			tTI.uId = reinterpret_cast<UINT_PTR>(hCtl);
			tTI.uFlags = TTF_PARSELINKS|TTF_SUBCLASS|TTF_IDISHWND;
			tTI.lpszText = LPSTR_TEXTCALLBACK;
			tTI.lParam = reinterpret_cast<LPARAM>(pMap);
			m_wndToolTip.AddTool(&tTI);
		}
		return 0;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		if (m_wndToolTip.IsWindow())
			m_wndToolTip.DestroyWindow();
		return 0;
	}
	void GetControlToolTip(SCtxHelpMap const* a_pItem, ULONG a_nBuffer, LPTSTR a_pszBuffer) const
	{
#ifdef MULTIMANGUAGE_STRING_INCLUDED
		if (a_pItem->pszHelpString)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(a_pItem->pszHelpString, static_cast<T const*>(this)->m_tLocaleID, &bstr);
			int n = min(bstr.Length(), a_nBuffer-1);
			wcsncpy(a_pszBuffer, bstr, min(bstr.Length(), a_nBuffer-1));
			a_pszBuffer[n] = _T('\0');
			return;
		}
#else
		ATLASSERT(a_pItem->pszHelpString == NULL); // #include <MultiLanguageString.h>
#endif
		Win32LangEx::LoadString(_pModule->get_m_hInst(), a_pItem->nHelpStringID, a_pszBuffer, a_nBuffer, LANGIDFROMLCID(static_cast<T const*>(this)->m_tLocaleID));
	}
	LRESULT OnGetDispInfo(int UNREF(a_nCtrlID), LPNMHDR a_pNMHeader, BOOL& a_bHandled)
	{
		LPNMTTDISPINFO pInfo = reinterpret_cast<LPNMTTDISPINFO>(a_pNMHeader);
		SCtxHelpMap const* pItem = reinterpret_cast<SCtxHelpMap const*>(pInfo->lParam);
		if (a_pNMHeader->hwndFrom != m_wndToolTip || pItem == NULL)
		{
			a_bHandled = FALSE;
			return 0;
		}
		TCHAR szBuffer[1024] = _T("");
		static_cast<T const*>(this)->GetControlToolTip(pItem, ULONG(itemsof(szBuffer)), szBuffer);
		pInfo->hinst = NULL;
		pInfo->szText[0] = _T('\0');
		if (_tcslen(szBuffer) >= itemsof(pInfo->szText))
		{
			if (m_pszBuffer.m_p == NULL)
				m_pszBuffer.Allocate(1024);
			_tcscpy(m_pszBuffer.m_p, szBuffer);
			pInfo->lpszText = m_pszBuffer.m_p;
		}
		else
		{
			_tcscpy(pInfo->szText, szBuffer);
		}
		return 0;
	}

	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		if (pHelpInfo->iContextType == HELPINFO_WINDOW && pHelpInfo->iCtrlId >= 0)
		{
			SCtxHelpMap const* pMap;
			for (pMap = static_cast<T const*>(this)->GetCtxHelpMap(); pMap->nControlID != 0xffffffff; pMap++)
			{
				if (pMap->nControlID == static_cast<UINT>(pHelpInfo->iCtrlId))
				{
					TCHAR szBuffer[1024] = _T("");
					static_cast<T const*>(this)->GetControlToolTip(pMap, itemsof(szBuffer), szBuffer);
					RECT rcItem;
					::GetWindowRect(static_cast<HWND>(pHelpInfo->hItemHandle), &rcItem);
					HH_POPUP hhp;
					hhp.cbStruct = sizeof(hhp);
					hhp.hinst = _pModule->get_m_hInst();
					hhp.idString = 0;
					hhp.pszText = szBuffer;
					hhp.pt.x = rcItem.right;
					hhp.pt.y = rcItem.bottom;
					hhp.clrForeground = 0xffffffff;
					hhp.clrBackground = 0xffffffff;
					hhp.rcMargins.left = -1;
					hhp.rcMargins.top = -1;
					hhp.rcMargins.right = -1;
					hhp.rcMargins.bottom = -1;
					hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
					HtmlHelp(static_cast<T const*>(this)->m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD_PTR>(&hhp));
					return 0;
				}
			}
		}
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnClickedHelp(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& a_bHandled)
	{
		if (m_eHelpLocation == EHLOnline)
		{
			ShellExecute(NULL, _T("open"), m_pszTopic, NULL, NULL, SW_SHOW);
		}
		else if (m_pszTopic)
		{
			TCHAR szHelpPath[MAX_PATH*2] = _T("");
			GetHelpFilePath(szHelpPath, MAX_PATH, static_cast<T const*>(this)->m_tLocaleID, m_eHelpLocation, m_pszHelpName);
			_tcscat(szHelpPath, m_pszTopic);
			HtmlHelp(static_cast<T const*>(this)->m_hWnd, szHelpPath, HH_DISPLAY_TOPIC, 0);
		}
		else
		{
			a_bHandled = FALSE;
		}
		return 0;
	}

private:
	EHelpLocation m_eHelpLocation;
	LPTSTR m_pszHelpName;
	LPTSTR m_pszTopic;
	CToolTipCtrl m_wndToolTip;
	CAutoVectorPtr<TCHAR> m_pszBuffer; // stupid MS tooltip callbacks
};

#pragma comment(lib, "htmlhelp.lib")