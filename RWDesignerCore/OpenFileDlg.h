// OpenFileDlg.h : Declaration of the COpenFileDlg

#pragma once

#include "RWDesignerCore.h"
#include "StartViewWnd.h"
#include "OpenAsChoices.h"
#include <Win32LangEx.h>
#include <WTL_PopupButton.h>
#include <ContextMenuWithIcons.h>
#include <IconRenderer.h>
#include <math.h>


// COpenFileDlg

class COpenFileDlg : 
	public CStartViewPageImpl<COpenFileDlg>, COpenAsChoicesHelper,
	public Win32LangEx::CLangIndirectDialogImpl<COpenFileDlg>,
	public CContextMenuWithIcons<COpenFileDlg>,
	public IStorageFilterWindowCallback
{
public:
	BEGIN_DIALOG_EX(0, 0, 186, 151, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_PUSHBUTTON(_T("[0409]Open[0405]Otevřít"), IDOK, 129, 130, 50, 14, BS_DEFPUSHBUTTON | WS_TABSTOP | WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(COpenFileDlg)
		COM_INTERFACE_ENTRY(IStartViewPage)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IStorageFilterWindowCallback)
	END_COM_MAP()

	BEGIN_MSG_MAP(COpenFileDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_SUBMENUCLICKED, OnSubMenuClicked)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		CHAIN_MSG_MAP(CContextMenuWithIcons<COpenFileDlg>)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig);
	IConfig* M_AppConfig() const { return m_pAppConfig; }
	virtual void OnFinalMessage(HWND a_hWnd) { Release(); }

	// handlers
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSubMenuClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ClickedDefault();
		return 0;
	}

	// IStartViewWnd methods
public:
	STDMETHOD(Activate)();
	STDMETHOD(Deactivate)();
	STDMETHOD(ClickedDefault)();

	// IChildWindow methods
public:
	STDMETHOD(Destroy)();
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IStorageFilterWindowCallback methods
public:
	STDMETHOD(ForwardOK)()
	{
		m_pClbk->OnOKEx();
		return S_OK;
	}
	STDMETHOD(ForwardCancel)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(DefaultCommand)(ILocalizedString** UNREF(a_ppName), ILocalizedString** UNREF(a_ppDesc), GUID* UNREF(a_pIconID)) { return E_NOTIMPL; }
	STDMETHOD(DefaultCommandIcon)(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon)) { return E_NOTIMPL; }

private:
	HRESULT HasChoices()
	{
		return BuilderCount(M_AppConfig()) > 1 ? S_OK : S_FALSE;
	}
	HRESULT GetChoiceProps(ULONG a_nIndex, ILocalizedString** a_ppName, ULONG a_nSize, HICON* a_phIcon);
	HRESULT ClickedChoice(ULONG a_nIndex);

private:
	CComPtr<IStartViewCallback> m_pClbk; 
	CComPtr<IStorageFilterWindow> m_pWnd;
	CComPtr<IInputManager> m_pInMgr;
	CComPtr<IStorageManager> m_pStorageManager;
	CComPtr<IConfig> m_pStorageContext;
	CComPtr<IConfig> m_pAppConfig;
	CComPtr<IEnumUnknownsInit> m_pBuilders;
	CComPtr<ILocalizedString> m_pSubmenuTemplate;
	CButtonWithPopup m_wndOKButton;
	RECT m_rcGaps;
};


