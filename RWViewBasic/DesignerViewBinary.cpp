// DesignerViewBinary.cpp : Implementation of CDesignerViewBinary

#include "stdafx.h"
#include "DesignerViewBinary.h"


// CDesignerViewBinary

void CDesignerViewBinary::Init(ISharedStateManager* a_pStateMgr, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID)
{
	//m_nStyle = a_nStyle;
	// not multithread safe, but it does not matter
	//static INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_STANDARD_CLASSES};
	//static BOOL b = InitCommonControlsEx(&tICCE);

	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentBinary), reinterpret_cast<void**>(&m_pBinDoc));
	m_pBinDoc->ObserverIns(ObserverGet(), 0);

	//m_tLocaleID = a_tLocaleID;

	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("HexEditor"), WS_CHILDWINDOW|WS_CLIPSIBLINGS) == NULL)
	{
		// creation failed
		throw E_FAIL; // TODO: error code
	}
	UpdateWindow();

	ShowWindow(SW_SHOW);
}

LRESULT CDesignerViewBinary::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CLogFont lf = AtlGetStockFont(OEM_FIXED_FONT);
   _tcscpy(lf.lfFaceName, _T("Lucida Console"));     // This is the font we prefer
   lf.lfPitchAndFamily = FIXED_PITCH;                // Make sure Windows finds a fixed-width font
   m_fontEditor.CreateFontIndirect(&lf);

   CClientDC dc = m_hWnd;
   HFONT hOldFont = dc.SelectFont(m_fontEditor);
   dc.GetTextMetrics(&m_tmEditor);
   dc.SelectFont(hOldFont);

   ModifyStyle(0, WS_VSCROLL);
   SetScrollPos(SB_VERT, 0, TRUE);

   return 0;
}

LRESULT CDesignerViewBinary::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CreateSolidCaret(2, m_tmEditor.tmHeight - 2);
	ShowCaret();
	return 0;
}

LRESULT CDesignerViewBinary::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   HideCaret();
   DestroyCaret();
   return 0;
}

LRESULT CDesignerViewBinary::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if( m_rcAscii.left == 0 && m_rcData.left == 0 ) return 0; // Not ready yet!
   // Determine what area the cursor is over and change
   // cursor shape...
   DWORD dwPos = GetMessagePos();
   POINT pt = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };
   ScreenToClient(&pt);
   DWORD dwDummy = 0;
   bool bDummy = false;
   ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(GetPosFromPoint(pt, dwDummy, bDummy) ? IDC_IBEAM : IDC_ARROW)));
   return TRUE;
}

LRESULT CDesignerViewBinary::OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if( m_bReadOnly ) {
      ::MessageBeep((UINT)-1);
      return 0;
   }
   if( wParam < VK_SPACE ) return 0;
   if( m_dwSelStart != m_dwSelEnd ) SetSel(m_dwSelStart, m_dwSelStart);
   if( m_bInDataPane ) {
      if( isdigit(wParam) || (toupper(wParam) >= 'A' && toupper(wParam) <= 'F') ) {
         BYTE b = (BYTE)( isdigit(wParam) ? wParam - '0' : toupper(wParam) - 'A' + 10 );
         AssignDigitValue(m_dwSelStart, m_dwDigitOffset, b);
      }
   }
   else {
      AssignCharValue(m_dwSelStart, m_dwDigitOffset, LOBYTE(wParam));
      if( HIBYTE(wParam) != 0 ) AssignCharValue(m_dwSelStart, m_dwDigitOffset, HIBYTE(wParam));
   }
   return 0;
}

