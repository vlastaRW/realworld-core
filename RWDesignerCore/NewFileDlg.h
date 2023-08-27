// NewFileDlg.h : Declaration of the CNewFileDlg

#pragma once

#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <Win32LangEx.h>
#include "StartViewWnd.h"
#include <MultiLanguageString.h>
#include <WTL_PopupButton.h>
#include <ContextMenuWithIcons.h>
#include <RWImaging.h>
#include <IconRenderer.h>


// CNewFileDlg

class CNewFileDlg : 
	public CStartViewPageImpl<CNewFileDlg>,
	public Win32LangEx::CLangDialogImpl<CNewFileDlg>,
	public CContextMenuWithIcons<CNewFileDlg>,
	public CDialogResize<CNewFileDlg>
{
public:
	CNewFileDlg() : m_nTimeStamp(0) {}

	void WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig)
	{
		m_pSubmenuTemplate.Attach(new CMultiLanguageString(L"[0409]Create as \"%s\"[0405]Vytvořit jako \"%s\""));
		m_pClbk = a_pCallback;
		m_tLocaleID = a_tLocaleID;
		m_pAppConfig = a_pAppConfig;
		InitBuilders();
		Create(a_hParent);
		MoveWindow(a_prc);
	}
	virtual void OnFinalMessage(HWND a_hWnd);

	enum { IDD = IDD_NEWFILEDLG, WM_RESTORESELECTION = WM_APP+837 };

BEGIN_MSG_MAP(CNewFileDlg)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	NOTIFY_HANDLER(IDC_LIST, LVN_ITEMACTIVATE, OnItemActivateList)
	NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnItemChangedList)
	NOTIFY_HANDLER(IDC_LIST, LVN_GETINFOTIP, OnGetInfoTipList)
	MESSAGE_HANDLER(WM_SIZE, OnSize);
	COMMAND_HANDLER(IDOK, BN_SUBMENUCLICKED, OnSubMenuClicked)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_RESTORESELECTION, OnRestoreSelection)
	CHAIN_MSG_MAP(CDialogResize<CNewFileDlg>)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CNewFileDlg>)
	REFLECT_NOTIFICATIONS()
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CNewFileDlg)
	DLGRESIZE_CONTROL(IDC_FILL, DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_SEPLINE, DLSZ_MOVE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_PANEL, DLSZ_MOVE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnItemActivateList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnItemChangedList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnSubMenuClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ClickedDefault();
		return 0;
	}
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		BOOL b;
		CDialogResize<CNewFileDlg>::OnSize(uMsg, wParam, lParam, b);
		m_wndList.Arrange(LVA_ALIGNTOP);
		return 0;
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return (LRESULT)m_hBGBrush.m_hBrush; }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ SetBkColor((HDC)a_wParam, m_clrBG); return (LRESULT)m_hBGBrush.m_hBrush; }
	LRESULT OnRestoreSelection(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (!a_bBeforeAccel || m_iActiveItem < 0 || m_iActiveItem >= int(m_cTemplates.size()))
			return S_FALSE;
		if ((a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_SYSKEYDOWN) &&
			a_pMsg->wParam != VK_MENU && a_pMsg->wParam != VK_SHIFT && a_pMsg->wParam != VK_CONTROL &&
			a_pMsg->wParam != VK_LMENU && a_pMsg->wParam != VK_LSHIFT && a_pMsg->wParam != VK_LCONTROL &&
			a_pMsg->wParam != VK_RMENU && a_pMsg->wParam != VK_RSHIFT && a_pMsg->wParam != VK_RCONTROL)
		{
			if ((a_pMsg->wParam == 'V' && (GetKeyState(VK_CONTROL)&0x8000)) ||
				(a_pMsg->wParam == VK_INSERT && (GetKeyState(VK_SHIFT)&0x8000)))
			{
				CComQIPtr<IDesignerWizardClipboard> pDWCB(m_cTemplates[m_iActiveItem].p);
				if (pDWCB)
				{
					ClickedDefault();
					return S_OK;
				}
			}
		}
		return S_FALSE;
	}

	STDMETHOD(Destroy)()
	{
		m_pClbk = NULL;
		return CStartViewPageImpl<CNewFileDlg>::Destroy();
	}

	// IStartViewWnd methods
