
#pragma once

#include <ConfigCustomGUIList.h>

extern __declspec(selectany) GUID const TMenuCommandsStorageCtx = {0xcdfc8178, 0x581d, 0x4371, {0xae, 0x61, 0x80, 0x7c, 0x76, 0xeb, 0xf5, 0x5c}};


class ATL_NO_VTABLE CConfigGUIVectorDlg :
	public CCustomConfigWndImpl<CConfigGUIVectorDlg>,
	public CCustomConfigGUIList<CConfigGUIVectorDlg, IDC_CG_LIST, IDC_CG_TOOLBAR, IDC_CG_ITEM, IDS_LISTBUTTONNAMES, 0, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON>,
	public CDialogResize<CConfigGUIVectorDlg>
{
public:
	CConfigGUIVectorDlg() :
		CCustomConfigGUIList<CConfigGUIVectorDlg, IDC_CG_LIST, IDC_CG_TOOLBAR, IDC_CG_ITEM, IDS_LISTBUTTONNAMES, 0, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON>(CFGID_VECTOR_ITEMS)
	{
	}
	enum
	{
		IDD = IDD_CONFIGGUI_VECTOR,
		ID_IMPORT = 3000,
		ID_EXPORT,
	};

	BEGIN_MSG_MAP(CConfigGUIVectorDlg)
		COMMAND_ID_HANDLER(ID_IMPORT, OnImport)
		COMMAND_ID_HANDLER(ID_EXPORT, OnExport)
		NOTIFY_HANDLER(IDC_CG_LIST, LVN_ITEMCHANGED, OnListItemChanged)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIVectorDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIVectorDlg>)
		CHAIN_MSG_MAP(ListClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIVectorDlg)
		DLGRESIZE_CONTROL(IDC_CG_LIST, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CG_ITEM, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIVectorDlg)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		static TBBUTTON aButtons[] =
		{
			{ 0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{ 5, ID_IMPORT, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_NOPREFIX, TBBUTTON_PADDING, 0, 5},
			{ 6, ID_EXPORT, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_NOPREFIX, TBBUTTON_PADDING, 0, 6},
		};
		CToolBarCtrl wndTB(GetDlgItem(IDC_CG_TOOLBAR));
		CImageList cIL(wndTB.GetImageList());
		SIZE sz;
		cIL.GetIconSize(sz);
		int nIconSize = XPGUI::GetSmallIconSize();
		RECT padding = {(sz.cx-nIconSize)>>1, (sz.cy-nIconSize)>>1, (sz.cx-nIconSize)-((sz.cx-nIconSize)>>1), (sz.cy-nIconSize)-((sz.cy-nIconSize)>>1)};
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.875f));
			HICON h = cRenderer.get(padding);
			cIL.AddIcon(h);
			DestroyIcon(h);
		}
		{
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIFloppySimple, cRenderer, IRTarget(0.875f));
			HICON h = cRenderer.get(padding);
			cIL.AddIcon(h);
			DestroyIcon(h);
		}
		wndTB.AddButtons(itemsof(aButtons), aButtons);
		wndTB.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

		DlgResize_Init(false, false, 0);

		return 1;
	}
	void ExtraConfigNotify()
	{
		UpdateList();
	}

	LRESULT OnListItemChanged(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		int nSel = CListViewCtrl(GetDlgItem(IDC_CG_LIST)).GetSelectedIndex();
		CToolBarCtrl(GetDlgItem(IDC_CG_TOOLBAR)).EnableButton(ID_EXPORT, nSel != -1);
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnImport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(L"rwcommands");
		CComBSTR bstrFilter(L"*.");
		bstrFilter += bstrExt;
		pDocType->InitEx(_SharedStringTable.GetStringAuto(IDS_RWCOMMANDS_FILTER), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
		CComPtr<IEnumUnknownsInit> pDocTypes;
		RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
		pDocTypes->Insert(pDocType);
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pStorage;
		pStMgr->FilterCreateInteractivelyUID(NULL, EFTOpenExisting, m_hWnd, pDocTypes, NULL, TMenuCommandsStorageCtx, _SharedStringTable.GetStringAuto(IDS_RWCOMMANDS_OPEN), NULL, m_tLocaleID, &pStorage);
		if (pStorage == NULL)
			return 0;

		CComPtr<IDataSrcDirect> pSrc;
		pStorage->SrcOpen(&pSrc);
		ULONG nSize = 0;
		if (pSrc == NULL || FAILED(pSrc->SizeGet(&nSize)) || nSize == 0)
		{
			// TODO: message
			return 0;
		}
		CDirectInputLock cData(pSrc, nSize);

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		pMemCfg->DataBlockSet(nSize, cData.begin());

		CConfigValue cVal;
		CComBSTR bstrRootCfgID(CFGID_VECTOR_ITEMS);
		if (SUCCEEDED(M_Config()->ItemValueGet(bstrRootCfgID, &cVal)))
		{
			cVal = cVal.operator LONG()+1;
			M_Config()->ItemValuesSet(1, &(bstrRootCfgID.m_str), cVal);
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VECTOR_ITEMS, cVal.operator LONG()-1);
			CComBSTR bstrProfileName(szNameID);
			CComPtr<IConfig> pDstCfg;
			M_Config()->SubConfigGet(bstrProfileName, &pDstCfg);
			CopyConfigValues(pDstCfg, pMemCfg);
			//FillList();
			CListViewCtrl(GetDlgItem(IDC_CG_LIST)).SetItemState(cVal.operator LONG()-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}

		return 0;
	}
	LRESULT OnExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = CListViewCtrl(GetDlgItem(IDC_CG_LIST)).GetSelectedIndex();
		if (nSel == -1)
			return 0;

		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VECTOR_ITEMS, nSel);
		CComBSTR bstrNameID(szNameID);
		CComPtr<IConfig> pSrcCfg;
		M_Config()->SubConfigGet(bstrNameID, &pSrcCfg);
		if (pSrcCfg == NULL)
			return 0;

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		CopyConfigValues(pMemCfg, pSrcCfg);
		ULONG nSize = 0;
		pMemCfg->DataBlockGetSize(&nSize);
		if (nSize == 0)
			return 0;

		CAutoVectorPtr<BYTE> pMem(new BYTE[nSize]);

		pMemCfg->DataBlockGet(nSize, pMem.m_p);

		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(L"rwcommands");
		CComBSTR bstrFilter(L"*.");
		bstrFilter += bstrExt;
		pDocType->InitEx(_SharedStringTable.GetStringAuto(IDS_RWCOMMANDS_FILTER), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
		CComPtr<IEnumUnknownsInit> pDocTypes;
		RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
		pDocTypes->Insert(pDocType);
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pStorage;
		pStMgr->FilterCreateInteractivelyUID(NULL, EFTCreateNew, m_hWnd, pDocTypes, NULL, TMenuCommandsStorageCtx, _SharedStringTable.GetStringAuto(IDS_RWCOMMANDS_SAVE), NULL, m_tLocaleID, &pStorage);
		if (pStorage == NULL)
			return 0;
		CComPtr<IDataDstStream> pDst;
		pStorage->DstOpen(&pDst);
		if (pDst == NULL || FAILED(pDst->Write(nSize, pMem.m_p)) || FAILED(pDst->Close()))
		{
			// TODO: message
		}

		return 0;
	}
};