LRESULT CDesignerViewBinary::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   if( GetCapture() == m_hWnd ) {
      ReleaseCapture();
      return 0;
   }
   DWORD dwPage = BYTES_PR_LINE * (DWORD) GetLinesPrPage();
   bool bCtrl = (::GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
   bool bShift = (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
   switch( wParam ) {
   case VK_F6:
      PostMessage(WM_COMMAND, MAKEWPARAM(ID_NEXT_PANE, 0));
      return 0;
   case VK_DELETE:
      if( m_bInDataPane && !m_bReadOnly ) {
         m_dwDigitOffset = 0;
         for( int i = 0; i < m_iDataSize * 2; i++ ) PostMessage(WM_CHAR, '0');
      }
      return 0;
   case VK_LEFT:
      if( m_dwSelEnd < m_iDataSize ) return 0;
      SetSel(bShift ? m_dwSelStart : m_dwSelEnd - m_iDataSize, m_dwSelEnd - m_iDataSize);
      return 0;
   case VK_RIGHT:
      if( m_dwSelStart + m_iDataSize > M_DataLen() ) return 0;
      SetSel(bShift ? m_dwSelStart : m_dwSelEnd + m_iDataSize, m_dwSelEnd + m_iDataSize);
      return 0;
   case VK_UP:
	   if( bCtrl ) return CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, SB_LINEUP);
      if( m_dwSelEnd < BYTES_PR_LINE ) return 0;
      SetSel(bShift ? m_dwSelStart : m_dwSelEnd - BYTES_PR_LINE, m_dwSelEnd - BYTES_PR_LINE);
      return 0;
   case VK_DOWN:
	   if( bCtrl ) return CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, SB_LINEDOWN);
      if( m_dwSelStart + BYTES_PR_LINE > M_DataLen() ) return 0;
      SetSel(bShift ? m_dwSelStart : m_dwSelEnd + BYTES_PR_LINE, m_dwSelEnd + BYTES_PR_LINE);
      return 0;
   case VK_HOME:
      if( bCtrl ) SetSel(bShift ? m_dwSelStart : 0, 0);
      else SetSel(bShift ? m_dwSelStart : m_dwSelEnd - (m_dwSelEnd % BYTES_PR_LINE), m_dwSelEnd - (m_dwSelEnd % BYTES_PR_LINE));
      return 0;
   case VK_END:
      if( bCtrl ) SetSel(bShift ? m_dwSelStart : M_DataLen() - 1, M_DataLen() - (bShift ? 0 : 1));
      else SetSel(bShift ? m_dwSelStart : (m_dwSelEnd | 0xF) + (bShift ? 1 : 0), (m_dwSelEnd | 0xF) + (bShift ? 1 : 0));
      return 0;
   case VK_PRIOR:
	   if( bCtrl ) return CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, SB_PAGEUP);
      if( m_dwSelEnd < dwPage ) SetSel(bShift ? m_dwSelStart : 0, 0);
      else SetSel(bShift ? m_dwSelStart : m_dwSelEnd - dwPage, m_dwSelEnd - dwPage);
      return 0;
   case VK_NEXT:
	   if( bCtrl ) return CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, SB_PAGEDOWN);
      SetSel(bShift ? m_dwSelStart : m_dwSelEnd + dwPage, m_dwSelEnd + dwPage);
      return 0;
   }
   return 0;
}

LRESULT CDesignerViewBinary::OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
   POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
   bool bInDataPane = false;
   DWORD dwPos = 0;
   if( !GetPosFromPoint(pt, dwPos, bInDataPane) ) return 0;
   m_bInDataPane = bInDataPane;
   SetSel(dwPos, dwPos + m_iDataSize);
   return 0;
}

LRESULT CDesignerViewBinary::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
   SetFocus();
   POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
   bool bInDataPane = false;
   DWORD dwPos = 0;
   if( !GetPosFromPoint(pt, dwPos, bInDataPane) ) return 0;
   m_bInDataPane = bInDataPane;
   SetSel(dwPos, dwPos);
   // If user is dragging the mouse, we'll initiate a selection...
   ClientToScreen(&pt);
   if( ::DragDetect(m_hWnd, pt) ) SetCapture();
   return 0;
}

LRESULT CDesignerViewBinary::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
   if( GetCapture() != m_hWnd ) return 0;
   CWindowImpl<CDesignerViewBinary>::SendMessage(WM_MOUSEMOVE, wParam, lParam);
   ReleaseCapture();
   return 0;
}

LRESULT CDesignerViewBinary::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
   if( GetCapture() != m_hWnd ) return 0;
   POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
   // Mouse outside client rectangle? Let's scroll the view
   RECT rcClient;
   GetClientRect(&rcClient);
   if( pt.y < 0 && m_dwPos > 0 ) CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, SB_LINEUP);
   if( pt.y > rcClient.bottom - rcClient.top ) CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, SB_LINEDOWN);
   // Expand the selection if mouse is over a valid position?
   bool bInDataPane = false;
   DWORD dwPos = 0;
   if( !GetPosFromPoint(pt, dwPos, bInDataPane) ) return 0;
   if( m_bInDataPane != bInDataPane ) return 0;
   SetSel(m_dwSelStart, dwPos == 0 ? 0 : dwPos + m_iDataSize);
   return 0;
}

