// ColorPicker.h : Declaration of the CColorPicker

#pragma once
#include "resource.h"       // main symbols

#include "RWImaging.h"
#include <SubjectImpl.h>
#include <Win32LangEx.h>
#include <WTL_ColorHistory.h>
#include <WTL_ColorWheel.h>
#include <WTL_ColorArea.h>
#include <WTL_ColorPopup.h>
#include <GammaCorrection.h>
#include <ContextHelpDlg.h>


extern __declspec(selectany) TCHAR const  COLORAREAFLEXAWNDCLASS[] = _T("WTL_ColorAreaFlexAlpha");


// CColorPicker

class ATL_NO_VTABLE CColorPicker :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CColorPicker, &CLSID_ColorPicker>,
	public Win32LangEx::CLangIndirectDialogImpl<CColorPicker>,
	public CContextHelpDlg<CColorPicker>,
	public CChildWindowImpl<CColorPicker, CSubjectImpl<IColorPicker, IColorPickerObserver, ULONG> >
{
public:
	CColorPicker() : m_wndARGB(this, 1), m_nCollapsedSize(20), m_nExpandedSize(20), m_nIconSize(12),
		m_bEnableUpdates(true), m_bExpanded(false), m_fFactor(1.0f), m_nDecimals(3)
	{
		m_tColor.fR = m_tColor.fG = m_tColor.fB = 0.0f;
		m_tColor.fA = 1.0f;
	}
	~CColorPicker()
	{
		m_cImageList.Destroy();
	}

DECLARE_NO_REGISTRY()

	enum
	{
		IDC_LABEL = 100, IDC_ACTIVE, IDC_HISTORY, IDC_COMMANDS, IDC_WHEEL, IDC_ALPHA,
		IDC_AARRGGBB, IDC_EDIT_BASE, IDC_SPIN_BASE = IDC_EDIT_BASE+4,
		ID_EXPAND = 200, ID_COLLAPSE, ID_DROPPER, ID_ACTIVE,
	};

BEGIN_DIALOG_EX(0, 0, 100, 12, 0)
	DIALOG_FONT_AUTO()
	DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
	DIALOG_EXSTYLE(0)
END_DIALOG()

BEGIN_CONTROLS_MAP()
	CONTROL_LTEXT(_T(""), IDC_LABEL, 0, 2, 0, 8, WS_VISIBLE, 0)
	CONTROL_CONTROL(_T(""), IDC_ACTIVE, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 0, 0, 20, 12, 0)
	//CONTROL_EDITTEXT(IDC_AARRGGBB, 0, 0, 40, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT | ES_UPPERCASE, 0)
	//CONTROL_COMBOBOX(IDC_AARRGGBB, 0, 0, 40, 12, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL | CBS_UPPERCASE, 0)
	CONTROL_CONTROL(_T(""), IDC_COMMANDS, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 64, 0, 36, 12, 0)
END_CONTROLS_MAP()

BEGIN_COM_MAP(CColorPicker)
	COM_INTERFACE_ENTRY(IColorPicker)
	COM_INTERFACE_ENTRY(IChildWindow)
END_COM_MAP()

BEGIN_MSG_MAP(CColorPicker)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColorEdit)
	COMMAND_HANDLER(IDC_AARRGGBB, EN_CHANGE, OnARGBChanged)
	COMMAND_HANDLER(IDC_AARRGGBB, EN_KILLFOCUS, OnARGBKillFocus)
	COMMAND_HANDLER(ID_DROPPER, BN_CLICKED, OnDropper)
	COMMAND_HANDLER(ID_EXPAND, BN_CLICKED, OnExpand)
	COMMAND_HANDLER(ID_COLLAPSE, BN_CLICKED, OnCollapse)
	NOTIFY_HANDLER(IDC_HISTORY, CColorHistory::CHN_COLORCHANGED, OnHistoryChanged)
	NOTIFY_HANDLER(IDC_ALPHA, CColorAreaAlphaFlex::CAN_COLOR_CHANGED, OnColorAlphaChanged)
	NOTIFY_HANDLER(IDC_WHEEL, CColorWheel::CWN_CHANGED, OnColorWheelChanged)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	NOTIFY_HANDLER(IDC_ACTIVE, TBN_DROPDOWN, OnActiveDropDown)
	COMMAND_RANGE_CODE_HANDLER(IDC_EDIT_BASE, IDC_EDIT_BASE+3, EN_CHANGE, OnEditChanged)
	COMMAND_RANGE_CODE_HANDLER(IDC_EDIT_BASE, IDC_EDIT_BASE+3, EN_KILLFOCUS, OnEditKillFocus)
	COMMAND_RANGE_CODE_HANDLER(IDC_EDIT_BASE, IDC_EDIT_BASE+3, EN_SETFOCUS, OnEditSetFocus)
	NOTIFY_RANGE_CODE_HANDLER(IDC_SPIN_BASE, IDC_SPIN_BASE+3, UDN_DELTAPOS, OnUpDownChange)
	CHAIN_MSG_MAP(CContextHelpDlg<CColorPicker>)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
	NOTIFY_HANDLER(IDC_WHEEL, NM_KILLFOCUS, OnCtrlKillFocus)
	NOTIFY_HANDLER(IDC_ALPHA, NM_KILLFOCUS, OnCtrlKillFocus)
