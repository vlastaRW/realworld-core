// DesignerViewWndStart.cpp : Implementation of CDesignerViewWndStart

#include "stdafx.h"
#include "DesignerViewWndStart.h"

#include <XPGUI.h>
#include "RecentFiles.h"
#include "ConfigIDsApp.h"
#include <PlugInCache.h>


// CDesignerViewWndStart

static CLSID g_tLastStartPage = GUID_NULL;
void CDesignerViewWndStart::Init(IConfig* a_pMainCfg, IDocumentControl* a_pDocCtrl, IStatusBarObserver* a_pStatusBar, HWND a_hParentWnd, RECT const* a_prcArea, LCID a_tLocaleID, size_t a_nPages, CLSID const* a_pPages, CLSID const& a_tConfigStartPage, CLSID const& a_tExplicitStartPage, IStorageManager* a_pStorageManager, ILocalizedString* a_pCaption)
{
	m_pMainCfg = a_pMainCfg;
	m_pStatusBar = a_pStatusBar;
	m_pDocCtrl = a_pDocCtrl;
	m_tLocaleID = a_tLocaleID;

	m_timestamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_StartViewPage);

	m_tStartPage = IsEqualGUID(a_tExplicitStartPage, GUID_NULL) ?
		(IsEqualGUID(g_tLastStartPage, GUID_NULL) ? a_tConfigStartPage : g_tLastStartPage) :
		a_tExplicitStartPage;
	m_pStorageManager = a_pStorageManager;
	m_pCaption = a_pCaption;
	bool bFoundStartPage = false;
	CAutoVectorPtr<GUID> aStartPages;
	if (a_nPages == 0 || a_pPages == NULL)
	{
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pGUIDs;
		if (pPIC) pPIC->CLSIDsEnum(CATID_StartViewPage, 0xffffffff, &pGUIDs);
		ULONG nGUIDs = 0;
		if (pGUIDs) pGUIDs->Size(&nGUIDs);
		aStartPages.Allocate(nGUIDs+4);
		aStartPages[0] = __uuidof(StartViewPageFactoryNewDocument);
		aStartPages[1] = __uuidof(StartViewPageFactoryOpenFile);
		aStartPages[2] = __uuidof(StartViewPageFactoryRecentFiles);
		pGUIDs->GetMultiple(0, nGUIDs, aStartPages.m_p+3);
		aStartPages[3+nGUIDs] = __uuidof(StartPageOnline);
		a_pPages = aStartPages;
		a_nPages = nGUIDs+4;
	}
	for (size_t i = 0; i < a_nPages; ++i)
	{
		SStartPageInfo tInfo;
		tInfo.tClsID = a_pPages[i];
		m_aWnds.push_back(tInfo);
		if (!IsEqualGUID(m_tStartPage, GUID_NULL) && IsEqualGUID(m_tStartPage, a_pPages[i]))
			bFoundStartPage = true;
	}
	if (!bFoundStartPage)
	{
		CConfigValue cRecentCount;
		m_pMainCfg->ItemValueGet(CComBSTR(RecentFiles::CFGID_RECENTFILES), &cRecentCount);
		m_tStartPage = cRecentCount.operator LONG() == 0 ? __uuidof(StartViewPageFactoryNewDocument) : __uuidof(StartViewPageFactoryRecentFiles);
	}
	for (size_t i = 0; i < a_nPages; ++i)
		if (!IsEqualGUID(m_tStartPage, GUID_NULL) && IsEqualGUID(m_tStartPage, a_pPages[i]))
			bFoundStartPage = true;
	if (!bFoundStartPage)
		for (size_t i = 0; i < a_nPages; ++i)
			if (!IsEqualGUID(a_pPages[i], GUID_NULL))
				m_tStartPage = a_pPages[i];
	Create(a_hParentWnd);
	MoveWindow(a_prcArea);
	ShowWindow(SW_SHOW);
}

STDMETHODIMP CDesignerViewWndStart::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (m_pStartPage)
	{
		HRESULT hRes = m_pStartPage->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
		if (hRes == S_OK)
			return S_OK;
		if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
			return S_OK;
	}
	return S_FALSE;
}

