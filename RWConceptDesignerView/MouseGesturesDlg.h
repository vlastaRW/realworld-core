
#pragma once

#include <Win32LangEx.h>
#include <XPGUI.h>
#include <DPIUtils.h>
#include <ContextHelpDlg.h>

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_arrowhead.h>
#include <agg_conv_marker.h>
#include <agg_vcgen_markers_term.h>
#include <agg_ellipse.h>
//#include <agg_gamma_lut.h>

extern __declspec(selectany) LONG g_nMGDPosistionX = CW_USEDEFAULT;
extern __declspec(selectany) LONG g_nMGDPosistionY = CW_USEDEFAULT;
extern __declspec(selectany) LONG g_nMGDSizeX = CW_USEDEFAULT;
extern __declspec(selectany) LONG g_nMGDSizeY = CW_USEDEFAULT;

class CMouseGesturesDlg :
	public Win32LangEx::CPreTranslateModalLangDialogImpl<CMouseGesturesDlg>,
	public CDialogResize<CMouseGesturesDlg>,
	public CContextHelpDlg<CMouseGesturesDlg>
{
public:
	CMouseGesturesDlg(LCID a_tLocaleID, IConfig* a_pConfig) :
		Win32LangEx::CPreTranslateModalLangDialogImpl<CMouseGesturesDlg>(a_tLocaleID),
		CContextHelpDlg<CMouseGesturesDlg>(_T("http://www.rw-designer.com/configure-mouse-gestures")),
		m_hIcon(NULL), m_pConfig(a_pConfig), m_nGestureImageSize(0), m_nActiveGesture(-1)
	{
	}
	~CMouseGesturesDlg()
	{
		if (m_hIcon) DestroyIcon(m_hIcon);
	}

	bool DoModalPreTransalate(MSG const* a_pMsg)
	{
		if (!m_pCfgWnd)
			return false;
		return m_pCfgWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK || m_pCfgWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK;
	}

	enum {IDD = IDD_CONFIGUREGESTURES};

	BEGIN_MSG_MAP(CMouseGesturesDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CMouseGesturesDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
		COMMAND_HANDLER(IDC_MG_GESTURES, LBN_SELCHANGE, OnGestureSelChanged)
		COMMAND_HANDLER(IDC_MG_OPERATION, CBN_SELCHANGE, OnOperationSelChanged)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		CHAIN_MSG_MAP(CDialogResize<CMouseGesturesDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CMouseGesturesDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_MG_GESTURES, DLSZ_MULDIVSIZE_X(1, 4) | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_MG_LABEL, DLSZ_MULDIVMOVE_X(1, 4))
		DLGRESIZE_CONTROL(IDC_MG_OPERATION, DLSZ_MULDIVMOVE_X(1, 4) | DLSZ_MULDIVSIZE_X(3, 4))
		DLGRESIZE_CONTROL(IDC_MG_CONFIG, DLSZ_MULDIVMOVE_X(1, 4) | DLSZ_MULDIVSIZE_X(3, 4) | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CMouseGesturesDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_MG_GESTURES, IDS_MG_GESTURES_HELP)
		CTXHELP_CONTROL_RESOURCE(IDC_MG_OPERATION, IDS_MG_OPERATION_HELP)
	END_CTXHELP_MAP()

	// message handlers
public:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		m_wndGestures = GetDlgItem(IDC_MG_GESTURES);
		m_wndOperation = GetDlgItem(IDC_MG_OPERATION);
		// center the dialog on the screen
		CenterWindow();

		try
		{
			//m_hIcon = DPIUtils::PrepareIconForCaption((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(pParams->dwFlags&EFTCreateNew ? IDI_FILE_SAVE : ID_FWFILE_FOLDER), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR));
			//SetIcon(m_hIcon, 0);

			CWindow wnd = GetDlgItem(IDC_MG_CONFIG);
			RECT rc;
			wnd.GetWindowRect(&rc);
			ScreenToClient(&rc);
			RWCoCreateInstance(m_pCfgWnd, __uuidof(AutoConfigWnd));
			m_pCfgWnd->Create(m_hWnd, &rc, IDC_MG_CONFIG, m_tLocaleID, TRUE, ECWBMNothing);
			wnd.DestroyWindow();

			for (SGestureSpecification const* p = g_aGestureSpecifications; p != g_aGestureSpecifications+itemsof(g_aGestureSpecifications); ++p)
			{
				m_wndGestures.AddString(reinterpret_cast<LPCTSTR>(p-g_aGestureSpecifications));
			}
			m_wndGestures.SetCurSel(0);
			SetActiveGesture(m_nActiveGesture = 0);

			DlgResize_Init();

			//CConfigValue cPosistionX;
			//CConfigValue cPosistionY;
			//CConfigValue cSizeX;
			//CConfigValue cSizeY;
			//m_pContextConfig->ItemValueGet(CComBSTR(CFGID_POSITIONX), &cPosistionX);
			//m_pContextConfig->ItemValueGet(CComBSTR(CFGID_POSITIONY), &cPosistionY);
			//m_pContextConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cSizeX);
			//m_pContextConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cSizeY);
			if (g_nMGDPosistionX != CW_USEDEFAULT && g_nMGDPosistionY != CW_USEDEFAULT &&
				g_nMGDSizeX != CW_USEDEFAULT && g_nMGDSizeY != CW_USEDEFAULT)
			{
				MoveWindow(g_nMGDPosistionX, g_nMGDPosistionY, g_nMGDSizeX, g_nMGDSizeY, FALSE);
			}
		}
		catch (...)
		{
		}

		return TRUE;
	}

	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		a_bHandled = FALSE;

		try
		{
			WINDOWPLACEMENT tPos;
			if (GetWindowPlacement(&tPos))
			{
				g_nMGDPosistionX = tPos.rcNormalPosition.left;
				g_nMGDPosistionY = tPos.rcNormalPosition.top;
				g_nMGDSizeX = tPos.rcNormalPosition.right-tPos.rcNormalPosition.left;
				g_nMGDSizeY = tPos.rcNormalPosition.bottom-tPos.rcNormalPosition.top;
		//		CComBSTR cCFGID_POSITIONX(CFGID_POSITIONX);
		//		CComBSTR cCFGID_POSITIONY(CFGID_POSITIONY);
		//		CComBSTR cCFGID_SIZEX(CFGID_SIZEX);
		//		CComBSTR cCFGID_SIZEY(CFGID_SIZEY);
		//		TConfigValue aVals[4];
		//		BSTR aIDs[4];
		//		aIDs[0] = cCFGID_POSITIONX;	aVals[0] = CConfigValue((LONG)tPos.rcNormalPosition.left);
		//		aIDs[1] = cCFGID_POSITIONY;	aVals[1] = CConfigValue((LONG)tPos.rcNormalPosition.top);
		//		aIDs[2] = cCFGID_SIZEX;		aVals[2] = CConfigValue((LONG)(tPos.rcNormalPosition.right-tPos.rcNormalPosition.left));
		//		aIDs[3] = cCFGID_SIZEY;		aVals[3] = CConfigValue((LONG)(tPos.rcNormalPosition.bottom-tPos.rcNormalPosition.top));
		//		m_pContextConfig->ItemValuesSet(4, aIDs, aVals);
			}
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnOK(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		EndDialog(IDOK);
		return 0;
	}
	LRESULT OnCancel(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		EndDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnGestureSelChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int i = m_wndGestures.GetCurSel();
		if (m_nActiveGesture != i)
			SetActiveGesture(m_nActiveGesture = i);
		return 0;
	}
	LRESULT OnOperationSelChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int i = m_wndOperation.GetCurSel();
		if (i >= 0 && m_nActiveGesture != -1 && m_pOptions)
		{
			CConfigValue cVal;
			m_pOptions->Get(m_wndOperation.GetItemData(i), &cVal);
			OLECHAR szID[64];
			wcscpy(szID, CFGID_2DEDIT_GESTUREPREFIX);
			wcscat(szID, g_aGestureSpecifications[m_nActiveGesture].szCode);
			CComBSTR bstrID(szID);
			m_pConfig->ItemValuesSet(1, &(bstrID.m_str), cVal);
		}
		return 0;
	}

	LRESULT OnDrawItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LPDRAWITEMSTRUCT pDrawItem = reinterpret_cast<LPDRAWITEMSTRUCT>(a_lParam);
		if (a_wParam == IDC_MG_GESTURES && pDrawItem->CtlID == IDC_MG_GESTURES && pDrawItem->itemID < m_wndGestures.GetCount())
		{
			SGestureSpecification const* pGesture = g_aGestureSpecifications+pDrawItem->itemID;
			CDCHandle cDC(pDrawItem->hDC);
			RECT rcItem = {0, 0, 0, 0};
			m_wndGestures.GetItemRect(pDrawItem->itemID, &rcItem);
			{
				RECT rc;
				m_wndGestures.GetClientRect(&rc);
				if (rcItem.right < rc.right)
					rcItem.right = rc.right;
			}
			cDC.FillRect(&rcItem, pDrawItem->itemState&ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW);
			RECT rcText;
			rcText.top = rcItem.top+1;
			rcText.bottom = rcItem.bottom-1;
			rcText.left = rcItem.left+(m_nGestureImageSize>>3)+m_nGestureImageSize;
			rcText.right = rcItem.right-(m_nGestureImageSize>>3);
			HFONT hOldFont = cDC.SelectFont(GetFont());
			COLORREF clrFg = GetSysColor(pDrawItem->itemState&ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);
			COLORREF clrBg = GetSysColor(pDrawItem->itemState&ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW);
			COLORREF clrOldText = cDC.SetTextColor(clrFg);
			COLORREF clrOldBg = cDC.SetBkColor(clrBg);
			TCHAR szItem[64] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), pGesture->nIDS, szItem, itemsof(szItem), LANGIDFROMLCID(m_tLocaleID));
			cDC.DrawText(szItem, -1, &rcText, DT_VCENTER|DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS);
			cDC.SetTextColor(clrOldText);
			cDC.SetBkColor(clrOldBg);
			cDC.SelectFont(hOldFont);

			CAutoVectorPtr<BYTE> pData(new BYTE[m_nGestureImageSize*m_nGestureImageSize*4]);

			agg::rendering_buffer rbuf;
			rbuf.attach(pData.m_p, m_nGestureImageSize, m_nGestureImageSize, -m_nGestureImageSize*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
			// Pixel format and basic primitives renderer
			agg::pixfmt_bgra32 pixf(rbuf);
			agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
			renb.clear(agg::rgba8(GetRValue(clrBg), GetGValue(clrBg), GetBValue(clrBg), 255));
			// Scanline renderer for solid filling.
			agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
			// Rasterizer & scanline
			agg::rasterizer_scanline_aa<> ras;
			agg::scanline_p8 sl;

			{
				agg::path_storage path;
				double xs = 0.0;
				double ys = 0.0;
				for (POINT const* pP = pGesture->aPoints; pP->x >= 0 && pP->y >= 0; ++pP)
				{
					double x = ((pP->x+32)*m_nGestureImageSize)/320.0;
					double y = ((pP->y+32)*m_nGestureImageSize)/320.0;
					if (pP == pGesture->aPoints)
						path.move_to(xs = x, ys = y);
					else
						path.line_to(x, y);
				}
				agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> stroke(path);
				//stroke.shorten(5.0);
				stroke.line_join(agg::round_join);
				stroke.width(m_nGestureImageSize/12.0);
				ren.color(agg::rgba8(GetRValue(clrFg), GetGValue(clrFg), GetBValue(clrFg), 255));
				ras.add_path(stroke);
				agg::arrowhead ah;
				ah.head(m_nGestureImageSize/10.0, m_nGestureImageSize/10.0, m_nGestureImageSize/6.0, 0);
				agg::conv_marker<typename agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term>::marker_type, agg::arrowhead> arrow(stroke.markers(), ah);
				ras.add_path(arrow);
				agg::ellipse e(xs, ys, m_nGestureImageSize/10.0, m_nGestureImageSize/10.0, 16);
				ras.add_path(e);
				agg::render_scanlines(ras, sl, ren);
			}


			BITMAPINFO bmp_info;
			ZeroMemory(&bmp_info, sizeof bmp_info);
			bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmp_info.bmiHeader.biWidth = m_nGestureImageSize;
			bmp_info.bmiHeader.biHeight = m_nGestureImageSize;
			bmp_info.bmiHeader.biPlanes = 1;
			bmp_info.bmiHeader.biBitCount = 32;
			bmp_info.bmiHeader.biCompression = BI_RGB;

			::SetDIBitsToDevice(cDC, rcItem.left, rcItem.top, m_nGestureImageSize, m_nGestureImageSize, 0, 0, 0, m_nGestureImageSize, pData.m_p, &bmp_info, 0);

			return 1;
		}
		return 0;
	}
	LRESULT OnMeasureItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LPMEASUREITEMSTRUCT pMeasureItem = reinterpret_cast<LPMEASUREITEMSTRUCT>(a_lParam);
		if (a_wParam == IDC_MG_GESTURES && pMeasureItem->CtlID == IDC_MG_GESTURES)
		{
			pMeasureItem->itemWidth = 128;
			if (m_nGestureImageSize == 0)
			{
				HDC hDC = GetDC();
				int nDPI = GetDeviceCaps(hDC, LOGPIXELSX);
				ReleaseDC(hDC);
				m_nGestureImageSize = (((32*nDPI+48)/96)&~1)+1;
			}
			pMeasureItem->itemHeight = m_nGestureImageSize;
			return 1;
		}
		return 0;
	}