LRESULT CDesignerViewBinary::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   SetScrollRange(SB_VERT, 0, (int) (M_DataLen() / BYTES_PR_LINE) - GetLinesPrPage() + 1, TRUE);
   return 0;
}

LRESULT CDesignerViewBinary::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   SCROLLINFO si = { sizeof(SCROLLINFO), SIF_POS | SIF_RANGE | SIF_TRACKPOS };
   GetScrollInfo(SB_VERT, &si);
   int nPos = m_dwPos / BYTES_PR_LINE;
   switch( LOWORD(wParam) ) {
   case SB_TOP:
      nPos = 0;
      break;
   case SB_BOTTOM:
      nPos = si.nMax;
      break;
   case SB_LINEUP:
      if( nPos > 0 ) nPos -= 1;
      break;
   case SB_LINEDOWN:
      nPos += 1;
      break;
   case SB_PAGEUP:
      if( nPos > GetLinesPrPage() ) nPos -= GetLinesPrPage(); else nPos = 0;
      break;
   case SB_PAGEDOWN:
      nPos += GetLinesPrPage();
      break;
   case SB_THUMBTRACK:      
   case SB_THUMBPOSITION:
      nPos = si.nTrackPos;
      break;
   }
   if( nPos < si.nMin ) nPos = si.nMin;
   if( nPos > si.nMax ) nPos = si.nMax;
   if( nPos == si.nPos ) return 0;
   SetScrollPos(SB_VERT, nPos, TRUE);
   m_dwPos = nPos * BYTES_PR_LINE;
   RecalcCaret();
   Invalidate();
   return 0;
}

LRESULT CDesignerViewBinary::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400) || defined(_WIN32_WCE)
   uMsg;
   int zDelta = (int) (short) HIWORD(wParam);
#else
   int zDelta = (uMsg == WM_MOUSEWHEEL) ? (int) (short) HIWORD(wParam) : (int) wParam;
#endif //!((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400) || defined(_WIN32_WCE))
   for( int i = 0; i < abs(zDelta / WHEEL_DELTA * 2); i++ ) CWindowImpl<CDesignerViewBinary>::SendMessage(WM_VSCROLL, zDelta > 0 ? SB_LINEUP : SB_LINEDOWN);
   return 0;
}

LRESULT CDesignerViewBinary::OnNextPane(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_bInDataPane = !m_bInDataPane;
   RecalcCaret();
   Invalidate();
   return 0;
}

LRESULT CDesignerViewBinary::OnEditUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   //Undo();
   return 0;
}

LRESULT CDesignerViewBinary::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
//   DWORD dwStart = 0;
//   DWORD dwEnd = 0;
//   GetSel(dwStart, dwEnd);
//   if( dwStart >= dwEnd ) return 0;
//   if( dwEnd - dwStart > 0x10000 ) {
//      if( IDNO == AtlMessageBox(m_hWnd, IDS_ERR_LARGECLIPBOARD, IDS_CAPTION_ERROR, MB_ICONQUESTION | MB_YESNO) ) return 0;
//   }
//   CWaitCursor cursor;
//   if( !::OpenClipboard(m_hWnd) ) return 0;
//   // Rescale string buffer
//   CString sText;
//   sText.GetBuffer((dwEnd - dwStart) * 4);
//   sText.ReleaseBuffer(0);
//   // Generate text for clipboard
//   LPBYTE pData = m_File.GetData();
//   if( m_bInDataPane ) {
//      TCHAR szBuffer[32];
//      DWORD nCount = 0;
//      for( DWORD i = dwStart; i < dwEnd; i++ ) {
//         ::wsprintf(szBuffer, _T("%02X "), (long) *(pData + i));
//         sText += szBuffer;
//         if( (++nCount % BYTES_PR_LINE) == 0 ) sText += _T("\r\n");
//      }
//   }
//   else {
//      for( DWORD i = dwStart; i < dwEnd; i++ ) {
//         TCHAR ch = *(pData + i);
//         ch = isprint(ch) ? ch : m_cInvalidAscii;
//         sText += ch;
//      }
//   }
//#ifdef _UNICODE
//   LPSTR p = (LPSTR) malloc( (sText.GetLength() + 1) * 2);
//   AtlW2AHelper(p, sText, sText.GetLength() + 1);
//#else
//   LPCSTR p = sText;
//#endif
//   ::EmptyClipboard();
//   HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, strlen(p) + 1);
//   LPSTR lptstrCopy = (LPSTR) GlobalLock(hglbCopy);
//   strcpy(lptstrCopy, p); 
//   GlobalUnlock(hglbCopy);
//   ::SetClipboardData(CF_TEXT, lptstrCopy); 
//   ::CloseClipboard(); 
//#ifdef _UNICODE
//   free(p);
//#endif
   return 0;
}