LRESULT CDesignerViewWndStart::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	iconSize = XPGUI::GetSmallIconSize()*3;
	m_hToolBarImages = ImageList_Create(iconSize, iconSize, XPGUI::GetImageListColorFlags(), 4, 1);
	TCHAR szToolTips[1024] = _T("");
	int iStr = 0;
	TCHAR* pTips = szToolTips;
	CAutoVectorPtr<TBBUTTON> aTBButtons(new TBBUTTON[m_aWnds.size()]);
	for (size_t i = 0; i < m_aWnds.size(); ++i)
	{
		if (!IsEqualGUID(m_aWnds[i].tClsID, GUID_NULL))
		{
			RWCoCreateInstance(m_aWnds[i].pFactory, m_aWnds[i].tClsID);
			if (m_aWnds[i].pFactory == NULL)
			{
				m_aWnds.erase(m_aWnds.begin()+i);
				--i;
			}
			else
			{
				CComPtr<ILocalizedString> pName;
				m_aWnds[i].pFactory->Name(&pName);
				CComBSTR bstrName;
				if (pName)
					pName->GetLocalized(m_tLocaleID, &bstrName);
				if (bstrName)
				{
					_tcscat(pTips, COLE2CT(bstrName));
					pTips += _tcslen(pTips)+1;
				}
				HICON hIcon = NULL;
				m_aWnds[i].pFactory->Icon(iconSize, &hIcon);
				if (hIcon)
				{
					ImageList_AddIcon(m_hToolBarImages, hIcon);
					DestroyIcon(hIcon);
				}
				aTBButtons[i].iBitmap = ImageList_GetImageCount(m_hToolBarImages)-1;
				aTBButtons[i].idCommand = i+ID_PAGE1;
				aTBButtons[i].fsState = IsEqualGUID(m_aWnds[i].tClsID, m_tStartPage) ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED;
				aTBButtons[i].fsStyle = BTNS_CHECKGROUP | BTNS_BUTTON | BTNS_NOPREFIX;
				aTBButtons[i].dwData = 0;
				aTBButtons[i].iString = iStr++;
			}
		}
		else
		{
			m_aWnds.erase(m_aWnds.begin()+i);
			--i;
			//static TBBUTTON const t = {0, ID_PAGE1+100, 0, BTNS_CHECKGROUP | BTNS_SEP, TBBUTTON_PADDING, 0, 0};
			//aTBButtons[i] = t;
		}
	}
	*pTips = _T('\0');

	RECT rcClient;
	GetClientRect(&rcClient);

	RECT rcGaps = {0, 0, 7, 7};
	MapDialogRect(&rcGaps);
	m_nTBGaps = rcGaps.right;

	m_wndToolBar = GetDlgItem(IDC_TOOLBAR);
	m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndToolBar.SetImageList(m_hToolBarImages);
	m_wndToolBar.AddStrings(szToolTips);
	m_wndToolBar.AddButtons(m_aWnds.size(), aTBButtons);
	RECT rect;
	m_wndToolBar.GetWindowRect(&rect);
	ScreenToClient(&rect);
	DWORD dwButtonSize = m_wndToolBar.GetButtonSize();
	m_wndToolBar.SetButtonSize(max(LOWORD(dwButtonSize), HIWORD(dwButtonSize)) + m_nTBGaps+m_nTBGaps, HIWORD(dwButtonSize) + m_nTBGaps);
	rect.right = rect.left + max(LOWORD(dwButtonSize), HIWORD(dwButtonSize)) + m_nTBGaps+m_nTBGaps;
	m_wndToolBar.MoveWindow(&rect);

	m_rcToolBar = rect;
	m_rcToolBar.bottom -= rcClient.bottom;

	ShowHidePages();

	AddRef();

	return 0;
}

LRESULT CDesignerViewWndStart::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pStartPage)
		m_pStartPage->Activate();
	return 0;
}

void CDesignerViewWndStart::OnFinalMessage(HWND a_hWnd)
{
	Release();
}

