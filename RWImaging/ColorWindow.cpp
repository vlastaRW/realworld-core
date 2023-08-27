// ColorWindow.cpp : Implementation of CColorWindow

#include "stdafx.h"
#include "ColorWindow.h"
#include <MultiLanguageString.h>
#include <Win32LangEx.h>
#include <ContextHelpDlg.h>
#include <WTL_ColorArea.h>
#include "ColorHistory.h"

extern __declspec(selectany) TCHAR const COLORAREAHORZAWNDCLASS[] = _T("WTL_ColorAreaHorzA");
extern __declspec(selectany) TCHAR const COLORAREA1DFLEXWNDCLASS[] = _T("WTL_ColorArea1DFlex");
extern __declspec(selectany) TCHAR const COLORAREA2DFLEXWNDCLASS[] = _T("WTL_ColorArea2DFlex");
extern __declspec(selectany) wchar_t const HELP_COLORCHANNEL[] = L"[0409]Select color component used by the color areas to the left.[0405]Vyberte komponentu pro barevné oblasti vlevo.";
extern __declspec(selectany) wchar_t const HELP_COLORNUMBER[] = L"[0409]Enter value for a single color component. Range and accuracy can be set in application options.[0405]Zadejte hodnotu pro barevnou komonentu. Rozsah a přesnost lze změnit v nastaveních aplikace.";


class CDialogColorPicker :
	public Win32LangEx::CLangIndirectDialogImpl<CDialogColorPicker>,
	public CDialogResize<CDialogColorPicker>,
	public CContextHelpDlg<CDialogColorPicker>
{
public:
	CDialogColorPicker(float a_fR, float a_fG, float a_fB, float a_fA, bool a_bAlphaChannel = true, LCID a_tLocaleID = GetThreadLocale(), IConfig* a_pGlobalColorCfg = NULL) :
		Win32LangEx::CLangIndirectDialogImpl<CDialogColorPicker>(a_tLocaleID), m_bEnableUpdates(true),
		m_pColorsCfg(a_pGlobalColorCfg), m_bAlphaChannel(a_bAlphaChannel)
	{
		if (m_pColorsCfg == NULL)
		{
			CComPtr<IGlobalConfigManager> pGCM;
			RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
			if (pGCM)
			{
				static GUID const tColorGCID = {0x2CBE06C7, 0x4847, 0x4766, {0xAA, 0x01, 0x22, 0x6A, 0xF5, 0x2D, 0x54, 0x88}};
				pGCM->Config(tColorGCID, &m_pColorsCfg);
			}
		}
		m_aColor[EChR] = a_fR;
		m_aColor[EChG] = a_fG;
		m_aColor[EChB] = a_fB;
		CColorBaseHLSA c;
		c.SetRGB(a_fR, a_fG, a_fB);
		m_aColor[EChH] = c.GetH();
		m_aColor[EChL] = c.GetL();
		m_aColor[EChS] = c.GetS();
		m_aColor[EChA] = a_fA;
		m_tInitial.fR = a_fR;
		m_tInitial.fG = a_fG;
		m_tInitial.fB = a_fB;
		m_tInitial.fA = a_fA;
	}

	float M_R() const { return m_aColor[EChR]; }
	float M_G() const { return m_aColor[EChG]; }
	float M_B() const { return m_aColor[EChB]; }
	float M_A() const { return m_aColor[EChA]; }

	enum EChannel
	{
		EChR = 0,
		EChG,
		EChB,
		EChH,
		EChL,
		EChS,
		EChA,
		EChCnt
	};

	enum
	{
		IDC_CPF_2DAREA = 100, IDC_CPF_1DAREA, IDC_CPF_ALPHAAREA,
		IDC_CPF_RADIO_R, IDC_CPF_EDIT_R, IDC_CPF_SPIN_R,
		IDC_CPF_RADIO_G, IDC_CPF_EDIT_G, IDC_CPF_SPIN_G,
		IDC_CPF_RADIO_B, IDC_CPF_EDIT_B, IDC_CPF_SPIN_B,
		IDC_CPF_SEP1,
		IDC_CPF_RADIO_H, IDC_CPF_EDIT_H, IDC_CPF_SPIN_H,
		IDC_CPF_RADIO_L, IDC_CPF_EDIT_L, IDC_CPF_SPIN_L,
		IDC_CPF_RADIO_S, IDC_CPF_EDIT_S, IDC_CPF_SPIN_S,
		IDC_CPF_SEP2,
		IDC_CPF_LABEL_A, IDC_CPF_EDIT_A, IDC_CPF_SPIN_A,
		IDC_CPF_HISTORY_SEP, IDC_CPF_HISTORY,
	};

BEGIN_DIALOG_EX(0, 0, 205, 168, 0)
	DIALOG_FONT_AUTO()
	DIALOG_STYLE(WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_CLIPCHILDREN|WS_CLIPSIBLINGS)
	DIALOG_EXSTYLE(WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT)
	DIALOG_CAPTION(_T("[0409]Choose Color[0405]Vybrat barvu"))
END_DIALOG()

BEGIN_CONTROLS_MAP()
	CONTROL_CONTROL(_T(""), IDC_CPF_2DAREA, WC_STATIC, SS_BLACKRECT | WS_CLIPCHILDREN | WS_VISIBLE, 7, 7, 112, 99, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_1DAREA, WC_STATIC, SS_BLACKRECT | WS_CLIPCHILDREN | WS_VISIBLE, 123, 7, 12, 99, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_ALPHAAREA, WC_STATIC, SS_BLACKRECT | WS_CLIPCHILDREN | WS_VISIBLE, 7, 110, 112, 12, 0)
	CONTROL_AUTORADIOBUTTON(_T("R"), IDC_CPF_RADIO_R, 141, 8, 21, 10, WS_VISIBLE, 0)
	CONTROL_AUTORADIOBUTTON(_T("G"), IDC_CPF_RADIO_G, 141, 24, 21, 10, WS_VISIBLE, 0)
	CONTROL_AUTORADIOBUTTON(_T("B"), IDC_CPF_RADIO_B, 141, 40, 21, 10, WS_VISIBLE, 0)
	CONTROL_AUTORADIOBUTTON(_T("H"), IDC_CPF_RADIO_H, 141, 59, 21, 10, WS_VISIBLE, 0)
	CONTROL_AUTORADIOBUTTON(_T("L"), IDC_CPF_RADIO_L, 141, 75, 21, 10, WS_VISIBLE, 0)
	CONTROL_AUTORADIOBUTTON(_T("S"), IDC_CPF_RADIO_S, 141, 91, 21, 10, WS_VISIBLE, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_R, 163, 7, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_R, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 7, 10, 12, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_G, 163, 23, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_G, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 23, 10, 12, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_B, 163, 39, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_B, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 39, 10, 12, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SEP1, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 143, 54, 53, 1, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_H, 163, 58, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_H, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 58, 10, 12, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_L, 163, 74, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_L, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 74, 10, 12, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_S, 163, 90, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_S, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 90, 10, 12, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SEP2, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 143, 106, 53, 1, 0)
	CONTROL_LTEXT(_T("A"), IDC_CPF_LABEL_A, 151, 112, 8, 8, WS_VISIBLE, 0)
	CONTROL_EDITTEXT(IDC_CPF_EDIT_A, 163, 110, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_SPIN_A, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 188, 110, 10, 12, 0)
	CONTROL_CONTROL(_T(""), IDC_CPF_HISTORY_SEP, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 7, 126, 191, 1, 0)
	CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 94, 147, 50, 14, WS_VISIBLE, 0)
	CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDCANCEL, 148, 147, 50, 14, WS_VISIBLE, 0)
	CONTROL_PUSHBUTTON(_T("[0409]Help[0405]Nápověda"), IDHELP, 7, 147, 50, 14, WS_VISIBLE, 0)
