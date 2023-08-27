
#pragma once

#include "MainFrame.h"
#include "ContextHelpDlg.h"


extern __declspec(selectany) GUID const MenuCommandsLocalHelpID = {0x6973c4ae, 0x7b38, 0x47d0, {0xb1, 0x2b, 0x77, 0x2a, 0x18, 0xe8, 0x85, 0xf6}};

// Help->Contents

class ATL_NO_VTABLE CDocumentMenuCommandContents : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandContents, IDS_MN_HELP_LOCAL, IDS_MD_HELP_LOCAL, NULL, 0>
{
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		TCHAR szHelpPath[MAX_PATH] = _T("");
		HtmlHelp(a_hParent, GetHelpFilePath(szHelpPath, itemsof(szHelpPath), a_tLocaleID), HH_DISPLAY_TOC, 0);
		return S_OK;
	}
};

// Help->Index

extern __declspec(selectany) GUID const MenuCommandIndexID = {0x8073964d, 0xfbdd, 0x401b, {0xa8, 0x3b, 0xb7, 0xd5, 0x89, 0x5f, 0x6a, 0x18}};

class ATL_NO_VTABLE CDocumentMenuCommandIndex : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandIndex, IDS_MN_HELP_INDEX, IDS_MD_HELP_INDEX, &MenuCommandIndexID, 0>
{
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			static IRPolyPoint const cover[] =
			{
				{0, 208}, {110, 208}, {113, 214}, {143, 214}, {146, 208}, {256, 208}, {228, 61}, {28, 61},
				//{0, 178}, {110, 178}, {113, 184}, {143, 184}, {146, 178}, {256, 178}, {213, 91}, {43, 91},
			};
			static IRPathPoint const pages[] =
			{
				{41, 55, 31, 4, 0, 0},
				{128, 55, 24, -19, -24, -19},
				{215, 55, 0, 0, -31, 4},
				{239, 196, -44, 7, 0, 0},
				{134, 202, 0, 0, 23, -37},
				{122, 202, -23, -37, 0, 0},
				{17, 196, 0, 0, 44, 7},
				//{56, 85, 31, 4, 0, 0},
				//{128, 85, 15, -21, -15, -21},
				//{200, 85, 0, 0, -31, 4},
				//{239, 166, -44, 7, 0, 0},
				//{134, 172, 0, 0, 23, -37},
				//{122, 172, -23, -37, 0, 0},
				//{17, 166, 0, 0, 44, 7},
			};
			static IRPolyPoint const shadow[] =
			{
				{128, 63}, {128, 184}, {135, 58},
			};

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));

			static IRGridItem const gridY[] = { {EGIFInteger, 208.0f} };//178.0f} };
			static IRCanvas const canvas = {0, 0, 256, 256, 0, itemsof(gridY), NULL, gridY};

			IRFill shadowMat(0x7f000000);
			IRFill fill(0xffa4bd7e);
			IROutlinedFill mat(&fill, pSI->GetMaterial(ESMContrast));
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&canvas, itemsof(cover), cover, pSI->GetMaterial(ESMManipulate));
			cRenderer(&canvas, itemsof(pages), pages, pSI->GetMaterial(ESMBackground));
			cRenderer(&canvas, itemsof(shadow), shadow, &shadowMat);
			*a_phIcon = cRenderer.get();

			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = VK_F1;
				a_pAccel->fVirtFlags = FSHIFT|FVIRTKEY;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		TCHAR szTmp[MAX_PATH] = _T("");
		::GetModuleFileName(NULL, szTmp, MAX_PATH);
		LPCTSTR pszName = _tcsrchr(szTmp, _T('\\'))+1;
		TCHAR szHelpPath[MAX_PATH] = _T("http://wiki.rw-designer.com/");
		if (pszName) _tcscat(szHelpPath, pszName);
		::ShellExecute(0, _T("open"), szHelpPath, 0, 0, SW_SHOWNORMAL);
		return S_OK;
	}
};

