
#pragma once

class CFatalErrorDlg : public Win32LangEx::CLangDialogImpl<CFatalErrorDlg>
{
public:
	CFatalErrorDlg(IDocument* a_pDoc, ILocalizedString* a_pCaption, LCID a_tLocaleID) :
		Win32LangEx::CLangDialogImpl<CFatalErrorDlg>(a_tLocaleID),
		m_pDoc(a_pDoc), m_pCaption(a_pCaption)
	{
	}

	enum { IDD = IDD_FATALERROR };

	BEGIN_MSG_MAP(CDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_FE_SAVEBUTTON, BN_CLICKED, OnClickedSave)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		try
		{
			CenterWindow();

			TCHAR szCaption[256] = _T("RealWorld Designer");
			if (m_pCaption)
			{
				CComBSTR bstrCaption;
				m_pCaption->GetLocalized(m_tLocaleID, &bstrCaption);
				USES_CONVERSION;
				_tcscpy(szCaption, OLE2CT(bstrCaption));
			}
			SetWindowText(szCaption);

			CStatic cIcon = GetDlgItem(IDC_FE_ICON);
			cIcon.SetIcon(::LoadIcon(NULL, IDI_ERROR));

			if (m_pDoc == NULL || m_pDoc->IsDirty() != S_OK)
			{
				GetDlgItem(IDC_FE_SAVEBUTTON).EnableWindow(FALSE);
				GetDlgItem(IDC_FE_SAVELABEL).EnableWindow(FALSE);
			}
		}
		catch (...)
		{
		}
		return TRUE;
	}
	LRESULT OnClickedOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnClickedCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnClickedSave(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		try
		{
			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));

			CComPtr<IStorageFilterWindowListener> pWindowListener;
			CComPtr<IEnumUnknowns> pFlts;
			CComPtr<IConfig> pSaveCfg;
			pIM->SaveOptionsGet(m_pDoc, &pSaveCfg, &pFlts, &pWindowListener);

			CComPtr<IStorageManager> pStMgr;
			RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
			static const GUID tContext = {0x83759b36, 0x3393, 0x47e3, {0x9b, 0x9a, 0xfb, 0x60, 0x6, 0xe1, 0x25, 0x79}};
			CComPtr<IStorageFilter> pFlt;
			pStMgr->FilterCreateInteractivelyUID(CComBSTR(L""), EFTCreateNew, m_hWnd, pFlts, pSaveCfg, tContext, _SharedStringTable.GetStringAuto(IDS_SAVEDLGCAPTION), pWindowListener, m_tLocaleID, &pFlt);
			if (pFlt)
			{
				CWaitCursor cWait;
				pIM->Save(m_pDoc, pSaveCfg, pFlt);
			}
		}
		catch (...)
		{
		}
		return 0;
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<ILocalizedString> m_pCaption;
};