STDMETHODIMP CDesignerViewWndStart::OnIdle()
{
	ULONG ts = CPlugInEnumerator::GetCategoryTimestamp(CATID_StartViewPage);
	if (m_timestamp != ts)
	{
		m_timestamp = ts;
		CAutoVectorPtr<GUID> aStartPages;
		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pGUIDs;
		if (pPIC) pPIC->CLSIDsEnum(CATID_StartViewPage, 0xffffffff, &pGUIDs);
		ULONG nGUIDs = 0;
		if (pGUIDs) pGUIDs->Size(&nGUIDs);
		aStartPages.Allocate(nGUIDs+4);
		aStartPages[0] = __uuidof(StartViewPageFactoryNewDocument);
		aStartPages[1] = __uuidof(StartViewPageFactoryOpenFile);
		aStartPages[2] = __uuidof(StartViewPageFactoryRecentFiles);
		pGUIDs->GetMultiple(0, nGUIDs, aStartPages.m_p+3);
		aStartPages[3+nGUIDs] = __uuidof(StartPageOnline);
		ULONG nPages = nGUIDs+4;

		bool reactivate = false;
		size_t i;
		for (i = 0; i < nPages; ++i)
		{
			if (IsEqualGUID(aStartPages[i], m_aWnds[i].tClsID))
			{
				m_wndToolBar.SetButtonInfo(i, TBIF_BYINDEX|TBIF_COMMAND, 0, 0, 0, 0, 0, ID_PAGE1+i, 0);
				continue;
			}
			if (i+1 < m_aWnds.size() && IsEqualGUID(aStartPages[i], m_aWnds[i+1].tClsID))
			{
				// remove one
				if (m_pStartPage == m_aWnds[i].pPage) // but it is active
				{
					m_pStartPage->Destroy();
					reactivate = false;
				}
				m_aWnds.erase(m_aWnds.begin()+i);
				m_wndToolBar.DeleteButton(i);
				m_wndToolBar.SetButtonInfo(i, TBIF_BYINDEX|TBIF_COMMAND, 0, 0, 0, 0, 0, ID_PAGE1+i, 0);
			}
			// add one
			SStartPageInfo tInfo;
			tInfo.tClsID = aStartPages[i];
			RWCoCreateInstance(tInfo.pFactory, tInfo.tClsID);
			TBBUTTON btn;
			ZeroMemory(&btn, sizeof btn);
			CComPtr<ILocalizedString> pName;
			tInfo.pFactory->Name(&pName);
			CComBSTR bstrName;
			if (pName)
				pName->GetLocalized(m_tLocaleID, &bstrName);
			HICON hIcon = NULL;
			tInfo.pFactory->Icon(iconSize, &hIcon);
			if (hIcon)
			{
				ImageList_AddIcon(m_hToolBarImages, hIcon);
				DestroyIcon(hIcon);
			}
			btn.iBitmap = ImageList_GetImageCount(m_hToolBarImages)-1;
			btn.idCommand = i+ID_PAGE1;
			btn.fsState = TBSTATE_ENABLED;
			btn.fsStyle = BTNS_CHECKGROUP | BTNS_BUTTON | BTNS_NOPREFIX;
			btn.dwData = 0;
			btn.iString = (INT_PTR)bstrName.m_str;
			m_wndToolBar.InsertButton(i, &btn);
			m_aWnds.insert(m_aWnds.begin()+i, tInfo);
		}
		while (i < m_aWnds.size())
		{
			// remove one
			if (m_pStartPage == m_aWnds[i].pPage) // but it is active
			{
				m_pStartPage->Destroy();
				reactivate = false;
			}
			m_aWnds.erase(m_aWnds.begin()+i);
			m_wndToolBar.DeleteButton(i);
		}

		RECT rect;
		m_wndToolBar.GetWindowRect(&rect);
		ScreenToClient(&rect);
		DWORD dwButtonSize = m_wndToolBar.GetButtonSize();
		m_wndToolBar.SetButtonSize(max(LOWORD(dwButtonSize), HIWORD(dwButtonSize)) + m_nTBGaps+m_nTBGaps, HIWORD(dwButtonSize) + m_nTBGaps);
		rect.right = rect.left + max(LOWORD(dwButtonSize), HIWORD(dwButtonSize)) + m_nTBGaps+m_nTBGaps;
		m_wndToolBar.MoveWindow(&rect);

		RECT rcClient;
		GetClientRect(&rcClient);
		m_rcToolBar = rect;
		m_rcToolBar.bottom -= rcClient.bottom;

		if (reactivate)
		{
			g_tLastStartPage = m_tStartPage = m_aWnds[0].tClsID;
			m_pStartPage = m_aWnds[0].pPage;
			ShowHidePages();
		}
	}
	else if (m_pStartPage)
	{
		m_pStartPage->OnIdle();
	}
	return S_OK;
}