// Help->Search

class ATL_NO_VTABLE CDocumentMenuCommandSearch : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandSearch, IDS_MN_HELP_SEARCH, IDS_MD_HELP_SEARCH, NULL, 0>
{
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		::ShellExecute(0, _T("open"), _T("http://www.rw-designer.com/help"), 0, 0, SW_SHOWNORMAL);
		return S_OK;
	}
};

void EnumLocalHelp(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	{
		CComObject<CDocumentMenuCommandContents>* p = NULL;
		CComObject<CDocumentMenuCommandContents>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		a_pCommands->Insert(pTmp);
	}
	{
		CComObject<CDocumentMenuCommandIndex>* p = NULL;
		CComObject<CDocumentMenuCommandIndex>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		a_pCommands->Insert(pTmp);
	}
	{
		CComObject<CDocumentMenuCommandSearch>* p = NULL;
		CComObject<CDocumentMenuCommandSearch>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		a_pCommands->Insert(pTmp);
	}
}


// Help->Ask Online

extern __declspec(selectany) GUID const MenuCommandsAskOnlineID = {0x92dce16f, 0x5091, 0x4980, {0xbd, 0xd5, 0xa6, 0xc3, 0x32, 0x39, 0x43, 0xc5}};

class ATL_NO_VTABLE CDocumentMenuCommandAskOnline : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandAskOnline, IDS_MN_HELP_ASKONLINE, IDS_MD_HELP_ASKONLINE, NULL, 0>
{
public:
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		::ShellExecute(0, _T("open"), _T("http://www.rw-designer.com/forum"), 0, 0, SW_SHOWNORMAL);
		return S_OK;
	}
};

void EnumAskOnline(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandAskOnline>* p = NULL;
	CComObject<CDocumentMenuCommandAskOnline>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	a_pCommands->Insert(pTmp);
}


// Help->Context

extern __declspec(selectany) GUID const MenuCommandsCustomID = {0x8d4cbeb1, 0x26c5, 0x4c5f, {0xbf, 0xe2, 0x91, 0x1, 0x4a, 0x44, 0xa0, 0xb0}};
extern __declspec(selectany) GUID const MenuCommandCustomID = {0x191b0df5, 0x9332, 0x4de1, {0x9e, 0xa9, 0xe7, 0xc5, 0x69, 0xe6, 0xc5, 0xbe}};

HICON GetIconHelpLightbulb(ULONG size);

class ATL_NO_VTABLE CDocumentMenuCommandCustom : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandCustom, IDS_MN_HELP_CUSTOM, IDS_MD_HELP_CUSTOM, &MenuCommandCustomID, 0>
{
public:
	void Init(CMainFrame* a_pFrame, BSTR a_bstrTopicID)
	{
		m_pFrame = a_pFrame;
		if (a_bstrTopicID && a_bstrTopicID[0])
		{
			m_strLayoutHelpPath = COLE2CT(a_bstrTopicID);
		}
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetIconHelpLightbulb(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		return m_strLayoutHelpPath.empty() ? EMCSDisabled : EMCSNormal;
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		if (!m_strLayoutHelpPath.empty())
		{
			if (_tcsncmp(m_strLayoutHelpPath.c_str(), _T("::/"), 3) == 0)
			{
				TCHAR szHelpPath[MAX_PATH] = _T("");
				GetHelpFilePath(szHelpPath, itemsof(szHelpPath), a_tLocaleID);
				HtmlHelp(a_hParent, (std::tstring(szHelpPath)+m_strLayoutHelpPath).c_str(), HH_DISPLAY_TOPIC, 0);
			}
			else
			{
				HtmlHelp(a_hParent, m_strLayoutHelpPath.c_str(), HH_DISPLAY_TOPIC, 0);
			}
		}
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
	std::tstring m_strLayoutHelpPath;
};

#include "ConfigIDsApp.h" // TODO: remove->replace by single cfgid

void EnumCustom(IOperationContext* a_pStates, IConfig* a_pConfig, IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_HELPTOPIC), &cVal);
	CComObject<CDocumentMenuCommandCustom>* p = NULL;
	CComObject<CDocumentMenuCommandCustom>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame, cVal);
	a_pCommands->Insert(pTmp);
}