END_CONTROLS_MAP()

BEGIN_MSG_MAP(CDialogColorPicker)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP(CContextHelpDlg<CDialogColorPicker>)
	COMMAND_HANDLER(IDC_CPF_RADIO_R, BN_CLICKED, OnChannelClicked<EChR>)
	COMMAND_HANDLER(IDC_CPF_RADIO_G, BN_CLICKED, OnChannelClicked<EChG>)
	COMMAND_HANDLER(IDC_CPF_RADIO_B, BN_CLICKED, OnChannelClicked<EChB>)
	COMMAND_HANDLER(IDC_CPF_RADIO_H, BN_CLICKED, OnChannelClicked<EChH>)
	COMMAND_HANDLER(IDC_CPF_RADIO_L, BN_CLICKED, OnChannelClicked<EChL>)
	COMMAND_HANDLER(IDC_CPF_RADIO_S, BN_CLICKED, OnChannelClicked<EChS>)
	COMMAND_HANDLER(IDC_CPF_EDIT_R, EN_CHANGE, OnEditChanged<EChR>)
	COMMAND_HANDLER(IDC_CPF_EDIT_G, EN_CHANGE, OnEditChanged<EChG>)
	COMMAND_HANDLER(IDC_CPF_EDIT_B, EN_CHANGE, OnEditChanged<EChB>)
	COMMAND_HANDLER(IDC_CPF_EDIT_H, EN_CHANGE, OnEditChanged<EChH>)
	COMMAND_HANDLER(IDC_CPF_EDIT_L, EN_CHANGE, OnEditChanged<EChL>)
	COMMAND_HANDLER(IDC_CPF_EDIT_S, EN_CHANGE, OnEditChanged<EChS>)
	COMMAND_HANDLER(IDC_CPF_EDIT_A, EN_CHANGE, OnEditChanged<EChA>)
	COMMAND_HANDLER(IDC_CPF_EDIT_R, EN_KILLFOCUS, OnEditKillFocus<EChR>)
	COMMAND_HANDLER(IDC_CPF_EDIT_G, EN_KILLFOCUS, OnEditKillFocus<EChG>)
	COMMAND_HANDLER(IDC_CPF_EDIT_B, EN_KILLFOCUS, OnEditKillFocus<EChB>)
	COMMAND_HANDLER(IDC_CPF_EDIT_H, EN_KILLFOCUS, OnEditKillFocus<EChH>)
	COMMAND_HANDLER(IDC_CPF_EDIT_L, EN_KILLFOCUS, OnEditKillFocus<EChL>)
	COMMAND_HANDLER(IDC_CPF_EDIT_S, EN_KILLFOCUS, OnEditKillFocus<EChS>)
	COMMAND_HANDLER(IDC_CPF_EDIT_A, EN_KILLFOCUS, OnEditKillFocus<EChA>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_R, UDN_DELTAPOS, OnUpDownChange<EChR>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_G, UDN_DELTAPOS, OnUpDownChange<EChG>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_B, UDN_DELTAPOS, OnUpDownChange<EChB>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_H, UDN_DELTAPOS, OnUpDownChange<EChH>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_L, UDN_DELTAPOS, OnUpDownChange<EChL>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_S, UDN_DELTAPOS, OnUpDownChange<EChS>)
	NOTIFY_HANDLER(IDC_CPF_SPIN_A, UDN_DELTAPOS, OnUpDownChange<EChA>)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
	COMMAND_HANDLER(IDHELP, BN_CLICKED, OnHelpClick)
	CHAIN_MSG_MAP(CDialogResize<CDialogColorPicker>)
	NOTIFY_HANDLER(IDC_CPF_2DAREA, CColorArea2DFlex::CAN_COLOR_CHANGED, OnColorArea2DChanged)
	NOTIFY_HANDLER(IDC_CPF_1DAREA, CColorArea1DFlex::CAN_COLOR_CHANGED, OnColorArea1DChanged)
	NOTIFY_HANDLER(IDC_CPF_ALPHAAREA, CColorAreaHorzA::CAN_COLOR_CHANGED, OnColorAreaAlphaChanged)
	NOTIFY_HANDLER(IDC_CPF_HISTORY, CColorHistory2::CH_COLOR_CHANGED, OnColorHistoryColorChanged)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CDialogColorPicker)
	DLGRESIZE_CONTROL(IDC_CPF_2DAREA, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_CPF_1DAREA, DLSZ_MOVE_X|DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_CPF_ALPHAAREA, DLSZ_SIZE_X|DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_CPF_SEP1, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SEP2, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_RADIO_R, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_R, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_R, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_RADIO_G, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_G, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_G, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_RADIO_B, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_B, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_B, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_RADIO_H, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_H, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_H, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_RADIO_L, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_L, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_L, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_RADIO_S, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_S, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_S, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_LABEL_A, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_EDIT_A, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_SPIN_A, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_CPF_HISTORY_SEP, DLSZ_SIZE_X|DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_CPF_HISTORY, DLSZ_SIZE_X|DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

BEGIN_CTXHELP_MAP(CDesignerViewColorAreas)
	CTXHELP_CONTROL_STRING(IDC_CPF_2DAREA, L"[0409]Click with mouse or use arrow keys to change color components complementary to the selected one.[0405]Klikněte myší nebo použijte klávesy šipek ke změně barevných komponent komplementárních ke zvolené.")
	CTXHELP_CONTROL_STRING(IDC_CPF_1DAREA, L"[0409]Click with mouse or use up and down arrow keys to change the selected color component.[0405]Klikněte myší nebo použijte klávesy nahoru a dolů ke změně zvolené barevné komponenty.")
	CTXHELP_CONTROL_STRING(IDC_CPF_ALPHAAREA, L"[0409]Click with mouse or use left and right arrow keys to change color opacity (alpha).[0405]Klikněte myší nebo použijte klávesy vlevo a vpravo ke změně krytí (alfy).")
	CTXHELP_CONTROL_STRING(IDC_CPF_RADIO_R, HELP_COLORCHANNEL)
	CTXHELP_CONTROL_STRING(IDC_CPF_RADIO_G, HELP_COLORCHANNEL)
	CTXHELP_CONTROL_STRING(IDC_CPF_RADIO_B, HELP_COLORCHANNEL)
	CTXHELP_CONTROL_STRING(IDC_CPF_RADIO_H, HELP_COLORCHANNEL)
	CTXHELP_CONTROL_STRING(IDC_CPF_RADIO_L, HELP_COLORCHANNEL)
	CTXHELP_CONTROL_STRING(IDC_CPF_RADIO_S, HELP_COLORCHANNEL)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_R, HELP_COLORNUMBER)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_G, HELP_COLORNUMBER)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_B, HELP_COLORNUMBER)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_H, HELP_COLORNUMBER)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_L, HELP_COLORNUMBER)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_S, HELP_COLORNUMBER)
	CTXHELP_CONTROL_STRING(IDC_CPF_EDIT_A, HELP_COLORNUMBER)
