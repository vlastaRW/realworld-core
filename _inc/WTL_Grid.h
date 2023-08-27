
#pragma once


class CGridListCtrl :
	public CWindowImpl<CGridListCtrl, CListViewCtrl>,
	public CCustomDraw<CGridListCtrl>
{
public:
	CGridListCtrl() : m_hEditBGBrush(NULL), m_wndEdit(this),
		m_nQuickEditRow(-1), m_nQuickEditCol(-1)
	{
	}
	~CGridListCtrl()
	{
		if (m_hEditBGBrush != NULL)
			DeleteObject(m_hEditBGBrush);
	}

	long curCol;

	BEGIN_MSG_MAP(CGridListCtrl)    
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouse)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKey)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColorEdit)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CGridListCtrl>, 1)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnSysColorChange)
	END_MSG_MAP()          	


	HWND GetEditCtrl() const
	{
		return m_wndEdit;
	}

	LRESULT OnCreate(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& a_bHandled)
	{
		BOOL bDummy;
		OnSysColorChange(0, 0, 0, bDummy);

		// Info
		curCol = 0;
		m_wndEdit.Create(m_hWnd, rcDefault, _T("GridEdit"), WS_CHILD | WS_CLIPSIBLINGS | ES_LEFT | ES_AUTOHSCROLL | WS_BORDER);

		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnMouse(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch(a_wParam)
		{
			case MK_CONTROL: break;
			case MK_LBUTTON:
			{
				EndQuickEdit();
				LVHITTESTINFO info;
				info.pt.x = LOWORD(a_lParam);
				info.pt.y = HIWORD(a_lParam);
				info.iItem  = 0;
				info.iSubItem = 0;
				SubItemHitTest(&info);
				if (GetFocusedRow() == info.iItem)
				{
					RedrawItems(GetFocusedRow(), GetFocusedRow());
				}
				curCol = info.iSubItem;
				break;
			}
			case MK_MBUTTON: break;
			case MK_RBUTTON: break;
			case MK_SHIFT: break;
		}
		a_bHandled = FALSE;
		
		return 0;
	}

	LRESULT OnKey(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM /*a_lParam*/, BOOL& a_bHandled)
	{
		switch (a_wParam)
		{
		case VK_RIGHT:
			EndQuickEdit();
			curCol = min(curCol+1, GetColCount()-1);
			RedrawItems(GetFocusedRow(), GetFocusedRow());
			break;
		case VK_LEFT:
			EndQuickEdit();
			curCol = max(curCol-1, 0);
			RedrawItems(GetFocusedRow(), GetFocusedRow());
			break;
		case VK_F2:
			EndQuickEdit();
			BeginItemTextEdit(GetFocusedRow(), curCol);
			break;
		}
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnChar(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM /*a_lParam*/, BOOL& a_bHandled)
	{
		if (a_wParam == _T('\n'))
		{
			EndQuickEdit();
			return 0;
		}

		BeginQuickEdit(GetFocusedRow(), curCol);
		if (!IsQuickEditActive())
			return 0;

		TCHAR szTmp[512] = _T("");
		GetItemText(m_nQuickEditRow, m_nQuickEditCol, szTmp, sizeof(szTmp)/sizeof(szTmp[0])-1);
		szTmp[sizeof(szTmp)/sizeof(szTmp[0])-2] = _T('\0');
		size_t nLen = _tcslen(szTmp);
		if (a_wParam == 8)
		{
			if (nLen > 0)
				szTmp[nLen-1] = _T('\0'); // delete last char
		}
		else if (a_wParam >= _T(' '))
		{
			szTmp[nLen] = a_wParam; // append character
			szTmp[nLen+1] = _T('\0');
		}
		SetItemText(m_nQuickEditRow, m_nQuickEditCol, szTmp);
		return 0;
	}


	// item text editing

	bool IsQuickEditActive() const
	{
		return m_nQuickEditCol >= 0 && m_nQuickEditRow >= 0;
	}

	void BeginQuickEdit(int a_nRow, int a_nCol)
	{
		if (a_nCol == m_nQuickEditCol && a_nRow == m_nQuickEditRow)
			return; // continue last quickedit

		if (a_nCol >= 0 && a_nCol < GetColCount() &&
			a_nRow >= 0 && a_nRow < GetItemCount())
		{
			EndQuickEdit();

			NMLVDISPINFO tNMInfo;
			tNMInfo.hdr.hwndFrom = m_hWnd;
			tNMInfo.hdr.idFrom = GetWindowLong(GWL_ID);
			tNMInfo.hdr.code = LVN_BEGINLABELEDIT;
			tNMInfo.item.mask = 0;
			tNMInfo.item.iItem = a_nRow;
			tNMInfo.item.iSubItem = a_nCol;
			if (GetParent().SendMessage(WM_NOTIFY, tNMInfo.hdr.idFrom, reinterpret_cast<LPARAM>(&tNMInfo)))
				return;

			m_nQuickEditRow = a_nRow;
			m_nQuickEditCol = a_nCol;
		}
	}

	void EndQuickEdit()
	{
		if (m_nQuickEditCol >= 0 && m_nQuickEditCol < GetColCount() &&
			m_nQuickEditRow >= 0 && m_nQuickEditRow < GetItemCount())
		{
			TCHAR szTmp[512] = _T("");
			GetItemText(m_nQuickEditRow, m_nQuickEditCol, szTmp, sizeof(szTmp)/sizeof(szTmp[0]));
			szTmp[sizeof(szTmp)/sizeof(szTmp[0])-1] = _T('\0');
			NMLVDISPINFO tNMInfo;
			tNMInfo.hdr.hwndFrom = m_hWnd;
			tNMInfo.hdr.idFrom = GetWindowLong(GWL_ID);
			tNMInfo.hdr.code = LVN_ENDLABELEDIT;
			tNMInfo.item.mask = LVIF_TEXT;
			tNMInfo.item.iItem = m_nQuickEditRow;
			tNMInfo.item.iSubItem = m_nQuickEditCol;
			tNMInfo.item.pszText = szTmp;
			GetParent().SendMessage(WM_NOTIFY, tNMInfo.hdr.idFrom, reinterpret_cast<LPARAM>(&tNMInfo));
			m_nQuickEditCol = -1;
			m_nQuickEditRow = -1;
		}
	}

	void BeginItemTextEdit(int a_nRow, int a_nCol)
	{
		EndItemTextEdit(true);

		NMLVDISPINFO tNMInfo;
		tNMInfo.hdr.hwndFrom = m_hWnd;
		tNMInfo.hdr.idFrom = GetWindowLong(GWL_ID);
		tNMInfo.hdr.code = LVN_BEGINLABELEDIT;
		tNMInfo.item.mask = 0;
		tNMInfo.item.iItem = a_nRow;
		tNMInfo.item.iSubItem = a_nCol;
		if (GetParent().SendMessage(WM_NOTIFY, tNMInfo.hdr.idFrom, reinterpret_cast<LPARAM>(&tNMInfo)))
			return;

		RECT rc;

		GetSubItemRect(a_nRow, a_nCol, LVIR_BOUNDS, &rc);
		if (a_nCol == 0) // correction for first column
		{
			RECT rc2 = {0 , 0, 0, 0};
			GetHeader().GetItemRect(0, &rc2);
			rc.right = rc2.right;
			rc.left = 0;
		}
		rc.right++; rc.top--; // some rect correction
		//rc.left++; rc.bottom--; // some rect correction

		m_wndEdit.m_nRow = a_nRow;
		m_wndEdit.m_nCol = a_nCol;
		m_wndEdit.MoveWindow(&rc, TRUE);
		m_wndEdit.SetFont(GetFont()); 
		if ( curCol ) m_wndEdit.SetMargins(5,5); // some margins correction
		else		  m_wndEdit.SetMargins(3,5);

		LPTSTR pszText = NULL;
		int nRes = 0;
		for(int nLen = 256; ; nLen *= 2)
		{
			ATLTRY(pszText = new TCHAR[nLen]);
			if(pszText == NULL)
				break;
			pszText[0] = _T('\0');
			nRes = GetItemText(a_nRow, a_nCol, pszText, nLen);		
			if(nRes < nLen - 1)
			{
				m_wndEdit.SetWindowText(pszText);
				delete[] pszText;
				break;
			}
			delete[] pszText;
			pszText = NULL;
		}

		m_wndEdit.ShowWindow(SW_SHOW);
		m_wndEdit.SetFocus(); 
		m_wndEdit.SetSelAll();
		m_wndEdit.UpdateWindow();
	}

	void EndItemTextEdit(bool a_bCancelled = false)
	{
		if (m_wndEdit.GetWindowLong(GWL_STYLE)&WS_VISIBLE)
		{
			m_wndEdit.ShowWindow(SW_HIDE);
			int nTextLen = m_wndEdit.GetWindowTextLength();
			CAutoVectorPtr<TCHAR> pBuffer(new TCHAR[nTextLen+1]);
			m_wndEdit.GetWindowText(pBuffer, nTextLen+1);
			pBuffer.m_p[nTextLen] = _T('\0'); // paranoia
			NMLVDISPINFO tNMInfo;
			tNMInfo.hdr.hwndFrom = m_hWnd;
			tNMInfo.hdr.idFrom = GetWindowLong(GWL_ID);
			tNMInfo.hdr.code = LVN_ENDLABELEDIT;
			tNMInfo.item.mask = LVIF_TEXT;
			tNMInfo.item.iItem = m_wndEdit.m_nRow;
			tNMInfo.item.iSubItem = m_wndEdit.m_nCol;
			tNMInfo.item.pszText = a_bCancelled ? NULL : pBuffer.m_p;
			GetParent().SendMessage(WM_NOTIFY, tNMInfo.hdr.idFrom, reinterpret_cast<LPARAM>(&tNMInfo));
		}

	}

	// colors and custom drawing

	DWORD OnPrePaint(int /*a_idCtrl*/, LPNMCUSTOMDRAW /*a_lpNMCustomDraw*/)
	{			
		return CDRF_NOTIFYITEMDRAW;
	}

 	DWORD OnItemPrePaint(int /*a_idCtrl*/, LPNMCUSTOMDRAW /*a_pCD*/)
	{
		return CDRF_NOTIFYSUBITEMDRAW;
	} 

	DWORD OnSubItemPrePaint(int /*a_idCtrl*/, LPNMCUSTOMDRAW a_pCD)
	{
		LPNMLVCUSTOMDRAW const pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(a_pCD);
		if ((pLVCD->nmcd.uItemState & CDIS_SELECTED) && (GetItemState(pLVCD->nmcd.dwItemSpec, LVIS_SELECTED) == LVIS_SELECTED))
		{
			pLVCD->clrText	 = m_clrSelectedText;
			pLVCD->clrTextBk = m_clrSelectedBack;
		}
		else
		{
			pLVCD->clrText	 = m_clrNormalText;
			pLVCD->clrTextBk = m_clrNormalBack;
		}
		if (pLVCD->nmcd.uItemState & CDIS_FOCUS)
		{
			if (pLVCD->iSubItem == curCol)
			{
				pLVCD->clrText	 = m_clrFocusedText;
				pLVCD->clrTextBk = m_clrFocusedBack;
			}
		}
		pLVCD->nmcd.uItemState &= ~(CDIS_SELECTED|CDIS_FOCUS); // no defautl focus or selection

		return CDRF_DODEFAULT;
	}

	LRESULT OnSysColorChange(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& a_bHandled)
	{
		m_clrFocusedBack  = GetSysColor(COLOR_HIGHLIGHT);
		m_clrFocusedText  = GetSysColor(COLOR_HIGHLIGHTTEXT);
		m_clrSelectedBack = GetSysColor(COLOR_BTNFACE);
		m_clrSelectedText = GetSysColor(COLOR_BTNTEXT);
		m_clrNormalBack   = GetSysColor(COLOR_WINDOW);
		m_clrNormalText   = GetSysColor(COLOR_WINDOWTEXT);
		if (m_hEditBGBrush != NULL)
			DeleteObject(m_hEditBGBrush);
		m_hEditBGBrush = CreateSolidBrush(m_clrSelectedBack);
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnCtlColorEdit(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM /*a_lParam*/, BOOL& /*a_bHandled*/)
	{
 		::SetBkColor(reinterpret_cast<HDC>(a_wParam), m_clrSelectedBack);
		::SetTextColor(reinterpret_cast<HDC>(a_wParam), m_clrSelectedText);
		return reinterpret_cast<LRESULT>(m_hEditBGBrush);
	}

protected:
	int GetColCount()
	{
		return GetHeader().GetItemCount();
	}
	int GetFocusedRow() const
	{
		return GetNextItem(-1, LVNI_ALL|LVNI_FOCUSED);
	}

private:
	class CGridEdit : public CWindowImpl<CGridEdit, CEdit>
	{
	public:

		CGridEdit(CGridListCtrl* a_pParent) : m_pParent(a_pParent)
		{
		}

		BEGIN_MSG_MAP(CGridEdit)
			MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
			MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		END_MSG_MAP()

		LRESULT OnKillFocus(UINT /*a_uMsg*/, WPARAM /*a_wParam*/, LPARAM /*a_lParam*/, BOOL& /*a_bHandled*/)
		{
			m_pParent->EndItemTextEdit();
			return 0;
		}
		LRESULT OnKeyDown(UINT /*a_uMsg*/, WPARAM a_wParam, LPARAM /*a_lParam*/, BOOL& a_bHandled)
		{
			switch (a_wParam)
			{
				case VK_RETURN:
					m_pParent->EndItemTextEdit();
					break;
				case VK_ESCAPE:
					m_pParent->EndItemTextEdit(true);
					break;
				case VK_UP:
					if (m_nRow > 0)
					{
						m_pParent->EndItemTextEdit();
						m_pParent->SetItemState(m_nRow-1, LVIS_FOCUSED, LVIS_FOCUSED);
						m_pParent->BeginItemTextEdit(m_nRow-1, m_nCol);
					}
					break;
				case VK_DOWN:
					if (m_nRow < (m_pParent->GetItemCount()-1))
					{
						m_pParent->EndItemTextEdit();
						m_pParent->SetItemState(m_nRow+1, LVIS_FOCUSED, LVIS_FOCUSED);
						m_pParent->BeginItemTextEdit(m_nRow+1, m_nCol);
					}
					break;
				default:
					a_bHandled = FALSE;
			}	
			return 1;
		}

		int m_nRow;
		int m_nCol;
		CGridListCtrl* const m_pParent;
	};

private:
	HBRUSH m_hEditBGBrush;
	COLORREF m_clrFocusedBack;
	COLORREF m_clrFocusedText;
	COLORREF m_clrSelectedBack;
	COLORREF m_clrSelectedText;
	COLORREF m_clrNormalBack;
	COLORREF m_clrNormalText;

	CGridEdit m_wndEdit;
	int m_nQuickEditRow;
	int m_nQuickEditCol;
};