void ConfigCustom(IConfigWithDependencies* a_pConfig)
{
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_HELPTOPIC), _SharedStringTable.GetStringAuto(IDS_CFGID_HELPTOPIC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HELPTOPIC_DESC), CConfigValue(L""), NULL, 0, NULL);
}

// Help->About

extern __declspec(selectany) GUID const MenuCommandsAboutID = {0x73b3e980, 0x3033, 0x425d, {0x86, 0x71, 0xbf, 0x7e, 0x9b, 0x6f, 0xe2, 0x3e}};

class ATL_NO_VTABLE CDocumentMenuCommandAbout : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandAbout, IDS_MN_HELP_ABOUT, IDS_MD_HELP_ABOUT, NULL, 0>,
	public ILocalizedString
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

BEGIN_COM_MAP(CDocumentMenuCommandAbout)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	COM_INTERFACE_ENTRY(ILocalizedString)
END_COM_MAP()

	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;
			OLECHAR szTempl[64] = L"";
			Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_MN_HELP_ABOUT, szTempl, itemsof(szTempl), LANGIDFROMLCID(a_tLCID));
			OLECHAR szBuf[256] = L"";
			CComBSTR bstrCaption;
			CComPtr<ILocalizedString> pStr;
			if (SUCCEEDED(m_pFrame->GetThread()->M_DesignerAppInfo()->Name(&pStr)) && pStr)
				pStr->GetLocalized(a_tLCID, &bstrCaption);
			_snwprintf(szBuf, itemsof(szBuf), szTempl, bstrCaption ? bstrCaption.m_str : L"");
			*a_pbstrString = CComBSTR(szBuf).Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = this;
			AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = m_pFrame->GetThread()->M_DesignerAppInfo() ? EMCSNormal : EMCSDisabled;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		if (m_pFrame->GetThread()->M_DesignerAppInfo())
			m_pFrame->GetThread()->M_DesignerAppInfo()->AboutBox(a_hParent, a_tLocaleID);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

template<UINT t_uIDName, UINT t_uIDDesc>
class ATL_NO_VTABLE CDocumentMenuCommandRegister : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandRegister<t_uIDName, t_uIDDesc>, t_uIDName, t_uIDDesc, NULL, 0>
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

BEGIN_COM_MAP(CDocumentMenuCommandAbout)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		if (m_pFrame->GetThread()->M_DesignerAppInfo())
			m_pFrame->GetThread()->M_DesignerAppInfo()->LicenseBox(a_hParent, a_tLocaleID);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumAbout(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* UNREF(a_pDocument), IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComPtr<IDocumentMenuCommand> pTmp;
	ELicensingMode eMode = ELMBeta;
	if (a_pFrame->GetThread()->M_DesignerAppInfo() && a_pFrame->GetThread()->M_DesignerAppInfo()->LicensingMode(&eMode) == S_OK)
	{
		if (eMode == ELMDonate)
		{
			CComObject<CDocumentMenuCommandRegister<IDS_MN_HELP_DONATE, IDS_MD_HELP_DONATE> >* p = NULL;
			CComObject<CDocumentMenuCommandRegister<IDS_MN_HELP_DONATE, IDS_MD_HELP_DONATE> >::CreateInstance(&p);
			pTmp = p;
			p->Init(a_pFrame);
		}
		else if (eMode == ELMBeta)
		{
			CComObject<CDocumentMenuCommandRegister<IDS_MN_HELP_BETA, IDS_MD_HELP_BETA> >* p = NULL;
			CComObject<CDocumentMenuCommandRegister<IDS_MN_HELP_BETA, IDS_MD_HELP_BETA> >::CreateInstance(&p);
			pTmp = p;
			p->Init(a_pFrame);
		}
		else
		{
			CComObject<CDocumentMenuCommandRegister<IDS_MN_HELP_REGISTER, IDS_MD_HELP_REGISTER> >* p = NULL;
			CComObject<CDocumentMenuCommandRegister<IDS_MN_HELP_REGISTER, IDS_MD_HELP_REGISTER> >::CreateInstance(&p);
			pTmp = p;
			p->Init(a_pFrame);
		}
		a_pCommands->Insert(pTmp);
	}
	CComObject<CDocumentMenuCommandAbout>* p = NULL;
	CComObject<CDocumentMenuCommandAbout>::CreateInstance(&p);
	pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}