END_CTXHELP_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		if (m_pColorsCfg)
		{
			CConfigValue cCh;
			m_pColorsCfg->ItemValueGet(CComBSTR(L"PDChannel"), &cCh);
			static int const aIDs[] = {IDC_CPF_RADIO_R, IDC_CPF_RADIO_G, IDC_CPF_RADIO_B, IDC_CPF_RADIO_H, IDC_CPF_RADIO_L, IDC_CPF_RADIO_S};
			if (cCh.TypeGet() == ECVTInteger && cCh.operator LONG() >= 0 && cCh.operator LONG() < LONG(itemsof(aIDs)))
			{
				m_wndArea2D.SetActiveChannel(static_cast<EChannel>(cCh.operator LONG()));
				m_wndArea1D.SetActiveChannel(static_cast<EChannel>(cCh.operator LONG()));
				CheckDlgButton(aIDs[cCh.operator LONG()], BST_CHECKED);
			}
		}

		HDC hdc = GetDC();
		m_fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);

		RECT rcA;
		RECT rc1;
		RECT rc2;
		CWindow wnd = GetDlgItem(IDC_CPF_ALPHAAREA);
		wnd.GetWindowRect(&rcA);
		ScreenToClient(&rcA);
		wnd.DestroyWindow();
		wnd = GetDlgItem(IDC_CPF_1DAREA);
		wnd.GetWindowRect(&rc1);
		ScreenToClient(&rc1);
		wnd.DestroyWindow();
		wnd = GetDlgItem(IDC_CPF_2DAREA);
		wnd.GetWindowRect(&rc2);
		ScreenToClient(&rc2);
		wnd.DestroyWindow();
		int nDelta = (rc1.right-rc1.left) - (rcA.bottom-rcA.top);
		rc2.right += nDelta;
		rc1.left += nDelta;
		rcA.right += nDelta;

		m_wndAreaAlpha.Create(m_hWnd, rcA, _T("Alpha"), WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_CPF_ALPHAAREA);
		m_wndAreaAlpha.SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOREDRAW);

		m_wndArea1D.Create(m_hWnd, rc1, _T("1D"), WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_CPF_1DAREA);
		m_wndArea1D.SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOREDRAW);

		m_wndArea2D.Create(m_hWnd, rc2, _T("2D"), WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_CPF_2DAREA);
		m_wndArea2D.SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOREDRAW);

		BOOL b;
		CContextHelpDlg<CDialogColorPicker>::OnInitDialog(0, 0, 0, b);

		m_wndEdit[EChR] = GetDlgItem(IDC_CPF_EDIT_R);
		m_wndEdit[EChG] = GetDlgItem(IDC_CPF_EDIT_G);
		m_wndEdit[EChB] = GetDlgItem(IDC_CPF_EDIT_B);
		m_wndEdit[EChH] = GetDlgItem(IDC_CPF_EDIT_H);
		m_wndEdit[EChL] = GetDlgItem(IDC_CPF_EDIT_L);
		m_wndEdit[EChS] = GetDlgItem(IDC_CPF_EDIT_S);
		m_wndEdit[EChA] = GetDlgItem(IDC_CPF_EDIT_A);

		if (!m_bAlphaChannel)
		{
			m_aColor[EChA] = 1.0f;
			m_bAlphaChannel = false;
			m_wndEdit[EChA].ShowWindow(SW_HIDE);
			m_wndAreaAlpha.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CPF_SEP2).ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CPF_LABEL_A).ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CPF_SPIN_A).ShowWindow(SW_HIDE);
			RECT rc;
			m_wndAreaAlpha.GetWindowRect(&rc);
			ScreenToClient(&rc);
			RECT rc2;
			m_wndArea1D.GetWindowRect(&rc2);
			ScreenToClient(&rc2);
			rc2.bottom = rc.bottom;
			m_wndArea1D.MoveWindow(&rc2);
			m_wndArea2D.GetWindowRect(&rc2);
			ScreenToClient(&rc2);
			rc2.bottom = rc.bottom;
			m_wndArea2D.MoveWindow(&rc2);
		}

		RECT rcHistory = {7, 131, 198, 141};
		MapDialogRect(&rcHistory);
		m_wndHistory.Init(m_pColorsCfg, m_hWnd, &rcHistory, m_tLocaleID, IDC_CPF_HISTORY);

		m_bEnableUpdates = false;
		Data2GUI();
		m_bEnableUpdates = true;

		DlgResize_Init();

		CConfigValue cSizeX;
		CConfigValue cSizeY;
		if (m_pColorsCfg)
		{
			m_pColorsCfg->ItemValueGet(CComBSTR(L"PDSizeX"), &cSizeX);
			m_pColorsCfg->ItemValueGet(CComBSTR(L"PDSizeY"), &cSizeY);
		}

		if (cSizeX.TypeGet() == ECVTFloat && cSizeX.operator float() > 0.0f && cSizeY.TypeGet() == ECVTFloat && cSizeY.operator float() > 0.0f)
		{
			RECT rc;
			GetWindowRect(&rc);
			rc.right = rc.left + cSizeX.operator float()*m_fScale;
			rc.bottom = rc.top + cSizeY.operator float()*m_fScale;
			MoveWindow(&rc);
		}

		CenterWindow(GetParent());

		return 0;
	}

	LRESULT OnOK(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		HWND hFocus = GetFocus();
		for (int i = 0; i < EChA; i++)
		{
			if (hFocus == m_wndEdit[i].m_hWnd)
			{
				int ii = ((i/3)*3) + ((i+1)%3);
				m_wndEdit[ii].SetSelAll();
				m_wndEdit[ii].SetFocus();
				break;
			}
		}

		if (m_pColorsCfg)
		{
			RECT rc;
			GetWindowRect(&rc);
			BSTR aIDs[2];
			CComBSTR bstrIDX(L"PDSizeX");
			CComBSTR bstrIDY(L"PDSizeY");
			aIDs[0] = bstrIDX;
			aIDs[1] = bstrIDY;
			TConfigValue aVals[2];
			aVals[0] = CConfigValue((rc.right-rc.left)/m_fScale);
			aVals[1] = CConfigValue((rc.bottom-rc.top)/m_fScale);
			m_pColorsCfg->ItemValuesSet(2, aIDs, aVals);
			if (m_tInitial.fR != m_aColor[EChR] || m_tInitial.fG != m_aColor[EChG] ||
				m_tInitial.fB != m_aColor[EChB] || m_tInitial.fA != m_aColor[EChA])
			{
				m_wndHistory.AddColor();
			}
		}

		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnCancel(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{

		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnHelpClick(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ShellExecute(NULL, _T("open"), _T("http://www.rw-designer.com/color-picker-window"), NULL, NULL, SW_SHOW);

		return 0;
	}

	template<EChannel t_nChannel>
	LRESULT OnChannelClicked(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		m_wndArea2D.SetActiveChannel(t_nChannel);
		m_wndArea2D.ResetCache();
		m_wndArea1D.SetActiveChannel(t_nChannel);
		m_wndArea1D.ResetCache();
		if (m_pColorsCfg)
		{
			CComBSTR bstrID(L"PDChannel");
			m_pColorsCfg->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(LONG(t_nChannel)));
		}
		return 0;
	}
	template<int t_nChannel>
	LRESULT OnEditChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		UpdateColorConfig();
		float const fMul1 = t_nChannel == EChH ? 10.0f : m_fFactor*powf(10, m_nDecimals);
		float const fMul2 = t_nChannel == EChH ? 0.1f : powf(10, -m_nDecimals);

		TCHAR szTmp[32] = _T("0");
		m_wndEdit[t_nChannel].GetWindowText(szTmp, itemsof(szTmp));
		szTmp[itemsof(szTmp)-1] = _T('\0');
		float f = 0.0f;
		if (1 == _stscanf(szTmp, _T("%f"), &f))
		{
			if (m_fFactor == 1.0f && f > 1.0f && _tcschr(szTmp, _T('.')) == NULL)
			{
				if (f <= 255 && f == int(f))
					f /= 255.0f; // assume number was entered using "old" format
			}
			if (t_nChannel != EChH)
				f /= m_fFactor;
			if (fabsf(f-m_aColor[t_nChannel]) >= (t_nChannel != EChH ? fMul2/m_fFactor : 1.0f))
			{
				ProcessChannelChange(t_nChannel, f);
			}
		}
		return 0;
	}
	template<int t_nChannel>
	LRESULT OnEditKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		UpdateColorConfig();
		float const fMul1 = t_nChannel == EChH ? 10.0f : m_fFactor*powf(10, m_nDecimals);
		float const fMul2 = t_nChannel == EChH ? 0.1f : powf(10, -m_nDecimals);

		TCHAR szTmp[32];
		_stprintf(szTmp, _T("%g"), float(int(m_aColor[t_nChannel]*fMul1+0.5f)*fMul2));
		m_bEnableUpdates = false;
		m_wndEdit[t_nChannel].SetWindowText(szTmp);
		m_bEnableUpdates = true;
		return 0;
	}
	template<int t_nChannel>
	LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		if (!m_bEnableUpdates)
			return 0;

		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		UpdateColorConfig();
		float const fMul1 = t_nChannel == EChH ? 10.0f : m_fFactor*powf(10, m_nDecimals);
		float const fMul2 = t_nChannel == EChH ? 0.1f : powf(10, -m_nDecimals);

		if (pNMUD->iDelta > 0)
		{
			if (m_aColor[t_nChannel] > 0.0f)
			{
				int n = m_aColor[t_nChannel]*(t_nChannel == EChH ? 1.0f : 255.0f)-0.5f;
				if (n < 0) n = 0;
				float const f = n/(t_nChannel == EChH ? 1.0f : 255.0f);
				m_bEnableUpdates = false;
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%g"), float(int(f*fMul1+0.5f)*fMul2));
				m_wndEdit[t_nChannel].SetWindowText(szTmp);
				ProcessChannelChange(t_nChannel, f);
				m_bEnableUpdates = true;
			}
		}
		else
		{
			if (m_aColor[t_nChannel] < (t_nChannel == EChH ? 360.0f : 1.0f))
			{
				int n = m_aColor[t_nChannel]*(t_nChannel == EChH ? 1.0f : 255.0f)+1.5f;
				if (n > (t_nChannel == EChH ? 360 : 255)) n = (t_nChannel == EChH ? 360 : 255);
				float f = n/(t_nChannel == EChH ? 1.0f : 255.0f);
				m_bEnableUpdates = false;
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%g"), float(int(f*fMul1+0.5f)*fMul2));
				m_wndEdit[t_nChannel].SetWindowText(szTmp);
				ProcessChannelChange(t_nChannel, f);
				m_bEnableUpdates = true;
			}
		}

		return 0;
	}
	LRESULT OnColorArea2DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		m_wndArea2D.GetColor().GetAll(m_aColor);
		m_bEnableUpdates = false;
		Data2GUI();
		m_bEnableUpdates = true;
		return 0;
	}
	LRESULT OnColorArea1DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		m_wndArea1D.GetColor().GetAll(m_aColor);
		m_bEnableUpdates = false;
		Data2GUI();
		m_bEnableUpdates = true;
		return 0;
	}
	LRESULT OnColorAreaAlphaChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		m_wndAreaAlpha.GetColor().GetAll(m_aColor);
		m_bEnableUpdates = false;
		Data2GUI();
		m_bEnableUpdates = true;
		return 0;
	}
	LRESULT OnColorHistoryColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		TSwatchColor t = m_wndHistory.GetColor();
		if (t.eSpace == ESCSRGB)
		{
			m_aColor[EChR] = t.f1;
			m_aColor[EChG] = t.f2;
			m_aColor[EChB] = t.f3;
			CColorBaseHLSA c;
			c.SetRGB(t.f1, t.f2, t.f3);
			m_aColor[EChH] = c.GetH();
			m_aColor[EChL] = c.GetL();
			m_aColor[EChS] = c.GetS();
			m_aColor[EChA] = t.fA;
		}
		else
		{
			m_aColor[EChH] = t.f1;
			m_aColor[EChL] = t.f2;
			m_aColor[EChS] = t.f3;
			CColorBaseHLSA c;
			c.SetHLS(t.f1, t.f2, t.f3);
			c.GetRGB(m_aColor+EChR, m_aColor+EChG, m_aColor+EChB);
			m_aColor[EChA] = t.fA;
		}

		m_bEnableUpdates = false;
		Data2GUI();
		m_bEnableUpdates = true;
		return 0;
	}