class CStartViewPageFactoryOpenFile : public CStartViewPageFactory<COpenFileDlg, &CLSID_StartViewPageFactoryOpenFile, IDS_STARTPAGEOPENFILE_NAME, IDS_STARTPAGEOPENFILE_DESC, 0>, public IAnimatedIcon
{
public:
	DECLARE_CLASSFACTORY_SINGLETON(CStartViewPageFactoryOpenFile)

BEGIN_COM_MAP(CStartViewPageFactoryOpenFile)
	COM_INTERFACE_ENTRY(IStartViewPageFactory)
	COM_INTERFACE_ENTRY(IAnimatedIcon)
END_COM_MAP()

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = Icon(0, a_nSize);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IAnimatedIcon methods
public:
	enum
	{
		INITIAL = 500,
		MOVEPHASES = 9,
		MOVEOUT = 75,
		PAUSEOUT = 2000,
		MOVEIN = 75,
		PAUSEIN = 4000,
		OFFSET = 25,
	};
	STDMETHOD_(ULONG, Phase)(ULONG a_nMSElapsed, ULONG* a_pNextPhase)
	{
		if (a_nMSElapsed < INITIAL)
		{
			if (a_pNextPhase)
				*a_pNextPhase = INITIAL-a_nMSElapsed+OFFSET;
			return 0;
		}
		a_nMSElapsed -= INITIAL;
		ULONG cycle = MOVEOUT*MOVEPHASES+PAUSEOUT+MOVEIN*MOVEPHASES+PAUSEIN;
		a_nMSElapsed %= cycle;
		if (a_nMSElapsed < MOVEOUT*MOVEPHASES)
		{
			ULONG phase = a_nMSElapsed/MOVEOUT;
			if (a_pNextPhase)
			{
				ULONG delta = a_nMSElapsed-phase*MOVEOUT;
				*a_pNextPhase = MOVEOUT-delta+OFFSET;
			}
			return phase;
		}
		a_nMSElapsed -= MOVEOUT*MOVEPHASES;
		if (a_nMSElapsed < PAUSEOUT)
		{
			if (a_pNextPhase)
			{
				*a_pNextPhase = PAUSEOUT-a_nMSElapsed+OFFSET;
			}
			return MOVEPHASES;
		}
		a_nMSElapsed -= PAUSEOUT;
		if (a_nMSElapsed < MOVEIN*MOVEPHASES)
		{
			ULONG phase = a_nMSElapsed/MOVEIN;
			if (a_pNextPhase)
			{
				ULONG delta = a_nMSElapsed-phase*MOVEIN;
				*a_pNextPhase = MOVEIN-delta+OFFSET;
			}
			return MOVEPHASES-phase;
		}
		a_nMSElapsed -= MOVEIN*MOVEPHASES;
		if (a_pNextPhase)
		{
			*a_pNextPhase = PAUSEIN-a_nMSElapsed+OFFSET;
		}
		return 0;
	}
	STDMETHOD_(HICON, Icon)(ULONG a_nPhase, ULONG a_nSize)
	{
		try
		{
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPolyPoint const clipBack[] =
			{
				{150, 224}, {153, 252}, {168, 252}, {168, 216}, {88, 216}, {88, 252}, {103, 252}, {106, 224},
			};
			static IRPathPoint const folderBack[] =
			{
				{240, 76, 0, 0, 0, -4.41828},
				{240, 230, 0, 0, 0, 0},
				{16, 230, 0, 0, 0, 0},
				{16, 40, 0, -4.41828, 0, 0},
				{24, 32, 0, 0, -4.41828, 0},
				{99, 32, 2.20914, 0, 0, 0},
				{108, 37, 0, 0, -2.76142, -2.76142},
				{143, 68, 0, 0, 0, 0},
				{232, 68, 4.41827, 0, 0, 0},
			};
			static IRPathPoint const folderFront[] =
			{
				{0, 120, 0, -4.41828, 0, 0},
				{8, 112, 0, 0, -4.41828, 0},
				{130, 112, 0, 0, 0, 0},
				{178, 76, 0, 0, 0, 0},
				{248, 76, 4.41827, 0, 0, 0},
				{256, 84, 0, 0, 0, -4.41828},
				{240, 230, 0, 0, 0, 0},
				{16, 230, 0, 0, 0, 0},
			};
			static IRPathPoint const clipFront[] =
			{
				{62, 252, 0, 0, 0, 0},
				{54, 193, -5.22546, -22.6437, 0, 0},
				{91, 152, 0, 0, -25.66, 0},
				{165, 152, 25.66, 0, 0, 0},
				{202, 193, 0, 0, 5.22546, -22.6437},
				{194, 252, 0, 0, 0, 0},
				{153, 252, 0, 0, 0, 0},
				{158, 193, 0, 0, 0, 0},
				{98, 193, 0, 0, 0, 0},
				{103, 252, 0, 0, 0, 0},
			};
			IRFill matFolderFill(0xfffbe99e);//f7d159);
			IRFill matClipFill(0xffd5ebf1);//a2dfed);//ffffff);
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			IROutlinedFill matFolder(&matFolderFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IROutlinedFill matClip(&matClipFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			IRGridItem gridFBX[] = {{0, 16}, {0, 240}};
			IRGridItem gridFBY[] = {{0, 32}, {0, 68}, {0, 230}};
			IRCanvas canvasFolderBack = {0, 0, 256, 256, itemsof(gridFBX), itemsof(gridFBY), gridFBX, gridFBY};
			IRGridItem gridFF[] = {{0, 76}, {0, 112}, {0, 230}};
			IRCanvas canvasFolderFront = {0, 0, 256, 256, 0, itemsof(gridFF), NULL, gridFF};
			IRCanvas canvasClip = {0, 0, 256, 256, 0, 0, NULL, NULL};

			cRenderer(&canvasClip, itemsof(clipBack), clipBack, &matClip);
			cRenderer(&canvasFolderBack, itemsof(folderBack), folderBack, &matFolder);

			IRPolyPoint center1 = {120, 140};
			IRPolyPoint center2 = {134, 92};
			IRPolyPoint size = {84, 63};
			IRPolyPoint corner = {52, 31};
			float off = 4;
			float angle1 = 0.261799f;//0;
			float angle2 = 1.570796327f;//0.261799f;
			float iphase = (a_nPhase%(MOVEPHASES+1))/float(MOVEPHASES);
			float phase = 1.0f-iphase;
			IRPolyPoint center = {center1.x*phase+center2.x*iphase, center1.y*phase+center2.y*iphase};
			float angle = angle1*phase+angle2*iphase;
			float s = sinf(angle);
			float c = cosf(angle);
			IRPolyPoint document[] =
			{
				{center.x-size.x*c+size.y*s, center.y-size.x*s-size.y*c},
				{center.x+size.x*c+size.y*s, center.y+size.x*s-size.y*c},
				{center.x+size.x*c-size.y*s, center.y+size.x*s+size.y*c},
				//{center.x-size.x*c-size.y*s, center.y-size.x*s+size.y*c},
				{center.x-corner.x*c-size.y*s, center.y-corner.x*s+size.y*c},
				{center.x-size.x*c-corner.y*s, center.y-size.x*s+corner.y*c},
			};
			IRGridItem gridDocX[] = {{0, center2.x-size.y}, {0, center2.x+size.y}};
			IRGridItem gridDocY[] = {{0, center2.y-size.x}, {0, center2.y+size.x}};
			IRCanvas canvasDoc = {0, 0, 256, 256, itemsof(gridDocX), itemsof(gridDocY), gridDocX, gridDocY};
			IROutlinedFill matDoc(pSI->GetMaterial(ESMInterior, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			cRenderer(&canvasDoc, itemsof(document), document, &matDoc);
			IRPolyPoint cornerPoly[] =
			{
				{center.x-(corner.x-off)*c-(size.y-off)*s, center.y-(corner.x-off)*s+(size.y-off)*c},
				{center.x-(corner.x-off)*c-(corner.y-off)*s, center.y-(corner.x-off)*s+(corner.y-off)*c},
				{center.x-(size.x-off)*c-(corner.y-off)*s, center.y-(size.x-off)*s+(corner.y-off)*c},
			};
			IRStroke matCorner(pSI->GetSRGBColor(ESMContrast), a_nSize*0.00025f);
			cRenderer(&canvasDoc, itemsof(cornerPoly), cornerPoly, &matCorner);

			cRenderer(&canvasFolderFront, itemsof(folderFront), folderFront, &matFolder);
			cRenderer(&canvasClip, itemsof(clipFront), clipFront, &matClip);

			return cRenderer.get();
		}
		catch (...)
		{
			return NULL;
		}
	}
};
OBJECT_ENTRY_AUTO(__uuidof(StartViewPageFactoryOpenFile), CStartViewPageFactoryOpenFile)