// Help->Context help

extern __declspec(selectany) GUID const MenuCommandsContextID = {0x2aba9849, 0xcffb, 0x4ac4, {0xa3, 0x20, 0x24, 0xc9, 0x20, 0x3f, 0x3c, 0x30}};

HICON GetIconHelpContext(ULONG size);

class ATL_NO_VTABLE CDocumentMenuCommandContext : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandContext, IDS_MN_HELP_CONTEXT, IDS_MD_HELP_CONTEXT, &MenuCommandsContextID, 0>
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetIconHelpContext(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			SetCapture(*m_pFrame);
			SetCursor(LoadCursor(NULL, IDC_HELP));
			MSG tMsg;
			while (GetMessage(&tMsg, NULL, 0, 0))
			{
				//if (tMsg.hwnd == a_hParent)
				//{
					if (tMsg.message >= WM_KEYFIRST && tMsg.message <= WM_KEYLAST)
						break;
					if (tMsg.message == WM_KILLFOCUS || tMsg.message == WM_CAPTURECHANGED)
					{
						TranslateMessage(&tMsg);
						DispatchMessage(&tMsg);
						break;
					}
					if (tMsg.message >= WM_MOUSEFIRST && tMsg.message <= WM_MOUSELAST)
					{
						if (tMsg.message == WM_LBUTTONDOWN)
						{
							POINT tPt = {GET_X_LPARAM(tMsg.lParam), GET_Y_LPARAM(tMsg.lParam)};
							ClientToScreen(tMsg.hwnd, &tPt);
							POINT tPt2;
							HWND hPrev = *m_pFrame;
							tPt2 = tPt;
							ScreenToClient(hPrev, &tPt2);
							HWND hClicked = ChildWindowFromPointEx(*m_pFrame, tPt2, CWP_SKIPINVISIBLE);
							if (hClicked)
							{
								while (hClicked != hPrev)
								{
									hPrev = hClicked;
									tPt2 = tPt;
									ScreenToClient(hPrev, &tPt2);
									hClicked = ChildWindowFromPointEx(hPrev, tPt2, CWP_SKIPINVISIBLE);
								}
								HELPINFO tHI;
								tHI.cbSize = sizeof tHI;
								tHI.iContextType = HELPINFO_WINDOW;
								tHI.iCtrlId = GetWindowLong(hClicked, GWL_ID);
								tHI.dwContextId = tHI.iCtrlId; // ???
								tHI.hItemHandle = hClicked;
								tHI.MousePos = tPt;
								SendMessage(hClicked, WM_HELP, 0, reinterpret_cast<LPARAM>(&tHI));
							}
							break;
						}
						else if (tMsg.message == WM_RBUTTONDOWN || tMsg.message == WM_MBUTTONDOWN || tMsg.message == WM_XBUTTONDOWN)
						{
							break;
						}
					}
//				}
				else
				{
					TranslateMessage(&tMsg);
					DispatchMessage(&tMsg);
				}
			}
			ReleaseCapture();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CMainFrame* m_pFrame;
};

void EnumContext(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* UNREF(a_pDocument), IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandContext>* p = NULL;
	CComObject<CDocumentMenuCommandContext>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}