BOOL CDesignerViewBinary::GetPosFromPoint(POINT pt, DWORD& dwPos, bool& bInDataPane)
{
   ATLASSERT(::IsWindow(m_hWnd));
   ATLASSERT(m_rcData.left>0);   // Oops! Not initialized! Call UpdateWindow() or delay the call!!!
   // Get rectangles for columns. Expand them a bit so it's easier
   // to hit with the mouse...
   RECT rcData = m_rcData;
   RECT rcAscii = m_rcAscii;
   ::InflateRect(&rcData, 4, 0);
   ::InflateRect(&rcAscii, 4, 0);
   if( ::PtInRect(&rcData, pt) ) {
      ::OffsetRect(&rcData, -2, -2);
      int xpos = (pt.x - rcData.left) / (((m_iDataSize * 2) + 1) * m_tmEditor.tmAveCharWidth) * m_iDataSize;
      if( xpos < 0 ) xpos = 0;
      if( xpos > BYTES_PR_LINE - 1 ) xpos = BYTES_PR_LINE - 1;
      int ypos = (pt.y - rcData.top) / GetLineHeight();
      dwPos = m_dwPos + (DWORD) xpos + ((DWORD) ypos * BYTES_PR_LINE);
      bInDataPane = true;
      return TRUE;
   }
   if( ::PtInRect(&rcAscii, pt) ) {
      ::OffsetRect(&rcAscii, 4, 0);
      int xpos = (pt.x - rcAscii.left) / m_tmEditor.tmAveCharWidth;
      if( xpos < 0 ) xpos = 0;
      if( xpos > BYTES_PR_LINE - 1 ) xpos = BYTES_PR_LINE - 1;
      int ypos = (pt.y - rcAscii.top) / GetLineHeight();
      dwPos = m_dwPos + (DWORD) xpos + ((DWORD) ypos * BYTES_PR_LINE);
      bInDataPane = false;
      return TRUE;
   }
   return FALSE;
}

void CDesignerViewBinary::RecalcCaret()
{
   ATLASSERT(::IsWindow(m_hWnd));
   ATLASSERT(m_rcData.left>0 || m_rcAscii.left>0);   // Oops! Not initialized! Call UpdateWindow() or delay the call!!!
   // Selection-mode does not display a caret
   if( m_dwSelStart != m_dwSelEnd ) return;
   // We'll try to determine where to place the caret
   DWORD dwPos = m_dwSelStart;
   int ypos = m_szMargin.cy + (((dwPos - m_dwPos) / BYTES_PR_LINE) * GetLineHeight());
   if( m_bInDataPane ) {
      int xpos = m_rcData.left;
      xpos += (dwPos % BYTES_PR_LINE) / m_iDataSize * m_tmEditor.tmAveCharWidth * ((m_iDataSize * 2) + 1);
      if( m_dwDigitOffset > 0 ) xpos += m_tmEditor.tmAveCharWidth * m_dwDigitOffset;
      ::SetCaretPos(xpos, ypos);
   }
   else {
      int xpos = m_rcAscii.left;
      xpos += ((dwPos % BYTES_PR_LINE) + (m_dwDigitOffset / 2)) * m_tmEditor.tmAveCharWidth;
      ::SetCaretPos(xpos, ypos);
   }
}

void CDesignerViewBinary::RecalcPosition(DWORD dwPos)
{
   // Is new selection-position out of bounds?
   // If so, we need to set a new view position.
   DWORD dwPage = (GetLinesPrPage() - 1) * BYTES_PR_LINE;
   if( dwPos < m_dwPos ) {
      m_dwPos = dwPos - (dwPos % BYTES_PR_LINE);
      SetScrollPos(SB_VERT, m_dwPos / BYTES_PR_LINE);
      Invalidate();
   }
   else if( dwPos > m_dwPos + dwPage ) {
      m_dwPos = dwPos - (dwPos % BYTES_PR_LINE);
      if( m_dwPos >= dwPage ) m_dwPos -= dwPage;
      else m_dwPos = 0;
      SetScrollPos(SB_VERT, m_dwPos / BYTES_PR_LINE);
      Invalidate();
   }
}

