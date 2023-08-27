// RecentFileDlg.h : Declaration of the CRecentFileDlg

#pragma once

#include "RWDesignerCore.h"
#include <Win32LangEx.h>
#include "StartViewWnd.h"
#include <ObserverImpl.h>
#include "RecentFiles.h"
#include "OpenAsChoices.h"
#include <RWThumbnails.h>
#include <ContextMenuWithIcons.h>
#include <MultiLanguageString.h>
#include <IconRenderer.h>


// CRecentFileDlg

class CRecentFileDlg : 
	public Win32LangEx::CLangIndirectDialogImpl<CRecentFileDlg>,
	public CStartViewPageImpl<CRecentFileDlg>, COpenAsChoicesHelper,
	public CContextMenuWithIcons<CRecentFileDlg>,
	public IThumbnailCallback
{
public:
	CRecentFileDlg() : m_bDestroyed(false), m_bIgnoreNext(false)
	{
	}
	~CRecentFileDlg()
	{
		m_cThumbnails.Destroy();
		m_cCtxMenu.Destroy();
	}

	void WindowCreate(RWHWND a_hParent, RECT const* a_prc, LCID a_tLocaleID, IStartViewCallback* a_pCallback, IConfig* a_pAppConfig)
	{
		m_pSubmenuTemplate.Attach(new CMultiLanguageString(L"[0409]Open as \"%s\"[0405]Otevřít jako \"%s\""));
		m_pClbk = a_pCallback;
		m_tLocaleID = a_tLocaleID;
		m_pCfg = a_pAppConfig;
		Create(a_hParent);
		MoveWindow(a_prc);
	}
	virtual void OnFinalMessage(HWND a_hWnd);
	IConfig* M_AppConfig() const { return m_pCfg; }

	enum { /*IDC_LIST = 1017,*/ WM_THUMBNAILREADY = WM_APP+825, ID_REFRESHTHUMBNAIL = 404, ID_REMOVERECENTFILE, ID_OPENFOLDER, ID_OPENFILE, ID_OPENFILEAS0 };

	BEGIN_DIALOG_EX(0, 0, 186, 151, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_LIST, WC_LISTVIEW, LVS_ICON | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_AUTOARRANGE | LVS_NOSORTHEADER | WS_TABSTOP | WS_VISIBLE, 0, 0, 186, 151, 0)
	END_CONTROLS_MAP()

	BEGIN_COM_MAP(CRecentFileDlg)
		COM_INTERFACE_ENTRY(IStartViewPage)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IThumbnailCallback)
	END_COM_MAP()

	BEGIN_MSG_MAP(CRecentFileDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMACTIVATE, OnItemActivateList)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnItemChangedList)
		NOTIFY_HANDLER(IDC_LIST, LVN_GETINFOTIP, OnGetInfoTipList)
		NOTIFY_HANDLER(IDC_LIST, LVN_GETEMPTYMARKUP, OnGetEmptyMarkup)
		NOTIFY_HANDLER(IDC_LIST, NM_CLICK, OnItemClicked)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CRecentFileDlg>)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_THUMBNAILREADY, OnThumbnailReady)
	END_MSG_MAP()

	// handlers