public:
	STDMETHOD(Activate)();
	STDMETHOD(Deactivate)();
	STDMETHOD(ClickedDefault)();
	STDMETHOD(OnIdle)();

	static HRESULT InitConfig(IConfigWithDependencies* a_pConfig);

private:
	struct STemplate
	{
		CLSID id;
		CComPtr<IDesignerWizard> p;
	};
	typedef vector<STemplate> CTemplates;
	typedef std::vector<CComPtr<IDocumentBuilder> > CBuilders;

	void InitBuilders();
	void SetDefaultButtonState(ILocalizedString* a_pText, bool a_bEnabled, bool a_bChoices);
	HBITMAP CreateWatermark(IDocumentImage* a_pImage, COLORREF a_clrBG);

	HRESULT HasChoices();
	HRESULT GetChoiceProps(ULONG a_nIndex, ILocalizedString** a_ppName, ULONG a_nSize, HICON* a_phIcon);
	HRESULT ClickedChoice(ULONG a_nIndex);
	void InitWizardList();

	friend struct initwizarddialog;

private:
	CComPtr<IStartViewCallback> m_pClbk;
	CComPtr<IConfig> m_pAppConfig;
	CBuilders m_cBuilders;
	int m_iActiveItem;
	int m_iAutoSelectedItem;
	CTemplates m_cTemplates;
	CListViewCtrl m_wndList;
	//CFont m_font;
	CComPtr<IConfigWnd> m_pConfigWnd;
	CComPtr<ILocalizedString> m_pSubmenuTemplate;
	CButtonWithPopup m_wndOKButton;
	CWindow m_wndFill;
	COLORREF m_clrBG;
	CBrush m_hBGBrush;
	ULONG m_nTimeStamp;
};