private:
	class CColorBaseRGBHLSA
	{
	public:
		CColorBaseRGBHLSA() : m_bValidRGB(true), m_bValidHLS(true)
		{
			m_aColor[EChR] = m_aColor[EChG] = m_aColor[EChB] = m_aColor[EChH] = m_aColor[EChL] = m_aColor[EChS] = 0;
			m_aColor[EChA] = 1.0f;
		}

		void SetAll(float const* a_pAll7) { CopyMemory(m_aColor, a_pAll7, sizeof m_aColor); m_bValidRGB = m_bValidHLS = true; }
		void SetH(float a_fVal) { SyncHLS(); m_aColor[EChH] = a_fVal; m_bValidRGB = false; }
		void SetL(float a_fVal) { SyncHLS(); m_aColor[EChL] = a_fVal; m_bValidRGB = false; }
		void SetS(float a_fVal) { SyncHLS(); m_aColor[EChS] = a_fVal; m_bValidRGB = false; }
		void SetR(float a_fVal) { SyncRGB(); m_aColor[EChR] = a_fVal; m_bValidHLS = false; }
		void SetG(float a_fVal) { SyncRGB(); m_aColor[EChG] = a_fVal; m_bValidHLS = false; }
		void SetB(float a_fVal) { SyncRGB(); m_aColor[EChB] = a_fVal; m_bValidHLS = false; }
		void SetCh(EChannel a_eCh, float a_fVal) { if (a_eCh < EChH) {SyncRGB(); m_bValidHLS = false;} else if (a_eCh < EChA) {SyncHLS(); m_bValidRGB = false;} m_aColor[a_eCh] = a_fVal; }
		void SetHLS(float a_fH, float a_fL, float a_fS) { m_aColor[EChH] = a_fH; m_aColor[EChL] = a_fL; m_aColor[EChS] = a_fS; m_bValidRGB = false; }
		void SetRGB(float a_fR, float a_fG, float a_fB) { m_aColor[EChR] = a_fR; m_aColor[EChG] = a_fG; m_aColor[EChB] = a_fB; m_bValidHLS = false; }
		void SetRGB(DWORD a_tRGB) { SetRGB(CGammaTables::FromSRGB(GetRValue(a_tRGB)), CGammaTables::FromSRGB(GetGValue(a_tRGB)), CGammaTables::FromSRGB(GetBValue(a_tRGB))); }
		void SetBGR(DWORD a_tBGR) { SetRGB(CGammaTables::FromSRGB(GetBValue(a_tBGR)), CGammaTables::FromSRGB(GetGValue(a_tBGR)), CGammaTables::FromSRGB(GetRValue(a_tBGR))); }
		void SetA(float a_fVal) { m_aColor[EChA] = a_fVal; }

		void GetAll(float* a_pAll7) const { SyncHLS(); SyncRGB(); CopyMemory(a_pAll7, m_aColor, sizeof m_aColor); }
		float GetH() const { SyncHLS(); return m_aColor[EChH]; }
		float GetL() const { SyncHLS(); return m_aColor[EChL]; }
		float GetS() const { SyncHLS(); return m_aColor[EChS]; }
		float GetR() const { SyncRGB(); return m_aColor[EChR]; }
		float GetG() const { SyncRGB(); return m_aColor[EChG]; }
		float GetB() const { SyncRGB(); return m_aColor[EChB]; }
		float GetCh(EChannel a_eCh) const { if (a_eCh < EChH) SyncRGB(); else if (a_eCh < EChA) SyncHLS(); return m_aColor[a_eCh]; }
		void GetHLS(float* a_pH, float* a_pL, float* a_pS) const { SyncHLS(); *a_pH = m_aColor[EChH]; *a_pL = m_aColor[EChL]; *a_pS = m_aColor[EChS]; }
		void GetRGB(float* a_pR, float* a_pG, float* a_pB) const { SyncRGB(); *a_pR = m_aColor[EChR]; *a_pG = m_aColor[EChG]; *a_pB = m_aColor[EChB]; }
		DWORD GetRGB() const { SyncRGB(); return RGB(CGammaTables::ToSRGB(m_aColor[EChR]), CGammaTables::ToSRGB(m_aColor[EChG]), CGammaTables::ToSRGB(m_aColor[EChB])); }
		DWORD GetBGR() const { SyncRGB(); return RGB(CGammaTables::ToSRGB(m_aColor[EChB]), CGammaTables::ToSRGB(m_aColor[EChG]), CGammaTables::ToSRGB(m_aColor[EChR])); }
		float GetA() const { return m_aColor[EChA]; }

	private:
		void SyncHLS() const
		{
			if (!m_bValidHLS)
			{
				RGB2HLS(m_aColor[EChR], m_aColor[EChG], m_aColor[EChB], m_aColor[EChH], m_aColor[EChL], m_aColor[EChS]);
				m_bValidHLS = true;
			}
		}
		void SyncRGB() const
		{
			if (!m_bValidRGB)
			{
				HLS2RGB(m_aColor[EChH], m_aColor[EChL], m_aColor[EChS], m_aColor[EChR], m_aColor[EChG], m_aColor[EChB]);
				m_bValidRGB = true;
			}
		}
		static float hls_value(float n1, float n2, float h)
		{
			h += 360.0f;
			float hue = h - 360.0f*(int)(h/360.0f);

			if (hue < 60.0f)
				return n1 + ( n2 - n1 ) * hue / 60.0f;
			else if (hue < 180.0f)
				return n2;
			else if (hue < 240.0f)
				return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
			else
				return n1;
		}
		static void HLS2RGB(float h, float l, float s, float& r, float& g, float& b)
		{ // h from <0, 360)
			float m1, m2;
			m2 = l + (l <= 0.5f ? l*s : s - l*s);
			m1 = 2.0f * l - m2;
			if (s == 0.0f)
				r = g = b = l;
			else
			{
				r = hls_value(m1, m2, h+120.0f);
				g = hls_value(m1, m2, h);
				b = hls_value(m1, m2, h-120.0f);
			}
			if (r > 1.0f) r = 1.0f;
			if (g > 1.0f) g = 1.0f;
			if (b > 1.0f) b = 1.0f;
			if (r < 0.0f) r = 0.0f;
			if (g < 0.0f) g = 0.0f;
			if (b < 0.0f) b = 0.0f;
		}
		static void RGB2HLS(float r, float g, float b, float& h, float& l, float& s)
		{
			float bc, gc, rc, rgbmax, rgbmin;

			// Compute luminosity.
			rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
			rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
			l = (rgbmax + rgbmin) * 0.5f;

			// Compute saturation.
			if (rgbmax == rgbmin)
				s = 0.0f;
			else if (l <= 0.5f)
				s = (rgbmax - rgbmin) / (rgbmax + rgbmin);
			else
				s = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

			// Compute the hue.
			if (rgbmax == rgbmin)
				h = 0.0f;
			else
			{
				rc = (rgbmax - r) / (rgbmax - rgbmin);
				gc = (rgbmax - g) / (rgbmax - rgbmin);
				bc = (rgbmax - b) / (rgbmax - rgbmin);

				if (r == rgbmax)
					h = bc - gc;
				else if (g == rgbmax)
					h = 2.0f + rc - bc;
				else
					h = 4.0f + gc - rc;

				h *= 60.0f;
				h += 360.0f;
				h = h - 360.0f*(int)(h/360.0f);
			}
		}

	private:
		float mutable m_aColor[EChCnt];
		bool mutable m_bValidRGB;
		bool mutable m_bValidHLS;
	};

	template<class TColorBase>
	struct CColorInterpreterHorzA
	{
		static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
			a_tColor.SetA(a_fX);
		}
		static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fX = a_tColor.GetA();
			if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			a_fY = 0.5f;
		}

		static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			DWORD const clr1 = GetSysColor(COLOR_3DLIGHT);
			DWORD const clr2 = GetSysColor(COLOR_3DSHADOW);
			float const aClr[8] =
			{
				CGammaTables::FromSRGB(GetBValue(clr1)), CGammaTables::FromSRGB(GetGValue(clr1)), CGammaTables::FromSRGB(GetRValue(clr1)), 0,
				CGammaTables::FromSRGB(GetBValue(clr2)), CGammaTables::FromSRGB(GetGValue(clr2)), CGammaTables::FromSRGB(GetRValue(clr2)), 0,
			};

			int a_nSquare = 4;

			float aBase[3];
			a_tCurrent.GetRGB(aBase+2, aBase+1, aBase);
			if (aBase[0] <= 0.0f) aBase[0] = 0.0f; else if (aBase[0] >= 1.0f) aBase[0] = 1.0f; else aBase[0] = aBase[0];
			if (aBase[1] <= 0.0f) aBase[1] = 0.0f; else if (aBase[1] >= 1.0f) aBase[1] = 1.0f; else aBase[1] = aBase[1];
			if (aBase[2] <= 0.0f) aBase[2] = 0.0f; else if (aBase[2] >= 1.0f) aBase[2] = 1.0f; else aBase[2] = aBase[2];

			for (; a_nSizeY > 0; --a_nSizeY)
			{
				float x = 0.0f;
				for (int nSizeX = a_nSizeX; nSizeX > 0; --nSizeX, x+=a_fStepX)
				{
					float nx = 1.0f-x;
					float const* p = aClr + ((((nSizeX/a_nSquare)^(a_nSizeY/a_nSquare))&1)<<2);
					*(a_pDst++) = RGB(CGammaTables::ToSRGB(aBase[0]*x+p[0]*nx), CGammaTables::ToSRGB(aBase[1]*x+p[1]*nx), CGammaTables::ToSRGB(aBase[2]*x+p[2]*nx));
				}
			}
		}

		static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}
		static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetA() != a2.GetA())
				return true;
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}
	};

	typedef CColorArea<CColorInterpreterHorzA, CColorBaseRGBHLSA, COLORAREAHORZAWNDCLASS> CColorAreaHorzA;

	template<class TColorBase>
	struct CColorInterpreter1DFlex
	{
		CColorInterpreter1DFlex() : eCh(EChR) {}

		void SetActiveChannel(EChannel a_eCh)
		{
			eCh = a_eCh;
		}

		void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			a_fY = 1.0f-a_fY; if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
			switch (eCh)
			{
			case EChR: a_tColor.SetR(a_fY); break;
			case EChG: a_tColor.SetG(a_fY); break;
			case EChB: a_tColor.SetB(a_fY); break;
			case EChH: a_tColor.SetH(a_fY*360.0f); break;
			case EChL: a_tColor.SetL(a_fY); break;
			case EChS: a_tColor.SetS(a_fY); break;
			}
		}
		void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fX = 0.5f;
			switch (eCh)
			{
			case EChR: a_fY = 1.0f-a_tColor.GetR(); break;
			case EChG: a_fY = 1.0f-a_tColor.GetG(); break;
			case EChB: a_fY = 1.0f-a_tColor.GetB(); break;
			case EChH: a_fY = 1.0f-a_tColor.GetH()/360.0f; break;
			case EChL: a_fY = 1.0f-a_tColor.GetL(); break;
			case EChS: a_fY = 1.0f-a_tColor.GetS(); break;
			}
			if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
		}

		void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			int a_nSquare = 4;

			TColorBase tCur = a_tCurrent;
			if (eCh == EChH)
			{
				tCur.SetL(0.5f);
				tCur.SetS(1.0f);
			}
			else if (eCh == EChS)
			{
				if (tCur.GetL() < 0.2f) tCur.SetL(0.2f);
				else if (tCur.GetL() > 0.8f) tCur.SetL(0.8f);
			}

			for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
			{
				float aBase[3];
				tCur.SetCh(eCh, 1.0f-(eCh == EChH ? y*360.0f : y));
				tCur.GetRGB(aBase+2, aBase+1, aBase);
				DWORD dw = RGB(CGammaTables::ToSRGB(aBase[0]), CGammaTables::ToSRGB(aBase[1]), CGammaTables::ToSRGB(aBase[2]));
				for (int nSizeX = a_nSizeX; nSizeX > 0; --nSizeX)
				{
					*(a_pDst++) = dw;
				}
			}
		}

		void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			if (eCh == EChH)
				return false;
			EChannel eCh1 = static_cast<EChannel>(((eCh/3)*3) + ((eCh+1)%3));
			EChannel eCh2 = static_cast<EChannel>(((eCh/3)*3) + ((eCh+2)%3));
			return a1.GetCh(eCh1) != a2.GetCh(eCh1) || a1.GetCh(eCh2) != a2.GetCh(eCh2);
		}
		bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetCh(eCh) != a2.GetCh(eCh))
				return true;
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}

	private:
		EChannel eCh;
	};

	typedef CColorArea<CColorInterpreter1DFlex, CColorBaseRGBHLSA, COLORAREA1DFLEXWNDCLASS> CColorArea1DFlex;

	template<class TColorBase>
	struct CColorInterpreter2DFlex
	{
		CColorInterpreter2DFlex() : eCh(EChR) {UpdateDependentChannels();}

		void SetActiveChannel(EChannel a_eCh)
		{
			eCh = a_eCh;
			UpdateDependentChannels();
		}

		void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
			switch (eChX)
			{
			case EChR: a_tColor.SetR(a_fX); break;
			case EChG: a_tColor.SetG(a_fX); break;
			case EChB: a_tColor.SetB(a_fX); break;
			case EChH: a_tColor.SetH(a_fX*360.0f); break;
			case EChL: a_tColor.SetL(a_fX); break;
			case EChS: a_tColor.SetS(a_fX); break;
			}
			a_fY = 1.0f-a_fY; if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
			switch (eChY)
			{
			case EChR: a_tColor.SetR(a_fY); break;
			case EChG: a_tColor.SetG(a_fY); break;
			case EChB: a_tColor.SetB(a_fY); break;
			case EChH: a_tColor.SetH(a_fY*360.0f); break;
			case EChL: a_tColor.SetL(a_fY); break;
			case EChS: a_tColor.SetS(a_fY); break;
			}
		}
		void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			switch (eChX)
			{
			case EChR: a_fX = a_tColor.GetR(); break;
			case EChG: a_fX = a_tColor.GetG(); break;
			case EChB: a_fX = a_tColor.GetB(); break;
			case EChH: a_fX = a_tColor.GetH()/360.0f; break;
			case EChL: a_fX = a_tColor.GetL(); break;
			case EChS: a_fX = a_tColor.GetS(); break;
			}
			switch (eChY)
			{
			case EChR: a_fY = 1.0f-a_tColor.GetR(); break;
			case EChG: a_fY = 1.0f-a_tColor.GetG(); break;
			case EChB: a_fY = 1.0f-a_tColor.GetB(); break;
			case EChH: a_fY = 1.0f-a_tColor.GetH()/360.0f; break;
			case EChL: a_fY = 1.0f-a_tColor.GetL(); break;
			case EChS: a_fY = 1.0f-a_tColor.GetS(); break;
			}
			if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
		}

		void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			int a_nSquare = 4;

			TColorBase tCur = a_tCurrent;
			if (eCh == EChL)
			{
				if (tCur.GetL() < 0.2f) tCur.SetL(0.2f);
				else if (tCur.GetL() > 0.8f) tCur.SetL(0.8f);
			}

			for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
			{
				tCur.SetCh(eChY, 1.0f-(eChY == EChH ? y*360.0f : y));
				int nSizeX = a_nSizeX;
				for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
				{
					tCur.SetCh(eChX, eChX == EChH ? x*360.0f : x);
					float aBase[3];
					tCur.GetRGB(aBase+2, aBase+1, aBase);
					*(a_pDst++) = RGB(CGammaTables::ToSRGB(aBase[0]), CGammaTables::ToSRGB(aBase[1]), CGammaTables::ToSRGB(aBase[2]));
				}
			}
		}

		void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			return a1.GetCh(eCh) != a2.GetCh(eCh);
		}
		bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetCh(eChX) != a2.GetCh(eChX) || a1.GetCh(eChY) != a2.GetCh(eChY))
				return true;
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}

	private:
		void UpdateDependentChannels()
		{
			switch (eCh)
			{
			case EChR: eChX = EChG; eChY = EChB; break;
			case EChG: eChX = EChR; eChY = EChB; break;
			case EChB: eChX = EChG; eChY = EChR; break;
			case EChH: eChX = EChS; eChY = EChL; break;
			case EChL: eChX = EChH; eChY = EChS; break;
			case EChS: eChX = EChH; eChY = EChL; break;
			default: eCh = eChX = eChY = EChR;
			}
		}

		EChannel eCh;
		EChannel eChX;
		EChannel eChY;
	};

	typedef CColorArea<CColorInterpreter2DFlex, CColorBaseRGBHLSA, COLORAREA2DFLEXWNDCLASS> CColorArea2DFlex;