void CDesignerViewBinary::AssignDigitValue(DWORD& dwPos, DWORD& dwCursorPos, BYTE bValue)
{
   ATLASSERT(dwPos<M_DataLen());
   ATLASSERT(bValue<0x10);
   // Calculate new data value (byte oriented)
   DWORD dwOffset = dwPos + (m_iDataSize - 1 - (dwCursorPos / 2));
   BYTE bData;
   m_pBinDoc->Data(dwOffset, 1, &bData);
   if( (dwCursorPos % 2) == 0 ) bValue = (BYTE) ((bData & 0x0F) | (bValue << 4));
   else bValue = (BYTE) ((bData & 0xF0) | bValue);
   // Create undo action
   //UNDOENTRY undo = { dwOffset, *(pData + dwOffset) };
   //m_aUndostack.Add(undo);
   // Assign value
   bData = bValue;
   m_pBinDoc->Replace(dwOffset, 1, &bData);
   // Advance cursor (might need to only move the caret to next digit).
   DWORD dwTotalDigits = m_iDataSize * 2;
   if( ++m_dwDigitOffset >= dwTotalDigits ) SetSel(dwPos + m_iDataSize, dwPos + m_iDataSize);
   else RecalcCaret();
   Invalidate();
}

void CDesignerViewBinary::AssignCharValue(DWORD& dwPos, DWORD& dwCursorPos, BYTE bValue)
{
   ATLASSERT(dwPos<M_DataLen());
   // Calculate new data value (cursor moves one digit; a byte i 2 digits)
   DWORD dwOffset = dwPos + (dwCursorPos / 2);
   BYTE bData;
   m_pBinDoc->Data(dwOffset, 1, &bData);
   // Create undo action
   //UNDOENTRY undo = { dwOffset, *(pData + dwOffset) };
   //m_aUndostack.Add(undo);
   // Assign value
   bData = bValue;
   m_pBinDoc->Replace(dwOffset, 1, &bData);
   // Advance cursor (probably to next 'char' only)
   dwCursorPos += 2;
   DWORD dwTotalDigits = m_iDataSize * 2;
   if( dwCursorPos >= dwTotalDigits ) SetSel(dwPos + m_iDataSize, dwPos + m_iDataSize);
   else RecalcCaret();
   Invalidate();
}