ALT_MSG_MAP(1)
	MESSAGE_HANDLER(WM_CHAR, OnARGBChar)
END_MSG_MAP()

BEGIN_CTXHELP_MAP(CColorPicker)
	CTXHELP_CONTROL_STRING(IDC_SPIN_BASE,   L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_SPIN_BASE+1, L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_SPIN_BASE+2, L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_SPIN_BASE+3, L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_EDIT_BASE,   L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_EDIT_BASE+1, L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_EDIT_BASE+2, L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_EDIT_BASE+3, L"[0409]Enter a number to set single color channel of the current color. Press Enter to go to next channel.[0405]Zadejte číslo pro změnu hodnoty barevného kanálu aktuální barvy. Stiskněte Enter pro přechod na další kanál.")
	CTXHELP_CONTROL_STRING(IDC_AARRGGBB, L"[0409]Enter color in hexadecimal web format as RRGGBB or include alpha channel using AARRGGBB format.[0405]Zadejte barvu v šestnáctkovém webovém formátu jako RRGGBB nebo přidejte i hodnotu alfa ve formátu AARRGGBB.")
	CTXHELP_CONTROL_STRING(IDC_WHEEL, L"[0409]Click on the wheel to change hue, click inside the triangle to change brightness and saturation.[0405]Klikněte na kruh pro změnu barevného tónu, klikněte na trojúhelník pro změnu jasu a saturace.")
	CTXHELP_CONTROL_STRING(IDC_ALPHA, L"[0409]Click with mouse or use the arrow keys to change color opacity (alpha).[0405]Klikněte myší nebo použijte klávesy šipek ke změně pokrytí (alfy).")