class CStartViewPageFactoryNewDocument : public CStartViewPageFactory<CNewFileDlg, &CLSID_StartViewPageFactoryNewDocument, IDS_STARTPAGENEWDOCUMENT_NAME, IDS_STARTPAGENEWDOCUMENT_DESC, 0, CNewFileDlg::InitConfig>, public IAnimatedIcon
{
public:
	DECLARE_CLASSFACTORY_SINGLETON(CStartViewPageFactoryNewDocument)

BEGIN_COM_MAP(CStartViewPageFactoryNewDocument)
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
		TAPPHASES = 5,
		TOTALPHASES = MOVEPHASES+MOVEPHASES+TAPPHASES+TAPPHASES,
		STEPLENGTH = 25,
		PAUSE = 4000,
		OFFSET = 12,
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
		ULONG cycle = STEPLENGTH*TOTALPHASES+PAUSE;
		a_nMSElapsed %= cycle;
		if (a_nMSElapsed < STEPLENGTH*TOTALPHASES)
		{
			ULONG phase = a_nMSElapsed/STEPLENGTH;
			if (a_pNextPhase)
			{
				ULONG delta = a_nMSElapsed-phase*STEPLENGTH;
				*a_pNextPhase = STEPLENGTH-delta+OFFSET;
			}
			if (phase < MOVEPHASES)
				return phase;
			phase -= MOVEPHASES;
			if (phase < TAPPHASES)
				return MOVEPHASES-phase;
			phase -= TAPPHASES;
			if (phase < TAPPHASES)
				return MOVEPHASES-TAPPHASES+phase;
			phase -= TAPPHASES;
			return MOVEPHASES-phase;
		}
		a_nMSElapsed -= STEPLENGTH*TOTALPHASES;
		if (a_pNextPhase)
		{
			*a_pNextPhase = PAUSE-a_nMSElapsed+OFFSET;
		}
		return 0;
	}
	STDMETHOD_(HICON, Icon)(ULONG a_nPhase, ULONG a_nSize)
	{
		try
		{
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPolyPoint const page[] =
			{
				{76, 0}, {200, 0}, {200, 256}, {0, 256}, {0, 76},
			};
			static IRPolyPoint const corner[] =
			{
				{5, 80}, {80, 80}, {80, 5},
			};
			static IRPathPoint const wandBot1[] =
			{
				{203, 180, 3.90524, -3.90524, 0, 0},
				{217, 180, 0, 0, -3.90524, -3.90524},
				{252, 215, 3.90524, 3.90524, 0, 0},
				{252, 229, 0, 0, 3.90524, -3.90524},
				{229, 252, -3.90524, 3.90524, 0, 0},
				{215, 252, 0, 0, 3.90524, 3.90524},
				{180, 217, -3.90524, -3.90524, 0, 0},
				{180, 203, 0, 0, -3.90524, 3.90524},
			};
			static IRPolyPoint const wandMid1[] =
			{
				{105, 77}, {225, 197}, {197, 225}, {77, 105},
			};
			static IRPathPoint const wandTop1[] =
			{
				{74, 52, 3.90524, -3.90524, 0, 0},
				{88, 52, 0, 0, -3.90524, -3.90524},
				{124, 88, 3.90524, 3.90524, 0, 0},
				{124, 102, 0, 0, 3.90524, -3.90524},
				{102, 124, -3.90524, 3.90524, 0, 0},
				{88, 124, 0, 0, 3.90524, 3.90524},
				{52, 88, -3.90524, -3.90524, 0, 0},
				{52, 74, 0, 0, -3.90524, 3.90524},
			};
			static IRPathPoint const wandBot2[] =
			{
				{190, 180, 2.90138, -4.59065, 0, 0},
				{204, 178, 0, 0, -4.77428, -3.12318},
				{249, 207, 5.17998, 3.38855, 0, 0},
				{253, 222, 0, 0, 2.9077, -4.93054},
				{235, 251, -3.00446, 5.09467, 0, 0},
				{221, 254, 0, 0, 5.17763, 3.66664},
				{176, 222, -4.76353, -3.37334, 0, 0},
				{173, 208, 0, 0, -2.99297, 4.73557},
			};
			static IRPolyPoint const wandMid2[] =
			{
				{82, 104}, {216, 193}, {195, 227}, {61, 134},
			};
			static IRPathPoint const wandTop2[] =
			{
				{53, 89, 2.83686, -3.87886, 0, 0},
				{64, 87, 0, 0, -3.72562, -2.43978},
				{100, 110, 4.01086, 2.62657, 0, 0},
				{102, 122, 0, 0, 2.86893, -4.13069},
				{86, 146, -2.94946, 4.24661, 0, 0},
				{73, 148, 0, 0, 3.98796, 2.82109},
				{38, 123, -3.69917, -2.61681, 0, 0},
				{36, 112, 0, 0, -2.9128, 3.98269},
			};
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			IROutlinedFill matPage(pSI->GetMaterial(ESMInterior, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IRStroke matCorner(0x9f000000|(0xffffff&pSI->GetSRGBColor(ESMContrast)), 1.0/40.0f);

			IRFill matWandMidFill(0xffa38863);
			//IRFill matSandFill(0xffc8bc69);
			IRFill matWandEndFill(0xffd5ebf1);//0xfffbe99e);//0xffa2dfed);
			IROutlinedFill matWandMid(&matWandMidFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IROutlinedFill matWandEnd(&matWandEndFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);

			IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			IRGridItem gridX[] = {{0, 0}, {0, 200}};
			IRGridItem gridY[] = {{0, 0}, {0, 256}};
			IRCanvas canvasPage = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

			IRTarget pageTarget(0.9f, -1, -1);
			IRTarget wandTarget(0.9f, 1, 1);
			cRenderer(&canvasPage, itemsof(page), page, &matPage, pageTarget);
			cRenderer(&canvasPage, itemsof(corner), corner, &matCorner, pageTarget);

			float w1 = float(a_nPhase)/MOVEPHASES;//0.0f;
			float w0 = 1.0f-w1;

			IRPolyPoint wandMid[sizeof(wandMid1)/sizeof(*wandMid1)];
			for (size_t i = 0; i < itemsof(wandMid); ++i)
			{
				wandMid[i].x = w0*wandMid1[i].x + w1*wandMid2[i].x;
				wandMid[i].y = w0*wandMid1[i].y + w1*wandMid2[i].y;
			}
			cRenderer(&canvas, itemsof(wandMid), wandMid, &matWandMid/*pSI->GetMaterial(ESMContrast)*/, wandTarget);

			IRPathPoint wandBot[sizeof(wandBot1)/sizeof(*wandBot1)];
			for (size_t i = 0; i < itemsof(wandBot); ++i)
			{
				wandBot[i].x = w0*wandBot1[i].x + w1*wandBot2[i].x;
				wandBot[i].y = w0*wandBot1[i].y + w1*wandBot2[i].y;
				wandBot[i].tnx = w0*wandBot1[i].tnx + w1*wandBot2[i].tnx;
				wandBot[i].tny = w0*wandBot1[i].tny + w1*wandBot2[i].tny;
				wandBot[i].tpx = w0*wandBot1[i].tpx + w1*wandBot2[i].tpx;
				wandBot[i].tpy = w0*wandBot1[i].tpy + w1*wandBot2[i].tpy;
			}
			cRenderer(&canvas, itemsof(wandBot), wandBot, &matWandEnd, wandTarget);

			IRPathPoint wandTop[sizeof(wandTop1)/sizeof(*wandTop1)];
			for (size_t i = 0; i < itemsof(wandTop); ++i)
			{
				wandTop[i].x = w0*wandTop1[i].x + w1*wandTop2[i].x;
				wandTop[i].y = w0*wandTop1[i].y + w1*wandTop2[i].y;
				wandTop[i].tnx = w0*wandTop1[i].tnx + w1*wandTop2[i].tnx;
				wandTop[i].tny = w0*wandTop1[i].tny + w1*wandTop2[i].tny;
				wandTop[i].tpx = w0*wandTop1[i].tpx + w1*wandTop2[i].tpx;
				wandTop[i].tpy = w0*wandTop1[i].tpy + w1*wandTop2[i].tpy;
			}
			cRenderer(&canvas, itemsof(wandTop), wandTop, &matWandEnd, wandTarget);

			//IRPolyPoint center1 = {120, 140};
			//IRPolyPoint center2 = {134, 92};
			//IRPolyPoint size = {84, 63};
			//IRPolyPoint corner = {52, 31};
			//float off = 4;
			//float angle1 = 0.261799f;//0;
			//float angle2 = 1.570796327f;//0.261799f;
			//float iphase = (a_nPhase%(MOVEPHASES+1))/float(MOVEPHASES);
			//float phase = 1.0f-iphase;
			//IRPolyPoint center = {center1.x*phase+center2.x*iphase, center1.y*phase+center2.y*iphase};
			//float angle = angle1*phase+angle2*iphase;
			//float s = sinf(angle);
			//float c = cosf(angle);
			//IRPolyPoint document[] =
			//{
			//	{center.x-size.x*c+size.y*s, center.y-size.x*s-size.y*c},
			//	{center.x+size.x*c+size.y*s, center.y+size.x*s-size.y*c},
			//	{center.x+size.x*c-size.y*s, center.y+size.x*s+size.y*c},
			//	//{center.x-size.x*c-size.y*s, center.y-size.x*s+size.y*c},
			//	{center.x-corner.x*c-size.y*s, center.y-corner.x*s+size.y*c},
			//	{center.x-size.x*c-corner.y*s, center.y-size.x*s+corner.y*c},
			//};
			//IRGridItem gridDocX[] = {{0, center2.x-size.y}, {0, center2.x+size.y}};
			//IRGridItem gridDocY[] = {{0, center2.y-size.x}, {0, center2.y+size.x}};
			//IRCanvas canvasDoc = {0, 0, 256, 256, itemsof(gridDocX), itemsof(gridDocY), gridDocX, gridDocY};
			//IROutlinedFill matDoc(pSI->GetMaterial(ESMInterior, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			//cRenderer(&canvasDoc, itemsof(document), document, &matDoc);
			//IRPolyPoint cornerPoly[] =
			//{
			//	{center.x-(corner.x-off)*c-(size.y-off)*s, center.y-(corner.x-off)*s+(size.y-off)*c},
			//	{center.x-(corner.x-off)*c-(corner.y-off)*s, center.y-(corner.x-off)*s+(corner.y-off)*c},
			//	{center.x-(size.x-off)*c-(corner.y-off)*s, center.y-(size.x-off)*s+(corner.y-off)*c},
			//};
			//IRStroke matCorner(pSI->GetSRGBColor(ESMContrast), a_nSize*0.00025f);
			//cRenderer(&canvasDoc, itemsof(cornerPoly), cornerPoly, &matCorner);

			//cRenderer(&canvasFolderFront, itemsof(folderFront), folderFront, &matFolder);
			//cRenderer(&canvasClip, itemsof(clipFront), clipFront, &matClip);

			return cRenderer.get();
		}
		catch (...)
		{
			return NULL;
		}
	}
};
OBJECT_ENTRY_AUTO(__uuidof(StartViewPageFactoryNewDocument), CStartViewPageFactoryNewDocument)
