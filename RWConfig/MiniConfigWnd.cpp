// MiniConfigWnd.cpp : Implementation of CMiniConfigWnd

#include "stdafx.h"
#include "MiniConfigWnd.h"

#include "ConfigItemCheckBox.h"
#include "ConfigItemComboBox.h"
#include "ConfigItemEditBox.h"
#include "ConfigItemEditBoxNumber.h"
#include "ConfigItemSlider.h"
#include "ConfigItemEditBoxFloat.h"
#include <htmlhelp.h>
#pragma comment(lib, "htmlhelp.lib")


// CMiniConfigWnd

STDMETHODIMP CMiniConfigWnd::TopWindowSet(BOOL a_bIsTopWindow, DWORD a_clrBackground)
{
	return S_FALSE;
}

STDMETHODIMP CMiniConfigWnd::Create(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, EConfigWindowBorderMode a_eBorderMode)
{
	m_tLocaleID = a_tLocaleID;
	static INITCOMMONCONTROLSEX tICCEx = { sizeof(INITCOMMONCONTROLSEX), ICC_USEREX_CLASSES|ICC_STANDARD_CLASSES };
	static BOOL bDummy = InitCommonControlsEx(&tICCEx);
	Win32LangEx::CLangDialogImpl<CMiniConfigWnd>::Create(reinterpret_cast<HWND>(a_hParent));
	SetWindowLong(GWL_ID, a_nCtlID);
	RECT rc;
	GetWindowRect(&rc);
	rc.bottom = a_prcPositon->top + rc.bottom-rc.top;
	rc.top = a_prcPositon->top;
	rc.left = a_prcPositon->left;
	rc.right = a_prcPositon->right;
	MoveWindow(&rc);
	RefreshItemIDs();
	ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
	return S_OK;
}

STDMETHODIMP CMiniConfigWnd::ConfigSet(IConfig* a_pConfig, EConfigPanelMode a_eMode)
{
	if (m_pConfig != a_pConfig)
	{
		if (m_pConfig != NULL)
		{
			m_pConfig->ObserverDel(ObserverGet(), 0);
		}
		m_pConfig = a_pConfig;
		RefreshItemIDs();
		if (a_pConfig != NULL)
		{
			m_pConfig->ObserverIns(ObserverGet(), 0);
		}
	}
	return S_OK;
}

void CMiniConfigWnd::OwnerNotify(TCookie a_tCookie, IUnknown* a_pIDs)
{
	RefreshItemIDs();
}

LRESULT CMiniConfigWnd::OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_wndItemIDs = GetDlgItem(IDC_MCF_ITEMID);
	RECT rc;
	m_wndItemIDs.GetWindowRect(&rc);
	m_nXMaxComboSize = rc.right-rc.left;
	rc.left = 0;
	rc.right = 4;
	rc.top = 0;
	rc.bottom = 0;
	MapDialogRect(&rc);
	m_nXGap = rc.right;
	AddRef();
	return 0;
}

void CMiniConfigWnd::OnFinalMessage(HWND a_hWnd)
{
	Release();
}

LRESULT CMiniConfigWnd::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pValueCtl)
	{
		m_pValueCtl->Delete();
		m_pValueCtl = NULL;
	}

	a_bHandled = FALSE;
	return 0;
}

LRESULT CMiniConfigWnd::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	RECT rcCombo = rc;
	RECT rcControl = rc;
	GetControlsWidths(rc.right, &rcCombo.left, &rcCombo.right, &rcControl.left, &rcControl.right);
	m_wndItemIDs.MoveWindow(&rcCombo);
	if (m_pValueCtl)
		m_pValueCtl->PositionSet(rcControl, false);
	return 0;
}

LRESULT CMiniConfigWnd::OnItemIDSelchange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	if (m_pValueCtl != NULL)
	{
		m_pValueCtl->Delete();
		m_pValueCtl = NULL;
	}
	m_bstrSelected.Empty();
	if (m_pItemIDs != NULL)
	{
		m_pItemIDs->Get(m_wndItemIDs.GetCurSel(), &m_bstrSelected);
	}

	RefreshValue();

	return 0;
}