private:
	void UpdateColorConfig()
	{
		if (m_pColorsCfg)
		{
			CConfigValue cRng;
			m_pColorsCfg->ItemValueGet(CComBSTR(L"Factor"), &cRng);
			m_fFactor = cRng;
			CConfigValue cDec;
			m_pColorsCfg->ItemValueGet(CComBSTR(L"Decimal"), &cDec);
			m_nDecimals = cDec;
		}
	}

	void Data2GUI()
	{
		UpdateColorConfig();
		float const fMul1 = m_fFactor*powf(10, m_nDecimals);
		float const fMul2 = powf(10, -m_nDecimals);
		float const fHMul1 = 10.0f;
		float const fHMul2 = 0.1f;

		for (int i = 0; i < EChCnt; ++i)
		{
			TCHAR szTmp[32];
			_stprintf(szTmp, _T("%g"), i == EChH ? float(int(m_aColor[i]*fHMul1+0.5f)*fHMul2) : float(int(m_aColor[i]*fMul1+0.5f)*fMul2));
			m_wndEdit[i].SetWindowText(szTmp);
		}
		CColorBaseRGBHLSA cCol;
		cCol.SetAll(m_aColor);
		m_wndArea2D.SetColor(cCol);
		m_wndArea1D.SetColor(cCol);
		m_wndAreaAlpha.SetColor(cCol);
		m_wndHistory.SetColor(m_aColor[EChR], m_aColor[EChG], m_aColor[EChB], m_aColor[EChA]);
	}

	void ProcessChannelChange(int a_eChannel, float a_fValue)
	{
		float const fMul1 = m_fFactor*powf(10, m_nDecimals);
		float const fMul2 = powf(10, -m_nDecimals);
		float const fHMul1 = 10.0f;
		float const fHMul2 = 0.1f;

		CColorBaseRGBHLSA tClr;
		tClr.SetAll(m_aColor);
		tClr.SetCh(static_cast<EChannel>(a_eChannel), a_fValue);
		tClr.GetAll(m_aColor);
		m_bEnableUpdates = false;
		if (a_eChannel < EChH)
		{
			for (int i = EChH; i < EChA; ++i)
			{
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%g"), i == EChH ? float(int(m_aColor[i]*fHMul1+0.5f)*fHMul2) : float(int(m_aColor[i]*fMul1+0.5f)*fMul2));
				m_wndEdit[i].SetWindowText(szTmp);
			}
		}
		else if (a_eChannel < EChA)
		{
			for (int i = EChR; i < EChH; ++i)
			{
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%g"), i == EChH ? float(int(m_aColor[i]*fHMul1+0.5f)*fHMul2) : float(int(m_aColor[i]*fMul1+0.5f)*fMul2));
				m_wndEdit[i].SetWindowText(szTmp);
			}
		}
		m_wndArea1D.SetColor(tClr);
		m_wndArea2D.SetColor(tClr);
		m_wndAreaAlpha.SetColor(tClr);
		m_bEnableUpdates = true;
	}