END_CTXHELP_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	virtual void OnFinalMessage(HWND a_hWnd)
	{
		Release();
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IColorPicker methods
public:
	STDMETHOD(ColorGet)(TColor* a_pColor);
	STDMETHOD(ColorSet)(TColor const* a_pColor);

	STDMETHOD(Create)(RWHWND a_hParent, RECT const* a_pPosition, BOOL a_bVisible, LCID a_tLocaleID, ILocalizedString* a_pName, BYTE a_bImportant, BSTR a_bstrContext, BSTR a_bstrLayout);
	STDMETHOD(Layout)(BSTR* a_bstrpLayout);
	STDMETHOD(OptimumSize)(SIZE* a_pSize);

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnCtlColorEdit(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnARGBChar(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnARGBChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnARGBKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnDropper(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnExpand(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCollapse(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnHistoryChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnColorAlphaChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnColorWheelChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnActiveDropDown(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);
	LRESULT OnEditChanged(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnEditKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnEditSetFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnUpDownChange(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCtrlKillFocus(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled);

private:
	template<typename TColorType>
	struct CColorInterpreterAlphaFlex
	{
		CColorInterpreterAlphaFlex() : m_bVertical(false), m_nSquareSize(4) {}

		void SetVertical() { m_bVertical = true; }
		void SetHorizontal() { m_bVertical = false; }
		void InitGamma(CGammaTables const* a_pGT) { m_pGT = a_pGT; }
		void SetSquareSize(int a_nSquareSize) { m_nSquareSize = a_nSquareSize; }

		void XY2Color(float a_fX, float a_fY, TColorType& a_tColor)
		{
			a_tColor.fA = m_bVertical ? 1.0f-a_fY : a_fX;
			if (a_tColor.fA < 0.0f) a_tColor.fA = 0.0f; else if (a_tColor.fA > 1.0f) a_tColor.fA = 1.0f;
		}
		void Color2XY(TColorType const& a_tColor, float& a_fX, float& a_fY)
		{
			if (m_bVertical)
			{
				a_fX = 0.5f;
				a_fY = 1.0f-a_tColor.fA;
				if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
			}
			else
			{
				a_fY = 0.5f;
				a_fX = a_tColor.fA;
				if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			}
		}

		void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorType const& a_tCurrent)
		{
			if (m_pGT == NULL) return;

			DWORD const clr1 = GetSysColor(COLOR_3DLIGHT);
			DWORD const clr2 = GetSysColor(COLOR_3DSHADOW);
			ULONG const aClr[8] =
			{
				m_pGT->m_aGamma[GetBValue(clr1)], m_pGT->m_aGamma[GetGValue(clr1)], m_pGT->m_aGamma[GetRValue(clr1)], 0,
				m_pGT->m_aGamma[GetBValue(clr2)], m_pGT->m_aGamma[GetGValue(clr2)], m_pGT->m_aGamma[GetRValue(clr2)], 0,
			};

			ULONG const aBase[3] =
			{
				a_tCurrent.fB <= 0.0f ? 0 : (a_tCurrent.fB >= 1.0f ? 65535 : ULONG(0.5f+65535*a_tCurrent.fB)),
				a_tCurrent.fG <= 0.0f ? 0 : (a_tCurrent.fG >= 1.0f ? 65535 : ULONG(0.5f+65535*a_tCurrent.fG)),
				a_tCurrent.fR <= 0.0f ? 0 : (a_tCurrent.fR >= 1.0f ? 65535 : ULONG(0.5f+65535*a_tCurrent.fR)),
			};

			if (m_bVertical)
			{
				ULONG xx = 0;
				ULONG dx = a_fStepY*0x1000000;
				for (; a_nSizeY > 0; --a_nSizeY, xx+=dx)
				{
					ULONG const nx = xx>>8;
					ULONG const x = 0x10000-nx;
					for (int nSizeX = a_nSizeX; nSizeX > 0; --nSizeX)
					{
						ULONG const* p = aClr + ((((nSizeX/m_nSquareSize)^(a_nSizeY/m_nSquareSize))&1)<<2);
						*(a_pDst++) = RGB(m_pGT->InvGamma((aBase[0]*x+p[0]*nx)>>16), m_pGT->InvGamma((aBase[1]*x+p[1]*nx)>>16), m_pGT->InvGamma((aBase[2]*x+p[2]*nx)>>16));
					}
				}
			}
			else
			{
				ULONG dx = a_fStepX*0x1000000;
				for (; a_nSizeY > 0; --a_nSizeY)
				{
					ULONG xx = 0;
					for (int nSizeX = a_nSizeX; nSizeX > 0; --nSizeX, xx+=dx)
					{
						ULONG const x = xx>>8;
						ULONG const nx = 0x10000-x;
						ULONG const* p = aClr + ((((nSizeX/m_nSquareSize)^(a_nSizeY/m_nSquareSize))&1)<<2);
						*(a_pDst++) = RGB(m_pGT->InvGamma((aBase[0]*x+p[0]*nx)>>16), m_pGT->InvGamma((aBase[1]*x+p[1]*nx)>>16), m_pGT->InvGamma((aBase[2]*x+p[2]*nx)>>16));
					}
				}
			}
		}

		void XY2ColorInternal(float a_fX, float a_fY, TColorType& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		bool InvalidateArea(TColor const& a1, TColorType const& a2)
		{
			return a1.fR != a2.fR || a1.fG != a2.fG || a1.fB != a2.fB;
		}
		bool InvalidateThumb(TColor const& a1, TColorType const& a2)
		{
			return a1.fA != a2.fA;
		}

	private:
		bool m_bVertical;
		CGammaTables const* m_pGT;
		int m_nSquareSize;
	};

	typedef CColorArea<CColorInterpreterAlphaFlex, CColor, COLORAREAFLEXAWNDCLASS> CColorAreaAlphaFlex;

private:
	void SplitArea(RECT const& a_rc, RECT* a_pWheel, RECT* a_pAlpha, bool* a_pVertical);
	void FormatFloatEdit(float a_f, LPTSTR a_psz);
	void FormatHexEdit(LPTSTR a_psz);
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
	void ControlLostFocus();
	enum EUpdateSource
	{
		EUSUnknown = 0,
		EUSCOM,
		EUSPopup,
		EUSHexEdit,
		EUSFloatEdit,
		EUSHistory,
		EUSDropper,
		EUSWheel,
		EUSAlpha,
	};
	void Data2GUI(EUpdateSource a_eSource);
	HBITMAP CreateColorBitmap();

private:
	CComPtr<IConfig> m_pColorsCfg;
	CComBSTR m_bstrContext;
	bool m_bExpanded;
	CComPtr<ILocalizedString> m_pName;

	TColor m_tColor;
	CComBSTR m_bstrName;
	CComBSTR m_bstrContextID;

	int m_nCollapsedSize;
	int m_nExpandedSize;
	int m_nIconSize;
	RECT m_rcGaps;
	bool m_bEnableUpdates;
	CContainedWindowT<CEdit> m_wndARGB;
	CFont m_cARGBFont;
	CToolBarCtrl m_wndActive;
	CColorPopup m_wndPopup;
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	SIZE m_tToolbarOffsets;
	CColorHistory m_wndHistory;
	SIZE m_tHistoryOffsets;

	CColorWheel m_wndWheel;
	CColorAreaAlphaFlex m_wndAlpha;
	CAutoPtr<CGammaTables> m_pGT;

	std::pair<CEdit, CUpDownCtrl> m_aEdits[4];
	RECT m_rcEditSize;
	COLORREF m_clrBrushR;
	CBrush m_hBrushR;
	COLORREF m_clrBrushG;
	CBrush m_hBrushG;
	COLORREF m_clrBrushB;
	CBrush m_hBrushB;
	CBrush m_hBrushA;
	std::pair<int, int> m_tHexCharPos;
	CBrush m_hBrushHex;
	float m_fFactor;
	LONG m_nDecimals;
};

OBJECT_ENTRY_AUTO(__uuidof(ColorPicker), CColorPicker)
