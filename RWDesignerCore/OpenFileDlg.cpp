// OpenFileDlg.cpp : Implementation of COpenFileDlg

#include "stdafx.h"
#include "OpenFileDlg.h"

#include <XPGUI.h>
#include <MultiLanguageString.h>
#include "resource.h"
#include "ConfigIDsApp.h"


void COpenFileDlg::WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig)
{
	m_tLocaleID = a_tLocaleID;
	m_pClbk = a_pCallback;
	m_pAppConfig = a_pAppConfig;
	a_pAppConfig->SubConfigGet(CComBSTR(CFGID_STORAGE), &m_pStorageContext);
	RWCoCreateInstance(m_pStorageManager, __uuidof(StorageManager));
	RWCoCreateInstance(m_pInMgr, __uuidof(InputManager));

	RWCoCreateInstance(m_pBuilders, __uuidof(EnumUnknowns));
	GetBuilders(M_AppConfig(), m_pBuilders);

	m_pSubmenuTemplate.Attach(new CMultiLanguageString(L"[0409]Open as \"%s\"[0405]Otevřít jako \"%s\""));

	Create(a_hParent);
	MoveWindow(a_prc);
}

STDMETHODIMP COpenFileDlg::Activate()
{
	ShowWindow(SW_SHOW);
	RWHWND hWnd = NULL;
	m_pWnd->Handle(&hWnd);
	if (hWnd) ::SetFocus(hWnd);
	//SetDefaultButtonState(_SharedStringTable.GetStringAuto(IDS_STARTVIEWBTN_OPEN), true, M_OpenAsTempl());
	return S_OK;
}

STDMETHODIMP COpenFileDlg::Deactivate()
{
	if (IsWindow())
	{
		ShowWindow(SW_HIDE);
	}
	return S_OK;
}

LRESULT COpenFileDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	AddRef();

	m_wndOKButton.SubclassWindow(GetDlgItem(IDOK), static_cast<IStartViewPage*>(this));

	RECT rc = {0, 0, 0, 0};
	GetClientRect(&rc);
	m_rcGaps.left = 0;
	m_rcGaps.top = 0;
	m_rcGaps.right = 57;
	m_rcGaps.bottom = 21;
	MapDialogRect(&m_rcGaps);
	rc.bottom -= m_rcGaps.bottom;

	CComPtr<IEnumUnknowns> pTypes;
	m_pInMgr->DocumentTypesEnumEx(m_pBuilders, &pTypes);
	m_pStorageManager->FilterWindowCreate(CComBSTR(L""), EFTOpenExisting, m_hWnd, pTypes, NULL, m_pStorageContext, this, NULL, m_tLocaleID, &m_pWnd);
	m_pWnd->Move(&rc);

	return 1;  // Let the system set the focus
}

LRESULT COpenFileDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_wndOKButton.SetWindowPos(NULL, GET_X_LPARAM(lParam)-m_rcGaps.right, GET_Y_LPARAM(lParam)-m_rcGaps.bottom, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	RECT rc = {0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)-m_rcGaps.bottom};
	if (m_pWnd) m_pWnd->Move(&rc);
	return 0;
}

LRESULT COpenFileDlg::OnSubMenuClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (HasChoices() != S_OK)
	{
		m_pClbk->ReportError(_SharedStringTable.GetStringAuto(IDS_ACTIONFAILED));
	}
	CImageList cImageList;
	cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 4, 4);
	Reset(cImageList);
	CComPtr<ILocalizedString> pName;
	HICON hIcon = NULL;
	ULONG nIcon = 0;
	CMenu cMenu;
	cMenu.CreatePopupMenu();
	TCHAR szTempl[64] = _T("");
	CComBSTR bstrTemplate;
	m_pSubmenuTemplate->GetLocalized(m_tLocaleID, &bstrTemplate);
	TCHAR szBuf[256] = _T("");
	HRESULT hRes;
	for (ULONG i = 0; E_RW_INDEXOUTOFRANGE != (hRes = GetChoiceProps(i, &pName, XPGUI::GetSmallIconSize(), &hIcon)); ++i, pName = NULL, hIcon = NULL)
	{
		if (FAILED(hRes) || pName == NULL) continue;

		CComBSTR bstr;
		pName->GetLocalized(m_tLocaleID, &bstr);
		_stprintf(szBuf, bstrTemplate.m_str, (LPCTSTR)(COLE2CT(bstr)));
		if (hIcon)
		{
			cImageList.AddIcon(hIcon);
			DestroyIcon(hIcon);
			AddItem(cMenu, i+1, szBuf, nIcon++);
		}
		else
		{
			AddItem(cMenu, i+1, szBuf, -1);
		}
	}
	RECT rc;
	m_wndOKButton.GetWindowRect(&rc);
	TPMPARAMS tTPMP = { sizeof(TPMPARAMS), rc };
	UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, rc.left, rc.bottom, m_hWnd, &tTPMP);
	cImageList.Destroy();
	if (nSelection != 0)
	{
		CWaitCursor cWait;
		if (FAILED(ClickedChoice(nSelection-1)))
		{
			m_pClbk->ReportError(_SharedStringTable.GetStringAuto(IDS_ACTIONFAILED));
		}
	}
	return 0;
}

STDMETHODIMP COpenFileDlg::ClickedDefault()
{
	if (m_pWnd)
	{
		CComPtr<IStorageFilter> pFilter;
		m_pWnd->FilterCreate(&pFilter);
		if (pFilter)
		{
			CComPtr<IDocument> pDoc;
			HRESULT hRes = m_pInMgr->DocumentCreateEx(m_pBuilders, pFilter, NULL, &pDoc);
			if (pDoc != NULL)
			{
				m_pClbk->OpenDocument(pDoc);
			}
			return hRes;
		}
	}
	return S_OK;
}

HRESULT COpenFileDlg::GetChoiceProps(ULONG a_nIndex, ILocalizedString** a_ppName, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		CComPtr<IDocumentBuilder> pBuilder;
		GetBuilder(M_AppConfig(), a_nIndex, &pBuilder);
		if (pBuilder == NULL)
			return E_RW_INDEXOUTOFRANGE;
		if (a_ppName)
			pBuilder->TypeName(a_ppName);
		if (a_phIcon)
			pBuilder->Icon(a_nSize, a_phIcon);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT COpenFileDlg::ClickedChoice(ULONG a_nIndex)
{
	if (m_pWnd)
	{
		CComPtr<IStorageFilter> pFilter;
		m_pWnd->FilterCreate(&pFilter);
		if (pFilter)
		{
			CComPtr<IDocumentBuilder> pBuilder;
			m_pBuilders->Get(a_nIndex, &pBuilder);
			CComPtr<IDocument> pDoc;
			HRESULT hRes = m_pInMgr->DocumentCreateEx(pBuilder, pFilter, NULL, &pDoc);
			if (pDoc != NULL)
			{
				m_pClbk->OpenDocument(pDoc);
			}
			return hRes;
		}
	}
	return S_OK;
}


STDMETHODIMP COpenFileDlg::Destroy()
{
	m_pClbk = NULL;

	if (m_pWnd)
		return m_pWnd->Destroy();
	return S_OK;
}

STDMETHODIMP COpenFileDlg::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	if (m_pWnd)
		return m_pWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
	return S_FALSE;
}