private:
	void SetActiveGesture(int a_nActiveGesture)
	{
		OLECHAR szID[64];
		wcscpy(szID, CFGID_2DEDIT_GESTUREPREFIX);
		wcscat(szID, g_aGestureSpecifications[a_nActiveGesture].szCode);
		CComBSTR bstrID(szID);
		CConfigValue cID;
		m_pConfig->ItemValueGet(bstrID, &cID);

		CComPtr<IConfigItemOptions> pItem;
		m_pConfig->ItemGetUIInfo(bstrID, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
		CComPtr<IEnumConfigItemOptions> pOptions;
		if (pItem == NULL || FAILED(pItem->OptionsEnum(&pOptions)) || pOptions == NULL)
		{
			m_wndOperation.EnableWindow(FALSE);
			return;
		}

		m_wndOperation.EnableWindow(TRUE);
		m_wndOperation.ResetContent();

		CConfigValue cOption;
		for (ULONG i = 0; SUCCEEDED(pOptions->Get(i, &cOption)); ++i)
		{
			int iCombo = -1;
			CComBSTR bstrText;
			CComPtr<ILocalizedString> pStr;
			if (SUCCEEDED(pItem->ValueGetName(cOption, &pStr)) &&
				SUCCEEDED(pStr->GetLocalized(m_tLocaleID, &bstrText)))
			{
				iCombo = m_wndOperation.AddString(COLE2CT(bstrText));
			}
			else
			{
				iCombo = m_wndOperation.AddString(_T("name unavailable"));
			}
			m_wndOperation.SetItemData(iCombo, i);
			if (cOption == cID)
				m_wndOperation.SetCurSel(iCombo);
			m_pOptions = pOptions;
		}

		CComPtr<IConfig> pCfg;
		m_pConfig->SubConfigGet(bstrID, &pCfg);
		m_pCfgWnd->ConfigSet(pCfg);
	}

private:
	CListBox m_wndGestures;
	CComboBox m_wndOperation;
	CComPtr<IEnumConfigItemOptions> m_pOptions;
	int m_nGestureImageSize;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IConfigWnd> m_pCfgWnd;
	HICON m_hIcon;
	int m_nActiveGesture;
};