private:
	CComPtr<IConfig> m_pColorsCfg;
	bool m_bAlphaChannel;
	float m_fFactor;
	LONG m_nDecimals;
	CEdit m_wndEdit[EChCnt];
	float m_aColor[EChCnt];
	CColorArea2DFlex m_wndArea2D;
	CColorArea1DFlex m_wndArea1D;
	CColorAreaHorzA m_wndAreaAlpha;
	bool m_bEnableUpdates;
	float m_fScale;
	CColorHistory2 m_wndHistory;
	TColor m_tInitial;
};

// CColorWindow

STDMETHODIMP CColorWindow::DoModal(RWHWND a_hParent, LCID a_tLocaleID, TColor* a_pColor, BYTE a_bAlpha)
{
	try
	{
		CDialogColorPicker cDlg(a_pColor->fR, a_pColor->fG, a_pColor->fB, a_pColor->fA, a_bAlpha, a_tLocaleID);
		if (IDOK != cDlg.DoModal(a_hParent))
			return S_FALSE;
		a_pColor->fR = cDlg.M_R();
		a_pColor->fG = cDlg.M_G();
		a_pColor->fB = cDlg.M_B();
		a_pColor->fA = cDlg.M_A();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CColorWindow::Name(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Colors[0405]Barvy");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CColorWindow::Description(ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		*a_ppDesc = new CMultiLanguageString(L"[0409]Define what should be displayed on a color button popup palette and how should the color picker dialog look like.[0405]Vyberte, co má být zobrazeno v navídce tlačítka pro rychlý výběr barev a co v dialogovém okně výběru barev.");
		return S_OK;
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>

class ATL_NO_VTABLE CConfigGUIColorDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIColorDlg>,
	public CDialogResize<CConfigGUIColorDlg>
{
public:
	CConfigGUIColorDlg()
	{
	}

	enum
	{
		IDC_CGC_RANGE = 100,
		IDC_CGC_DECIMAL_LABEL, IDC_CGC_DECIMAL, IDC_CGC_DECIMAL_SPIN,
		IDC_CGC_GAMMA,
		IDC_CGC_BUTTON_GRP, IDC_CGC_BP_DERIVEDCOLORS, IDC_CGC_BP_LASTCOLORS,
		IDC_CGC_WINDOW_GRP, IDC_CGC_TESTDLG,
	};

	BEGIN_DIALOG_EX(0, 0, 219, 79-16, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Color picker value range:[0405]Rozsah hodnot barev:"), IDC_STATIC, 0, 2, 91, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGC_RANGE, 94, 0, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Decimal places:[0405]Desetinná místa:"), IDC_CGC_DECIMAL_LABEL, 135, 2, 57, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGC_DECIMAL, 193, 0, 25, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL | ES_NUMBER, 0)
		CONTROL_CONTROL(_T(""), IDC_CGC_DECIMAL_SPIN, UPDOWN_CLASS, UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 207, 0, 11, 12, 0)
		//CONTROL_LTEXT(_T("[0409]Default gamma correction:[0405]Výchozí gama korekce:"), IDC_STATIC, 0, 18, 93, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_CGC_GAMMA, 94, 16, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		//CONTROL_GROUPBOX(_T("[0409]Color button palette[0405]Paleta tlačítka barev"), IDC_CGC_BUTTON_GRP, 0, 32, 106, 47, WS_VISIBLE, WS_EX_TRANSPARENT)
		//CONTROL_CHECKBOX(_T("[0409]Similar colors[0405]Podobné barvy"), IDC_CGC_BP_DERIVEDCOLORS, 7, 46, 93, 10, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_CHECKBOX(_T("[0409]Recently used colors[0405]Naposled použité barvy"), IDC_CGC_BP_LASTCOLORS, 7, 60, 93, 10, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_GROUPBOX(_T("[0409]Color picker window[0405]Okno výběru barev"), IDC_CGC_WINDOW_GRP, 113, 32, 106, 47, WS_VISIBLE, WS_EX_TRANSPARENT)
		//CONTROL_PUSHBUTTON(_T("[0409]Test[0405]Testovat"), IDC_CGC_TESTDLG, 120, 43, 84, 14, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_GROUPBOX(_T("[0409]Color button palette[0405]Paleta tlačítka barev"), IDC_CGC_BUTTON_GRP, 0, 16, 106, 47, WS_VISIBLE, WS_EX_TRANSPARENT)
		CONTROL_CHECKBOX(_T("[0409]Similar colors[0405]Podobné barvy"), IDC_CGC_BP_DERIVEDCOLORS, 7, 30, 93, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Recently used colors[0405]Naposled použité barvy"), IDC_CGC_BP_LASTCOLORS, 7, 44, 93, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_GROUPBOX(_T("[0409]Color picker window[0405]Okno výběru barev"), IDC_CGC_WINDOW_GRP, 113, 16, 106, 47, WS_VISIBLE, WS_EX_TRANSPARENT)
		CONTROL_PUSHBUTTON(_T("[0409]Test[0405]Testovat"), IDC_CGC_TESTDLG, 120, 27, 84, 14, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIColorDlg)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIColorDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CGC_TESTDLG, BN_CLICKED, OnClickedTest)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIColorDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIColorDlg)
		//DLGRESIZE_CONTROL(IDC_CGC_RANGE, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_CGC_DECIMAL_LABEL, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CGC_DECIMAL, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CGC_DECIMAL_SPIN, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CGC_PICKERVIEWID, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_CGC_PICKERVIEWCFG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGC_BUTTON_GRP, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGC_WINDOW_GRP, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGC_TESTDLG, DLSZ_DIVMOVE_X(2))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIColorDlg)
		CONFIGITEM_EDITBOX(IDC_CGC_RANGE, L"Factor")
		CONFIGITEM_EDITBOX(IDC_CGC_DECIMAL, L"Decimal")
		//CONFIGITEM_EDITBOX(IDC_CGC_GAMMA, L"Gamma")
		CONFIGITEM_CHECKBOX_FLAG(IDC_CGC_BP_LASTCOLORS, L"ButtonPal", 1)
		CONFIGITEM_CHECKBOX_FLAG(IDC_CGC_BP_DERIVEDCOLORS, L"ButtonPal", 2)
		//CONFIGITEM_COMBOBOX(IDC_CGC_PICKERVIEWID, L"PickerDialog")
		//CONFIGITEM_SUBCONFIG_NOMARGINS(IDC_CGC_PICKERVIEWCFG, L"PickerDialog")
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CUpDownCtrl wndUD = GetDlgItem(IDC_CGC_DECIMAL_SPIN);
		wndUD.SetRange(0, 5);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnClickedTest(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		static TColor tColor = {0.0f, 0.0f, 0.0f, 1.0f};
		ColorWindowModal(&tColor, m_tLocaleID, m_hWnd, true);
		return 0;
	}

};

extern GUID const tColorDlgID = {0x9c472a83, 0x87fe, 0x4357, {0x89, 0xc9, 0x27, 0xd7, 0x42, 0xb2, 0x4b, 0x13}};

STDMETHODIMP CColorWindow::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		CComPtr<ILocalizedString> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
		pCfg->ItemInsSimple(CComBSTR(L"Colors"), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"LastColors"), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"Expanded"), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(L"ButtonPal"), CMultiLanguageString::GetAuto(L"[0409]Color button palette[0405]Paleta tlačítka barvy"), CMultiLanguageString::GetAuto(L"[0409]Colors displayed in the color button's popup palette (beside the stadard ones).[0405]Barvy zobrazené v paletě tlačítka pro výběr barvy (mimo standardních)."), CConfigValue(1L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"Factor"), CMultiLanguageString::GetAuto(L"[0409]Color range[0405]Barevný rozsah"), CMultiLanguageString::GetAuto(L"[0409]Maximum color value. Recommended values are 1 (the native HDR range), 100 (percent range), or 255 (classic 8-bit color).[0405]Maximámní hodnota pro barvy. Doporučené hodnoty jsou 1 (přirozený HDR rozsah), 100 (rozsah v procentech) nebo 255 (klasický 8-bitový rozsah)."), CConfigValue(100.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"Decimal"), CMultiLanguageString::GetAuto(L"[0409]Decimal places[0405]Desetinná místa"), CMultiLanguageString::GetAuto(L"[0409]Number of places behind the decimal point. Increase this value for higher precision color accuracy or when range is low.[0405]Počet míst za desetinnou čárkou. Zvyšte tuto hodnotu pro vyšší přesnost při zadávání barev nebo když rozsah je nízký."), CConfigValue(1L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"Gamma"), CMultiLanguageString::GetAuto(L"[0409]Default gamma[0405]Výchozí gama"), CMultiLanguageString::GetAuto(L"[0409]Gamma correction value assumed when no color profile is assigned to an image.[0405]Gama korekce předpokládaná v případech, kdy obrázek nemá barevný profil."), CConfigValue(2.2f), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(L"PDSizeX"), pDummy, pDummy, CConfigValue(-1.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"PDSizeY"), pDummy, pDummy, CConfigValue(-1.0f), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"PDChannel"), pDummy, pDummy, CConfigValue(0L), NULL, 0, NULL);
		CConfigCustomGUI<&tColorDlgID, CConfigGUIColorDlg>::FinalizeConfig(pCfg);
		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}