void CDesignerViewBinary::DoPaint(CDCHandle dc)
{
   RECT rcClient;
   GetClientRect(&rcClient);

   dc.FillSolidRect(&rcClient, ::GetSysColor(COLOR_WINDOW));

   rcClient.left += m_szMargin.cx;
   rcClient.top += m_szMargin.cy;

   HFONT hOldFont = dc.SelectFont(m_fontEditor);
   int nLines = GetLinesPrPage() + 1;
   int iHeight = GetLineHeight();

   DWORD dwSize = M_DataLen();
   CAutoVectorPtr<BYTE> pData(dwSize ? new BYTE[dwSize] : NULL);
   m_pBinDoc->Data(0, dwSize, pData);
   DWORD dwPos = m_dwPos;

   ::ZeroMemory(&m_rcData, sizeof(m_rcData));
   ::ZeroMemory(&m_rcAscii, sizeof(m_rcAscii));
   m_rcData.top = m_rcAscii.top = m_szMargin.cy;

   COLORREF clrTextH, clrBackH;
   COLORREF clrTextN = ::GetSysColor(m_bReadOnly ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT);
   COLORREF clrBackN = ::GetSysColor(COLOR_WINDOW);
   bool bHighlighted = false;

   dc.SetBkMode(OPAQUE);

   DWORD dwSelStart = 0;
   DWORD dwSelEnd = 0;
   GetSel(dwSelStart, dwSelEnd);

   int ypos = rcClient.top;
   TCHAR szBuffer[64] = { 0 };
   for( int i = 0; i < nLines; i++ ) {
      int xpos = rcClient.left;
      // Draw address text
      if( m_bShowAddress && dwPos < dwSize ) 
      {
         dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
         dc.SetBkColor(clrBackN);
         ::wsprintf(szBuffer, _T("%08X  "), dwPos);
         RECT rcAddress = { xpos, ypos, xpos + 200, ypos + iHeight };
         dc.DrawText(szBuffer, -1, &rcAddress, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
         xpos += m_tmEditor.tmAveCharWidth * _tcslen(szBuffer);
      }
      // Draw hex values
      if( m_bShowData ) 
      {
         if( m_rcData.left == 0 ) m_rcData.left = xpos;

         clrBackH = ::GetSysColor(m_bInDataPane ? COLOR_HIGHLIGHT : COLOR_BTNFACE);
         clrTextH = ::GetSysColor(m_bInDataPane ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);

         dc.SetTextColor(clrTextN);
         dc.SetBkColor(clrBackN);
         bHighlighted = false;

         for( DWORD j = 0; j < BYTES_PR_LINE; j += m_iDataSize ) {
            if( dwPos + j >= dwSize ) break;

            if( dwPos + j >= dwSelStart && dwPos + j < dwSelEnd ) {
               if( !bHighlighted ) {
                  dc.SetTextColor(clrTextH);
                  dc.SetBkColor(clrBackH);
                  bHighlighted = true;
               }
            }
            else {
               if( bHighlighted ) {
                  dc.SetTextColor(clrTextN);
                  dc.SetBkColor(clrBackN);
                  bHighlighted = false;
               }
            }

            LPTSTR p = szBuffer + (j == 0 ? 0 : 1);
            szBuffer[0] = ' ';
            switch( m_iDataSize ) {
            case 1:
               ::wsprintf( p, _T("%02X"), (long) *(pData + dwPos + j) );
               break;
            case 2:
               ::wsprintf( p, _T("%04X"), (long) *(LPWORD) (pData + dwPos + j) );
               break;
            case 4:
               ::wsprintf( p, _T("%08X"), (long) *(LPDWORD) (pData + dwPos + j) );
               break;
            default:
               ATLASSERT(false);
            }
            RECT rcData = { xpos, ypos, xpos + 200, ypos + iHeight };
            dc.DrawText(szBuffer, -1, &rcData, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
            xpos += m_tmEditor.tmAveCharWidth * _tcslen(szBuffer);
         }

         if( m_rcData.right == 0 ) m_rcData.right = xpos;
      }
      // Draw ASCII representation
      if( m_bShowAscii )
      {
         xpos += m_tmEditor.tmAveCharWidth * 3;

         if( m_rcAscii.left == 0 ) m_rcAscii.left = xpos;
         xpos = m_rcAscii.left;

         clrBackH = ::GetSysColor(m_bInDataPane ? COLOR_BTNFACE : COLOR_HIGHLIGHT);
         clrTextH = ::GetSysColor(m_bInDataPane ? COLOR_WINDOWTEXT : COLOR_HIGHLIGHTTEXT);

         dc.SetTextColor(clrTextN);
         dc.SetBkColor(clrBackN);
         bHighlighted = false;

         DWORD j = 0;
         for( ; j < BYTES_PR_LINE; j++ ) {
            if( dwPos + j >= dwSize ) break;

            if( dwPos + j >= dwSelStart && dwPos + j < dwSelEnd ) {
               if( !bHighlighted ) {
                  dc.SetTextColor(clrTextH);
                  dc.SetBkColor(clrBackH);
                  bHighlighted = true;
               }
            }
            else {
               if( bHighlighted ) {
                  dc.SetTextColor(clrTextN);
                  dc.SetBkColor(clrBackN);
                  bHighlighted = false;
               }
            }

            TCHAR ch = *(pData + dwPos + j);
            ch = isprint(ch) ? ch : m_cInvalidAscii;
            RECT rcAscii = { xpos, ypos, xpos + 100, ypos + iHeight };
            dc.DrawText(&ch, 1, &rcAscii, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);
            xpos += m_tmEditor.tmAveCharWidth;
         }

         if( m_rcAscii.right == 0 ) m_rcAscii.right = xpos;
      }
      dwPos += BYTES_PR_LINE;
      ypos += iHeight;
   }

   dc.SelectFont(hOldFont);

   m_rcData.bottom = m_rcAscii.bottom = ypos;

   // HACK: Delayed activation of first caret position.
   //       We need the sizes of m_rcData and m_rcAscii before
   //       we can set selection (position caret)!
   if( m_dwSelStart == (DWORD) -1 ) SetSel(0, 0);
} 