public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnItemActivateList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnItemClicked(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnItemChangedList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetInfoTipList(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnGetEmptyMarkup(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnThumbnailReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	STDMETHOD(Destroy)()
	{
		m_bDestroyed = true;
		{
			ObjectLock cLock(this);
			if (m_wndList.m_hWnd)
			{
				int nCount = m_wndList.GetItemCount();
				for (int i = 0; i < nCount; ++i)
				{
					IStorageFilter* pLoc = reinterpret_cast<IStorageFilter*>(m_wndList.GetItemData(i));
					if (pLoc)
						pLoc->Release();
				}
				m_wndList.DeleteAllItems();
			}
			if (m_pThumbnails)
			{
				m_pThumbnails->CancelRequests(this);
				m_pThumbnails = NULL;
			}
			m_pCache = NULL;
			m_pClbk = NULL;
		}
		return CStartViewPageImpl<CRecentFileDlg>::Destroy();
	}

	// IStartViewWnd methods
public:
	STDMETHOD(Activate)();
	STDMETHOD(Deactivate)();
	STDMETHOD(ClickedDefault)();
	HRESULT ClickedChoice(IDocumentBuilder* a_pBuilder);

	// IThumbnailCallback methods
public:
	STDMETHOD(AdjustSize)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		*a_pSizeX = m_szThumbnail.cx;
		*a_pSizeY = m_szThumbnail.cy;
		return S_OK;
	}
	STDMETHOD(SetThumbnail)(IStorageFilter* a_pFile, ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData, RECT const* a_prcBounds, BSTR a_bstrInfo);

	static HICON IconFromThumbnail(ULONG a_nSizeX, ULONG a_nSizeY, DWORD const* a_pRGBAData);
	int IndexFromPath(IStorageFilter* a_pFilter);

private:
	CComPtr<IStartViewCallback> m_pClbk; 
	CComPtr<IConfig> m_pCfg;
	CListViewCtrl m_wndList;
	int m_iActiveItem;
	CImageList m_cThumbnails;
	SIZE m_szThumbnail;
	CComPtr<IAsyncThumbnailRenderer> m_pThumbnails;
	CComPtr<IThumbnailCache> m_pCache;
	CComPtr<ILocalizedString> m_pSubmenuTemplate;
	CImageList m_cCtxMenu;
	bool m_bDestroyed;
	bool m_bIgnoreNext;
};


class CStartViewPageFactoryRecentFiles : public CStartViewPageFactory<CRecentFileDlg, &CLSID_StartViewPageFactoryRecentFiles, IDS_STARTPAGERECENTFILES_NAME, IDS_STARTPAGERECENTFILES_DESC, 0, RecentFiles::InitConfig>
{
public:
	DECLARE_CLASSFACTORY_SINGLETON(CStartViewPageFactoryRecentFiles)

	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPolyPoint const stand1[] =
			{
				{28, 256}, {28, 232}, {36, 224}, {220, 224}, {228, 232}, {228, 256},
			};
			static IRPolyPoint const stand2[] =
			{
				{228, 0}, {228, 24}, {220, 32}, {36, 32}, {28, 24}, {28, 0},
			};
			static IRPathPoint const glass[] =
			{
				{149, 128, 0, -27, 0, 27},
				{210, 17, 0, 0, 8, 85},
				{46, 17, -8, 85, 0, 0},
				{107, 128, 0, 27, 0, -27},
				{46, 239, 0, 0, -8, -85},
				{210, 239, 8, -85, 0, 0},
			};
			static IRPathPoint const sand1[] =
			{
				{128, 165, 0, 0, 0, 0},
				{205, 235, 0, 0, -17, -30},
				{51, 235, 17, -30, 0, 0},
			};
			static IRPathPoint const sand2[] =
			{
				{128, 125, 4, -20, -4, -20},
				{165, 84, -15, 0, -7, 8},
				{128, 87, -9, 0, 9, 0},
				{91, 84, 7, 8, 15, 0},
			};
			IRFill matStandFill(0xffa38863);
			IRFill matSandFill(0xffc8bc69);
			IRFill matGlassFill(0x3ca2dfed);
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			IROutlinedFill matStand(&matStandFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IROutlinedFill matSand(&matSandFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IROutlinedFill matGlass(&matGlassFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
			IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			IRGridItem gridX[] = {{0, 28}, {0, 228}};
			IRGridItem gridY[] = {{0, 0}, {0, 32}, {0, 224}, {0, 256}};
			IRCanvas canvasStand = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
			IRTarget target(0.92f);

			cRenderer(&canvas, itemsof(sand1), sand1, &matSand, target);
			cRenderer(&canvas, itemsof(sand2), sand2, &matSand, target);
			cRenderer(&canvas, itemsof(glass), glass, &matGlass, target);
			cRenderer(&canvasStand, itemsof(stand1), stand1, &matStand, target);
			cRenderer(&canvasStand, itemsof(stand2), stand2, &matStand, target);

			*a_phIcon = cRenderer.get();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};
OBJECT_ENTRY_AUTO(__uuidof(StartViewPageFactoryRecentFiles), CStartViewPageFactoryRecentFiles)
