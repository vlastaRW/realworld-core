
#pragma once


class CFilePropertiesDlg : 
	public Win32LangEx::CLangDialogImpl<CFilePropertiesDlg>
{
public:
	CFilePropertiesDlg(LCID a_tLocaleID, LPCTSTR a_pszName, LPCTSTR a_pszCrtd, LPCTSTR a_pszMdfd, LPTSTR a_pszTags, size_t a_nTags, std::wstring& a_strNote, HICON a_hIcon) :
		Win32LangEx::CLangDialogImpl<CFilePropertiesDlg>(a_tLocaleID),
		m_pszName(a_pszName), m_pszCrtd(a_pszCrtd), m_pszMdfd(a_pszMdfd),
		m_pszTags(a_pszTags), m_nTags(a_nTags), m_strNote(a_strNote), m_hIcon(a_hIcon)
	{
	}
	~CFilePropertiesDlg()
	{
	}

	enum { IDD = IDD_FILEPROPERTIES };

	BEGIN_MSG_MAP(CFilePropertiesDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CenterWindow(GetParent());
		SetDlgItemText(IDC_MTN_FILENAME, m_pszName);
		SetDlgItemText(IDC_MTN_CREATED, m_pszCrtd);
		SetDlgItemText(IDC_MTN_MODIFIED, m_pszMdfd);
		CStatic wnd = GetDlgItem(IDC_MTN_THUMBNAIL);
		wnd.SetIcon(m_hIcon);
		RECT rcIcon;
		wnd.GetWindowRect(&rcIcon);
		RECT rcTags;
		GetDlgItem(IDC_MTN_TAGS).GetWindowRect(&rcTags);
		int nDX = (rcTags.left-rcIcon.right-8)>>1;
		int nDY = (rcTags.top-rcIcon.bottom-8)>>1;
		rcIcon.left += nDX; rcIcon.right += nDX;
		rcIcon.top += nDY; rcIcon.bottom += nDY;
		ScreenToClient(&rcIcon);
		wnd.MoveWindow(&rcIcon);
		SetDlgItemText(IDC_MTN_TAGS, m_pszTags);
		SetDlgItemText(IDC_MTN_NOTE, CW2CT(m_strNote.c_str()));
		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		GetDlgItemText(IDC_MTN_TAGS, m_pszTags, m_nTags);
		m_pszTags[m_nTags-1] = _T('\0');
		CWindow wnd = GetDlgItem(IDC_MTN_NOTE);
		int n = wnd.GetWindowTextLength();
		CAutoVectorPtr<TCHAR> cBuf(new TCHAR[n+1]);
		wnd.GetWindowText(cBuf, n+1);
		cBuf[n] = _T('\0');
		CT2CW str(cBuf.m_p);
		m_strNote = str;
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnClickedCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		EndDialog(a_wID);
		return 0;
	}

private:
	LPCTSTR m_pszName;
	LPCTSTR m_pszCrtd;
	LPCTSTR m_pszMdfd;
	LPTSTR m_pszTags;
	size_t m_nTags;
	std::wstring& m_strNote;
	HICON m_hIcon;
};