LRESULT CMiniConfigWnd::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		if (pHelpInfo->iCtrlId == IDC_MCF_ITEMID)
		{
			TCHAR szBuffer[1024] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_HELP_MCF_ITEMID, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
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
			HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
			return 0;
		}
		else if (pHelpInfo->iCtrlId >= (IDC_MCF_ITEMID+1) && pHelpInfo->iCtrlId <= (IDC_MCF_ITEMID+16))
		{
			CComPtr<IConfigItem> pUIInfo;
			m_pConfig->ItemGetUIInfo(m_bstrSelected, __uuidof(IConfigItem), reinterpret_cast<void**>(&pUIInfo));
			CComPtr<ILocalizedString> pHelpText;
			if (pUIInfo != NULL)
				pUIInfo->NameGet(NULL, &pHelpText);
			CComBSTR bstrHelp;
			if (pHelpText != NULL)
				pHelpText->GetLocalized(m_tLocaleID, &bstrHelp);
			if (bstrHelp.Length() != 0)
			{
				RECT rcItem;
				::GetWindowRect(static_cast<HWND>(pHelpInfo->hItemHandle), &rcItem);
				HH_POPUP hhp;
				hhp.cbStruct = sizeof(hhp);
				hhp.hinst = _pModule->get_m_hInst();
				hhp.idString = 0;
				COLE2CT cText(bstrHelp);
				hhp.pszText = cText;
				hhp.pt.x = rcItem.right;
				hhp.pt.y = rcItem.bottom;
				hhp.clrForeground = 0xffffffff;
				hhp.clrBackground = 0xffffffff;
				hhp.rcMargins.left = -1;
				hhp.rcMargins.top = -1;
				hhp.rcMargins.right = -1;
				hhp.rcMargins.bottom = -1;
				hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
				HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
				return 0;
			}
		}
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CMiniConfigWnd::ReflectEvent(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_pValueCtl)
	{
		m_pValueCtl->EventReflected(wNotifyCode);
	}
	return 0;
}

LRESULT CMiniConfigWnd::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pValueCtl)
	{
		m_pValueCtl->ScrollReflected(LOWORD(wParam), HIWORD(wParam));
	}
	return 0;
}