LRESULT CDesignerViewWndStart::OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	int nX = LOWORD(a_lParam);
	int nY = HIWORD(a_lParam);
	RECT rc = m_rcToolBar;
	RECT rcPage = GetPageRectangle(nX, nY);

	rc.bottom += nY;
	m_wndToolBar.MoveWindow(&rc);

	rc = GetPageRectangle(nX, nY);
	for (CStartPages::const_iterator i = m_aWnds.begin(); i != m_aWnds.end(); ++i)
	{
		if (i->pPage)
			i->pPage->Move(&rcPage);
	}

	return 0;
}

LRESULT CDesignerViewWndStart::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	for (CStartPages::const_iterator i = m_aWnds.begin(); i != m_aWnds.end(); ++i)
	{
		if (i->pPage)
			i->pPage->SendMessage(uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CDesignerViewWndStart::OnSelectPage(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
{
	m_wndToolBar.CheckButton(a_wID);
	unsigned nID = a_wID-ID_PAGE1;
	if (nID < m_aWnds.size() && m_aWnds[nID].pFactory)
	{
		g_tLastStartPage = m_tStartPage = m_aWnds[nID].tClsID;
		m_pStartPage = m_aWnds[nID].pPage;
		ShowHidePages();
	}
	return 0;
}

LRESULT CDesignerViewWndStart::OnOK(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
{
	OnOKEx();

	return 0;
}

void CDesignerViewWndStart::ShowHidePages()
{
	CWaitCursor cWait;
	CStartPages::iterator a = m_aWnds.end();
	for (CStartPages::iterator i = m_aWnds.begin(); i != m_aWnds.end(); ++i)
	{
		if (IsEqualCLSID(m_tStartPage, i->tClsID))
		{
			a = i;
		}
		else if (i->pPage)
		{
			i->pPage->Deactivate();
		}
	}
	if (a != m_aWnds.end())
	{
		RECT rc;
		GetClientRect(&rc);
		rc = GetPageRectangle(rc.right, rc.bottom);
		if (a->pPage == NULL)
		{
			a->pFactory->Create(m_hWnd, &rc, m_tLocaleID, m_pInternObserver, m_pMainCfg, &(a->pPage));
			m_pStartPage = a->pPage;
		}
		a->pPage->Activate();
		if (m_pAnimation)
		{
			m_pAnimation = NULL;
			KillTimer(ANIMATIONTIMERID);
			ClearAnimationCache();
		}
		a->pFactory->QueryInterface(&m_pAnimation);
		m_dwAnimationStart = GetTickCount();
		BOOL b;
		OnTimer(0, 0, 0, b);
	}
}

RECT CDesignerViewWndStart::GetPageRectangle(int a_nSizeX, int a_nSizeY) const
{
	RECT rc = {m_rcToolBar.right+m_rcToolBar.left/*+2*/, 0, a_nSizeX, a_nSizeY};
	return rc;
}

void CDesignerViewWndStart::ReportError(ILocalizedString* a_pMessage)
{
	CComBSTR bstr;
	m_pCaption->GetLocalized(m_tLocaleID, &bstr);
	CComBSTR bstrMsg;
	a_pMessage->GetLocalized(m_tLocaleID, &bstrMsg);
	MessageBox(bstrMsg, bstr, MB_OK|MB_ICONEXCLAMATION);
}

void CDesignerViewWndStart::OpenDocument(IDocument* a_pDoc)
{
	if (a_pDoc) a_pDoc->ClearDirty();
	m_pDocCtrl->SetNewDocument(a_pDoc);
}

void CDesignerViewWndStart::OnOKEx()
{
	CWaitCursor cWait;
	IStartViewPage* p = m_pStartPage;
	HRESULT hRes = S_OK;
	if (p == NULL || FAILED(hRes = p->ClickedDefault()))
	{
		if (hRes != E_RW_CANCELLEDBYUSER)
		{
			CComBSTR bstr;
			m_pCaption->GetLocalized(m_tLocaleID, &bstr);
			TCHAR szText[192] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_ACTIONFAILED, szText, itemsof(szText), LANGIDFROMLCID(m_tLocaleID));

			MessageBox(szText, COLE2CT(bstr), MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

LRESULT CDesignerViewWndStart::OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	m_pStartPage = NULL;
	//m_wndOKButton.DestroyWindow();
	// due to multithreading issues, child windows must be destroyed explicitly ;-(
	for (CStartPages::iterator i = m_aWnds.begin(); i != m_aWnds.end(); ++i)
	{
		if (i->pPage)
		{
			i->pPage->Destroy();
			i->pPage = NULL;
		}
	}
	if (m_pAnimation)
	{
		m_pAnimation = NULL;
		KillTimer(ANIMATIONTIMERID);
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CDesignerViewWndStart::OnHotItemChange(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	NMTBHOTITEM* p = reinterpret_cast<NMTBHOTITEM*>(a_pNMHdr);
	if (p->dwFlags & HICF_LEAVING)
	{
		if (m_nLastHotButton == p->idOld)
			m_bShowButtonDesc = false;
	}
	else
	{
		m_bShowButtonDesc = false;
		m_bstrButtonDesc.Empty();
		unsigned nID = p->idNew-ID_PAGE1;
		if (nID < m_aWnds.size() && m_aWnds[nID].pFactory)
		{
			CComPtr<ILocalizedString> pDesc;
			m_aWnds[nID].pFactory->HelpText(&pDesc);
			m_bShowButtonDesc = pDesc && SUCCEEDED(pDesc->GetLocalized(m_tLocaleID, &m_bstrButtonDesc)) && m_bstrButtonDesc.m_str;
			m_nLastHotButton = p->idNew;
		}
	}
	if (m_pStatusBar) m_pStatusBar->Notify(0, 0);

	return 0;
}

#include <math.h>

//ULONG GetShadowLevel(LONG a_nPos, ULONG a_nBlur)
//{
//	if (a_nPos >= (a_nBlur<<1))
//		return 0x100;
//	if (a_nPos < 0)
//		return 0;
//	float f1 = ((a_nBlur<<1)-0.5f-a_nPos)/(a_nBlur<<1);
//	return 0x100*expf(-f1*f1*2)+0.5f;
//	//return 0x100*(sqrtf((a_nBlur<<1)*(a_nBlur<<1)-f1*f1)/(a_nBlur<<1))+0.5f;
//}

//ULONG GetShadowLevel(float a_fPos, ULONG a_nBlur)
//{
//	if (a_fPos >= (a_nBlur<<1))
//		return 0x100;
//	float f1 = (a_nBlur<<1)-0.5f-a_fPos;
//	return 0x100*(sqrtf((a_nBlur<<1)*(a_nBlur<<1)-f1*f1)/(a_nBlur<<1))+0.5f;
//}

COLORREF GammaBlend(COLORREF a_clrBg, COLORREF a_clrFg, ULONG a_nAlpha)
{
	float const fG1 = 2.2f;
	float const fG2 = 1.0f/fG1;
	float const fA1 = a_nAlpha/256.0f;
	float const fA2 = 1.0f-fA1;
	return RGB(
		ULONG(powf(powf(GetRValue(a_clrFg)/255.0f, fG1)*fA1+powf(GetRValue(a_clrBg)/255.0f, fG1)*fA2, fG2)*255.0f+0.5f),
		ULONG(powf(powf(GetGValue(a_clrFg)/255.0f, fG1)*fA1+powf(GetGValue(a_clrBg)/255.0f, fG1)*fA2, fG2)*255.0f+0.5f),
		ULONG(powf(powf(GetBValue(a_clrFg)/255.0f, fG1)*fA1+powf(GetBValue(a_clrBg)/255.0f, fG1)*fA2, fG2)*255.0f+0.5f)
		);
}

LRESULT CDesignerViewWndStart::OnTBCustomDraw(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
{
	NMTBCUSTOMDRAW* pTBCD = reinterpret_cast<NMTBCUSTOMDRAW*>(a_pNMHdr);
	switch (pTBCD->nmcd.dwDrawStage)
	{
	case CDDS_PREERASE:
		{
			RECT rc;// = pTBCD->nmcd.rc;
			m_wndToolBar.GetClientRect(&rc);
			rc.right -= 2;
			HBRUSH hBr = CreateSolidBrush(GammaBlend(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW), 0x80));
			FillRect(pTBCD->nmcd.hdc, &rc, hBr);
			DeleteObject(hBr);
			//FillRect(pTBCD->nmcd.hdc, &rc, (HBRUSH)(COLOR_3DFACE+1));
			rc.left = rc.right; ++rc.right; --rc.bottom;
			FillRect(pTBCD->nmcd.hdc, &rc, (HBRUSH)(COLOR_3DSHADOW+1));
			rc.left = rc.right; ++rc.right;
			FillRect(pTBCD->nmcd.hdc, &rc, (HBRUSH)(COLOR_3DHIGHLIGHT+1));
			--rc.left; rc.top = rc.bottom; ++rc.bottom;
			FillRect(pTBCD->nmcd.hdc, &rc, (HBRUSH)(COLOR_3DHIGHLIGHT+1));
		}
		return CDRF_SKIPDEFAULT;
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		{
			TBBUTTONINFO tTBBI;
			ZeroMemory(&tTBBI, sizeof tTBBI);
			tTBBI.cbSize = sizeof tTBBI;
			TCHAR szText[256] = _T("");
			tTBBI.pszText = szText;
			tTBBI.cchText = itemsof(szText);
			tTBBI.dwMask = TBIF_TEXT|TBIF_IMAGE;
			tTBBI.idCommand = pTBCD->nmcd.dwItemSpec;
			m_wndToolBar.GetButtonInfo(pTBCD->nmcd.dwItemSpec, &tTBBI);
			if (*szText == _T('\0')) m_wndToolBar.GetButtonText(pTBCD->nmcd.dwItemSpec, szText); // fix for a strange error (text not fetched) on Windows 2000

			RECT rcItem = pTBCD->nmcd.rc;
			ULONG nOff = pTBCD->nmcd.uItemState&CDIS_CHECKED ? m_nTBGaps>>1 : m_nTBGaps;
			rcItem.left += nOff;
			rcItem.right -= m_nTBGaps-nOff;

			if (pTBCD->nmcd.uItemState&CDIS_CHECKED)
			{
				RECT rc = pTBCD->nmcd.rc;
				rc.left += m_nTBGaps>>1;
				{ RECT rc2 = {rc.left+3, rc.top+3, rc.right, rc.bottom-3}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DFACE+1)); }
				COLORREF clrL = GetSysColor(COLOR_3DHIGHLIGHT);
				COLORREF clrD = GetSysColor(COLOR_3DSHADOW);
				COLORREF clrF = GetSysColor(COLOR_3DFACE);
				{ RECT rc2 = {rc.left+3, rc.top+2, rc.right-2, rc.top+3}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DHIGHLIGHT+1)); }
				{ RECT rc2 = {rc.left+3, rc.top+1, rc.right-2, rc.top+2}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				//SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.top, clrD);
				//SetPixel(pTBCD->nmcd.hdc, rc.right-1, rc.top, clrL);
				SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.top+1, clrL);
				SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.top+2, clrF);
				SetPixel(pTBCD->nmcd.hdc, rc.right-1, rc.top+2, clrF);
				SetPixel(pTBCD->nmcd.hdc, rc.right-1, rc.top+1, clrF);
				{ RECT rc2 = {rc.left+2, rc.top+3, rc.left+3, rc.bottom-3}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DHIGHLIGHT+1)); }
				{ RECT rc2 = {rc.left+1, rc.top+3, rc.left+2, rc.bottom-3}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				SetPixel(pTBCD->nmcd.hdc, rc.left+2, rc.top+2, clrD);
				SetPixel(pTBCD->nmcd.hdc, rc.left+2, rc.bottom-3, clrD);
				{ RECT rc2 = {rc.left+3, rc.bottom-2, rc.right-2, rc.bottom-1}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				{ RECT rc2 = {rc.left+3, rc.bottom-3, rc.right-2, rc.bottom-2}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DHIGHLIGHT+1)); }
				SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.bottom-2, clrL);
				SetPixel(pTBCD->nmcd.hdc, rc.right-1, rc.bottom-2, clrF);
				SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.bottom-3, clrF);
				SetPixel(pTBCD->nmcd.hdc, rc.right-1, rc.bottom-3, clrF);
			}
			else if (pTBCD->nmcd.uItemState&(CDIS_HOT|CDIS_SELECTED))
			{
				RECT rcWin;
				m_wndToolBar.GetClientRect(&rcWin);
				RECT rc = rcItem;
				rc.left += 2;
				rc.top += 2;
				rc.right = rcWin.right-1;
				rc.bottom -= 2;
				COLORREF clrD = GetSysColor(COLOR_3DSHADOW);
				{ RECT rc2 = {rc.left+2, rc.top, rc.right-2, rc.top+1}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				{ RECT rc2 = {rc.left+2, rc.bottom-1, rc.right-2, rc.bottom}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				{ RECT rc2 = {rc.left, rc.top+2, rc.left+1, rc.bottom-2}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				//{ RECT rc2 = {rc.right-1, rc.top+2, rc.right, rc.bottom-2}; FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DSHADOW+1)); }
				{
					RECT rc2 = {rc.left+1, rc.top+1, rc.right-1, rc.bottom-1};
					//FillRect(pTBCD->nmcd.hdc, &rc2, (HBRUSH)(COLOR_3DFACE+1));
					HBRUSH hBr = CreateSolidBrush(GammaBlend(GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW), 0x40));
					FillRect(pTBCD->nmcd.hdc, &rc2, hBr);
					DeleteObject(hBr);
				}
				SetPixel(pTBCD->nmcd.hdc, rc.left+1, rc.top+1, clrD);
				//SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.top+1, clrD);
				SetPixel(pTBCD->nmcd.hdc, rc.left+1, rc.bottom-2, clrD);
				//SetPixel(pTBCD->nmcd.hdc, rc.right-2, rc.bottom-2, clrD);
			}

			RECT rcText = rcItem;
			HFONT hFont = m_wndToolBar.GetFont();
			HGDIOBJ hPrevFont = SelectObject(pTBCD->nmcd.hdc, hFont);
			DrawText(pTBCD->nmcd.hdc, szText, -1, &rcText, DT_SINGLELINE|DT_CALCRECT);
			ULONG nTextY = rcText.bottom-rcText.top;
			ULONG nSpareY = rcItem.bottom-rcItem.top-m_nTBGaps-iconSize-nTextY;
			rcText.left = rcItem.left;
			rcText.right = rcItem.right;
			rcText.top = rcItem.top+iconSize+nSpareY-(nSpareY/3)+(m_nTBGaps>>1);
			rcText.bottom = rcText.top+nTextY;
			int iPrevMode = SetBkMode(pTBCD->nmcd.hdc, TRANSPARENT);
			DrawText(pTBCD->nmcd.hdc, szText, -1, &rcText, DT_SINGLELINE|DT_CENTER);
			SelectObject(pTBCD->nmcd.hdc, hPrevFont);
			SetBkMode(pTBCD->nmcd.hdc, iPrevMode);
			if ((pTBCD->nmcd.uItemState&CDIS_CHECKED) && m_pAnimation)
			{
				DWORD dwTime = GetTickCount()-m_dwAnimationStart;
				ULONG next = 0;
				ULONG index = m_pAnimation->Phase(dwTime, &next);
				HICON hIcon = NULL;
				CAnimationCache::const_iterator c = m_cAnimationCache.find(index);
				if (c != m_cAnimationCache.end())
					hIcon = c->second;
				else
					m_cAnimationCache[index] = hIcon = m_pAnimation->Icon(index, iconSize);
				DrawIconEx(pTBCD->nmcd.hdc, (rcItem.right+rcItem.left-iconSize)>>1, rcItem.top+(nSpareY/3)+(m_nTBGaps>>1), hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);
				if (next < 0xfffffff)
					SetTimer(ANIMATIONTIMERID, next);
			}
			else
			{
				ImageList_Draw(m_hToolBarImages, tTBBI.iImage, pTBCD->nmcd.hdc, (rcItem.right+rcItem.left-iconSize)>>1, rcItem.top+(nSpareY/3)+(m_nTBGaps>>1), ILD_NORMAL);
			}

			return CDRF_SKIPDEFAULT;
		}
	}
	return CDRF_DODEFAULT;
}

LRESULT CDesignerViewWndStart::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pAnimation)
	{
		for (CStartPages::const_iterator i = m_aWnds.begin(); i != m_aWnds.end(); ++i)
			if (i->pPage.p == m_pStartPage)
			{
				RECT rc = {0, 0, 0, 0};
				m_wndToolBar.GetItemRect(i-m_aWnds.begin(), &rc);
				m_wndToolBar.InvalidateRect(&rc);
				break;
			}
	}
	return 0;
}

void CDesignerViewWndStart::ClearAnimationCache()
{
	for (CAnimationCache::const_iterator i = m_cAnimationCache.begin(); i != m_cAnimationCache.end(); ++i)
		DestroyIcon(i->second);
	m_cAnimationCache.clear();
}

