// OpenFilterFrameDlg.h : interface of the COpenFilterFrameDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "RWStorage.h"
#include <PlugInCache.h>
#include <Win32LangEx.h>
#include "OpenFilterDlg.h"
#include "ConfigIDsStorage.h"
#include <XPGUI.h>
#include <DPIUtils.h>
#include <IconRenderer.h>


class COpenFilterFrameDlg :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<COpenFilterFrameDlg>,
	public CDialogResize<COpenFilterFrameDlg>,
	public IStorageFilterWindowCallback
{
public:
	COpenFilterFrameDlg() : m_hIcon(NULL)
	{
	}
	~COpenFilterFrameDlg()
	{
		if (m_hIcon) DestroyIcon(m_hIcon);
	}

	bool DoModalPreTranslate(MSG const* a_pMsg)
	{
		if (!m_pSFWnd)
			return false;
		return m_pSFWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK || m_pSFWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK;
	}
	IStorageFilter* DoModal(HWND a_hParent, BSTR a_bstrInitial, DWORD a_dwFlags, IEnumUnknowns* a_pFormatFilters, IConfig* a_pUserConfig, IConfig* a_pContextConfig, ILocalizedString* a_pCaption, BSTR a_bstrHelp, IStorageFilterWindowListener* a_pListener, LCID a_tLocaleID)
	{
		m_tLocaleID = a_tLocaleID;
		m_pContextConfig = a_pContextConfig;

		SParams tParams;
		tParams.bstrInitial = a_bstrInitial;
		tParams.dwFlags = a_dwFlags;
		tParams.pFormatFilters = a_pFormatFilters;
		tParams.pListener = a_pListener;
		tParams.pUserConfig = a_pUserConfig;
		tParams.pCaption = a_pCaption;
		m_bstrHelp = a_bstrHelp;

		Win32LangEx::CLangIndirectDialogImpl<COpenFilterFrameDlg>::DoModal(a_hParent, reinterpret_cast<LPARAM>(&tParams));

		return m_pFilter.Detach();
	}

	enum { IDC_INNERDLG = 150 };

	BEGIN_DIALOG_EX(0, 0, 320, 220, 0)
		DIALOG_CAPTION(_T("[0409]Select Location[0405]Výběr umístění"))
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | DS_CONTEXTHELP)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 209, 199, 50, 14, WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDCANCEL, 263, 199, 50, 14, WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Help[0405]Nápověda"), IDHELP, 7, 199, 50, 14, WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(COpenFilterFrameDlg)
		COM_INTERFACE_ENTRY(IStorageFilterWindowCallback)
	END_COM_MAP()

	BEGIN_MSG_MAP(COpenFilterFrameDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
		COMMAND_HANDLER(IDHELP, BN_CLICKED, OnHelp)
		CHAIN_MSG_MAP(CDialogResize<COpenFilterFrameDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStartDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_INNERDLG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	// IStorageFilterWindowCallback methods
public:
	STDMETHOD(ForwardOK)()
	{
		if (S_OK == m_pSFWnd->FilterCreate(&m_pFilter))
		{
			EndDialog(IDOK);
			return S_OK;
		}
		return S_FALSE;
	}
	STDMETHOD(ForwardCancel)()
	{
		EndDialog(IDCANCEL);
		return S_OK;
	}
	STDMETHOD(DefaultCommand)(ILocalizedString** UNREF(a_ppName), ILocalizedString** UNREF(a_ppDesc), GUID* UNREF(a_pIconID)) { return E_NOTIMPL; }
	STDMETHOD(DefaultCommandIcon)(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon)) { return E_NOTIMPL; }

	// message handlers
public:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		SParams const* pParams = reinterpret_cast<SParams const*>(a_lParam);

		// center the dialog on the screen
		CenterWindow();

		try
		{
			if (pParams->pCaption != NULL)
			{
				CComBSTR bstrCaption;
				pParams->pCaption->GetLocalized(m_tLocaleID, &bstrCaption);
				if (bstrCaption != NULL)
				{
					SetWindowText(CW2CT(bstrCaption));
				}
			}
			{
				//m_hIcon = DPIUtils::PrepareIconForCaption((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(pParams->dwFlags&EFTCreateNew ? IDI_FILE_SAVE : ID_FWFILE_FOLDER), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR));
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(GetSystemMetrics(SM_CXSMICON));
				if (pParams->dwFlags&EFTCreateNew)
				{
					pSI->GetLayers(ESIFloppy, cRenderer);
				}
				else
				{
					pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.9f, -1, -1));
					pSI->GetLayers(ESIMagnifier, cRenderer, IRTarget(0.85f, 1, 1));
				}
				m_hIcon = cRenderer.get();
			}
			SetIcon(m_hIcon, 0);

			if (m_bstrHelp)
				GetDlgItem(IDHELP).ShowWindow(SW_SHOW);

			CComObject<COpenFilterDlg>* pwndDlg = NULL;
			CComObject<COpenFilterDlg>::CreateInstance(&pwndDlg);
			m_pSFWnd = pwndDlg;
			pwndDlg->Create(m_hWnd, pParams->bstrInitial, pParams->dwFlags, pParams->pFormatFilters, pParams->pUserConfig, m_pContextConfig, this, pParams->pListener, m_tLocaleID);
			HWND hChildDlg = NULL;
			pwndDlg->Handle(&hChildDlg);
			::SetWindowLong(hChildDlg, GWL_ID, IDC_INNERDLG); // misuse the IDD
			RECT rcIDOK;
			RECT rcGap = {0, 0, 0, 4};
			MapDialogRect(&rcGap);
			::GetWindowRect(GetDlgItem(IDOK), &rcIDOK);
			ScreenToClient(&rcIDOK);
			RECT rcClient;
			GetClientRect(&rcClient);
			::MoveWindow(hChildDlg, 0, 0, rcClient.right, rcIDOK.top-rcGap.bottom, FALSE);
			::SetFocus(hChildDlg);

			DlgResize_Init();

			CConfigValue cPosistionX;
			CConfigValue cPosistionY;
			CConfigValue cSizeX;
			CConfigValue cSizeY;
			m_pContextConfig->ItemValueGet(CComBSTR(CFGID_POSITIONX), &cPosistionX);
			m_pContextConfig->ItemValueGet(CComBSTR(CFGID_POSITIONY), &cPosistionY);
			m_pContextConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cSizeX);
			m_pContextConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cSizeY);
			if (cPosistionX.operator LONG() != CW_USEDEFAULT && cPosistionY.operator LONG() != CW_USEDEFAULT &&
				cSizeX.operator LONG() != CW_USEDEFAULT && cSizeY.operator LONG() != CW_USEDEFAULT)
			{
				MoveWindow(cPosistionX.operator LONG(), cPosistionY.operator LONG(), cSizeX.operator LONG(), cSizeY.operator LONG(), FALSE);
			}
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		a_bHandled = FALSE;

		try
		{
			WINDOWPLACEMENT tPos;
			if (GetWindowPlacement(&tPos))
			{
				CComBSTR cCFGID_POSITIONX(CFGID_POSITIONX);
				CComBSTR cCFGID_POSITIONY(CFGID_POSITIONY);
				CComBSTR cCFGID_SIZEX(CFGID_SIZEX);
				CComBSTR cCFGID_SIZEY(CFGID_SIZEY);
				TConfigValue aVals[4];
				BSTR aIDs[4];
				aIDs[0] = cCFGID_POSITIONX;	aVals[0] = CConfigValue((LONG)tPos.rcNormalPosition.left);
				aIDs[1] = cCFGID_POSITIONY;	aVals[1] = CConfigValue((LONG)tPos.rcNormalPosition.top);
				aIDs[2] = cCFGID_SIZEX;		aVals[2] = CConfigValue((LONG)(tPos.rcNormalPosition.right-tPos.rcNormalPosition.left));
				aIDs[3] = cCFGID_SIZEY;		aVals[3] = CConfigValue((LONG)(tPos.rcNormalPosition.bottom-tPos.rcNormalPosition.top));
				m_pContextConfig->ItemValuesSet(4, aIDs, aVals);
			}
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_pSFWnd)
			m_pSFWnd->SendMessage(uMsg, wParam, lParam);
		return 0;
	}

	LRESULT OnOK(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ForwardOK();
		return 0;
	}

	LRESULT OnCancel(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ForwardCancel();
		return 0;
	}

	LRESULT OnHelp(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_bstrHelp)
		{
			CComPtr<IApplicationInfo> pAI;
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			CComBSTR bstr;
			if (pAI) pAI->Account(&bstr, NULL, NULL);
			if (bstr)
			{
				bstr += m_bstrHelp;
				ShellExecute(NULL, _T("open"), bstr, NULL, NULL, SW_SHOW);
			}
		}
		
		return 0;
	}

private:
	struct SParams
	{
		BSTR bstrInitial;
		IEnumUnknowns* pFormatFilters;
		IConfig* pUserConfig;
		DWORD dwFlags;
		IStorageFilterWindowListener* pListener;
		ILocalizedString* pCaption;
	};

private:
	CComPtr<IStorageFilterWindow> m_pSFWnd;
	CComPtr<IConfig> m_pContextConfig;
	CComPtr<IStorageFilter> m_pFilter;
	HICON m_hIcon;
	BSTR m_bstrHelp;
};