bool CMiniConfigWnd::RefreshItemIDs()
{
	if (!IsWindow() || !m_wndItemIDs.IsWindow())
		return false;

	m_wndItemIDs.ResetContent();

	if (m_pConfig == NULL)
	{
		m_wndItemIDs.EnableWindow(FALSE);
		if (m_pValueCtl != NULL)
		{
			m_pValueCtl->Delete();
			m_pValueCtl = NULL;
			m_bstrSelected.Empty();
		}
		return false;
	}

	m_pItemIDs = NULL;
	m_pConfig->ItemIDsEnum(&m_pItemIDs);

	ULONG nItems = 0;
	if (m_pItemIDs == NULL || FAILED(m_pItemIDs->Size(&nItems)) || nItems == 0)
	{
		m_wndItemIDs.EnableWindow(FALSE);
		if (m_pValueCtl != NULL)
		{
			m_pValueCtl->Delete();
			m_pValueCtl = NULL;
			m_bstrSelected.Empty();
		}
		return false;
	}

	ULONG nSel = 0xffffffff;

	CComBSTR bstr = NULL;
	ULONG i;
	for (i = 0; SUCCEEDED(m_pItemIDs->Get(i, &bstr)); i++)
	{
		if (m_bstrSelected == bstr)
		{
			nSel = i;
		}
		COMBOBOXEXITEM tItem;
		tItem.mask = CBEIF_TEXT | CBEIF_INDENT;
		tItem.iItem = i;
		tItem.iIndent = 0;

		OLECHAR const* pStr = bstr;
		OLECHAR const* pName = bstr;
		while (*pStr)
		{
			if (*pStr == L'\\')
			{
				tItem.iIndent++;
				pName = pStr+1;
			}
			pStr++;
		}

		CComPtr<IConfigItem> pItem;
		m_pConfig->ItemGetUIInfo(bstr, __uuidof(IConfigItem), reinterpret_cast<void**>(&pItem));
		CComPtr<ILocalizedString> pLocName;
		if (pItem != NULL)
		{
			pItem->NameGet(&pLocName, NULL);
		}
		CComBSTR bstrName;
		if (pLocName != NULL)
		{
			pLocName->GetLocalized(m_tLocaleID, &bstrName);
		}
		COLE2CT strName(bstrName != NULL ? bstrName : pName);
		tItem.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(strName));

		m_wndItemIDs.InsertItem(&tItem);
		bstr.Empty();
	}

	if (i == 0)
	{
		m_wndItemIDs.EnableWindow(FALSE);
		if (m_pValueCtl != NULL)
		{
			m_pValueCtl->Delete();
			m_pValueCtl = NULL;
			m_bstrSelected.Empty();
		}
		return true;
	}

	if (nSel == 0xffffffff)
	{
		// previously selected item is not available
		// TODO: try to select item parent
		m_bstrSelected.Empty();
		m_pItemIDs->Get(0, &m_bstrSelected);
		m_wndItemIDs.SetCurSel(0);
		if (m_pValueCtl != NULL)
		{
			m_pValueCtl->Delete();
			m_pValueCtl = NULL;
		}
	}
	else
	{
		m_wndItemIDs.SetCurSel(nSel);
	}

	RefreshValue();
	m_wndItemIDs.EnableWindow();

	return true;
}

void CMiniConfigWnd::RefreshValue()
{
	if (m_pValueCtl != NULL)
	{
		if (m_pValueCtl->IsValid())
		{
			m_pValueCtl->Update();
			return;
		}
		m_pValueCtl->Delete();
		m_pValueCtl = NULL;
	}

	if (m_pConfig == NULL || m_bstrSelected.Length() == 0)
		return;

	static FControlCreator * const afControlCreators[] = 
	{
		// controls creators sorted by priority
		CConfigItemCheckBox::CreateControl,
		CConfigItemComboBox::CreateControl,
		CConfigItemSlider::CreateControl,
		CConfigItemEditBox::CreateControl,
		CConfigItemEditBoxNumber::CreateControl,
		CConfigItemEditBoxFloat::CreateControl,
	};

	RECT rcClient;
	GetClientRect(&rcClient);
	RECT rcCombo = rcClient;
	RECT rcVal = rcClient;
	GetControlsWidths(rcClient.right, &rcCombo.left, &rcCombo.right, &rcVal.left, &rcVal.right);

	size_t i;
	for (i = 0; m_pValueCtl == NULL && i < itemsof(afControlCreators); i++)
	{
		m_pValueCtl = afControlCreators[i](this, rcVal, m_bstrSelected, IDC_MCF_ITEMID+1, m_tLocaleID, false);
	}

	if (m_pValueCtl)
	{
		m_pValueCtl->Update();
	}
}

void CMiniConfigWnd::GetControlsWidths(LONG a_nXSize, LONG* a_pComboLeft, LONG* a_pComboRight, LONG* a_pControlLeft, LONG* a_pControlRight) const
{
	*a_pComboLeft = 0;
	*a_pControlRight = a_nXSize;
	if ((a_nXSize-m_nXGap) > (m_nXMaxComboSize<<1))
	{
		*a_pComboRight = m_nXMaxComboSize;
		*a_pControlLeft = m_nXMaxComboSize+m_nXGap;
	}
	else
	{
		*a_pComboRight = (a_nXSize-m_nXGap)>>1;
		*a_pControlLeft = m_nXGap+*a_pComboRight;
	}
}

