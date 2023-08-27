// DesignerViewFactoryToolbar.cpp : Implementation of CDesignerViewFactoryToolbar

#include "stdafx.h"
#include "DesignerViewFactoryToolbar.h"
#include "DesignerViewToolbar.h"
#include <SharedStringTable.h>
#include "ConfigIDsToolbar.h"
#include "ConfigGUIToolbar.h"
#include "ConfigGUIItem.h"


// CDesignerViewFactoryToolbar

STDMETHODIMP CDesignerViewFactoryToolbar::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_VIEWTOOLBAR_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

extern const GUID TItemConfigID = {0xdabf310a, 0xc4c2, 0x4fd5, {0xba, 0x40, 0xea, 0xc8, 0x71, 0xe2, 0x14, 0x40}};

STDMETHODIMP CDesignerViewFactoryToolbar::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pConfigPattern;
		RWCoCreateInstance(pConfigPattern, __uuidof(ConfigWithDependencies));
		CComPtr<ISubConfigVector> pConfigVector;
		RWCoCreateInstance(pConfigVector, __uuidof(SubConfigVector));

		// insert values to the pattern
		CComPtr<IMenuCommandsManager> pMenuCmds;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&pMenuCmds));
		if (pMenuCmds == NULL)
		{
			RWCoCreateInstance(pMenuCmds, __uuidof(MenuCommandsManager));
		}
		pMenuCmds->InsertIntoConfigAs(pMenuCmds, pConfigPattern, CComBSTR(CFGID_TOOLBAR_COMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_COMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_COMMANDS_DESC), 0, NULL);

		// name
		CComBSTR cCFGID_TOOLBAR_NAME(CFGID_TOOLBAR_NAME);
		pConfigPattern->ItemInsSimple(cCFGID_TOOLBAR_NAME, _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_NAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_NAME_DESC), CConfigValue(L""), NULL, 0, NULL);

		// ID
		CComBSTR cCFGID_TOOLBAR_ID(CFGID_TOOLBAR_ID);
		pConfigPattern->ItemInsSimple(cCFGID_TOOLBAR_ID, _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_ID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_ID_DESC), CConfigValue(L""), NULL, 0, NULL);

		// hidden flag
		pConfigPattern->ItemInsSimple(CComBSTR(CFGID_TOOLBAR_HIDDEN), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_HIDDEN_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_HIDDEN_DESC), CConfigValue(false), NULL, 0, NULL);

		// finalize pattern
		CConfigCustomGUI<&TItemConfigID, CConfigGUIItemDlg>::FinalizeConfig(pConfigPattern);

		// insert pattern to the vector
		pConfigVector->Init(FALSE, pConfigPattern);

		// insert values and vector to the config
		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_TABS_ACTIVEINDEX), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_ACTIVEINDEX_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TABS_ACTIVEINDEX_DESC), CConfigValue(0L), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TOOLBAR_ITEMS), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_ITEMS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_ITEMS_DESC), CConfigValue(1L), pConfigVector.p, 0, NULL);

		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_TOOLBAR_VIEW), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_VIEW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLBAR_VIEW_DESC), 0, NULL);
		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryToolbar, CConfigGUIToolbarDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryToolbar::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewToolbar>* pWnd = NULL;
		CComObject<CDesignerViewToolbar>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pTmp = pWnd;
		pWnd->Init(a_pManager, a_pDoc, a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID);

		*a_ppDVWnd = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryToolbar::CheckSuitability(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	try
	{
		CConfigValue cViewID;
		CComBSTR cCFGID_TOOLBAR_VIEW(CFGID_TOOLBAR_VIEW);
		a_pConfig->ItemValueGet(cCFGID_TOOLBAR_VIEW, &cViewID);
		CComPtr<IConfig> pViewCfg;
		a_pConfig->SubConfigGet(cCFGID_TOOLBAR_VIEW, &pViewCfg);
		return a_pManager->CheckSuitability(a_pManager, cViewID, pViewCfg, a_pDocument, a_pCallback);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

typedef HICON (*pfnGetIcon)(ULONG size);

HICON GetIconNewDoc(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.9f, -1, 1));
	pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.65f, 1, -1));
	return cRenderer.get();
}

HICON GetIconOpenFile(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIFolder, cRenderer);
	return cRenderer.get();
}

HICON GetIconSaveFile(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIFloppy, cRenderer, IRTarget(0.92f));
	return cRenderer.get();
}

HICON GetIconEditCut(ULONG size)
{
	static IRPolyPoint const poly[] = {{0, 160}, {0, 256}, {128, 256}, {176, 208}, {176, 99}, {227, 99}, {128, 0}, {29, 99}, {80, 99}, {80, 160}};

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));

	static IRTarget target(0.8f);
	static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
	static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 99.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(poly), poly, pSI->GetMaterial(ESMDelete), &target);
	return cRenderer.get();
}

HICON GetIconEditCopy(ULONG size)
{
	static IRPolyPoint const poly[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}, {176, 160}, {176, 99}, {227, 99}, {128, 0}, {29, 99}, {80, 99}, {80, 160}};

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));

	static IRTarget target(0.8f);
	static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
	static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 99.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(poly), poly, pSI->GetMaterial(ESMManipulate), &target);
	return cRenderer.get();
}

HICON GetIconEditPaste(ULONG size)
{
	static IRPolyPoint const poly0[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}};
	static IRPolyPoint const poly1[] = {{80, 0}, {80, 114}, {29, 114}, {128, 213}, {227, 114}, {176, 114}, {176, 0}};

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));

	static IRTarget target(0.8f);
	static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
	static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 114.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};

	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(poly0), poly0, pSI->GetMaterial(ESMManipulate), &target);
	cRenderer(&canvas, itemsof(poly1), poly1, pSI->GetMaterial(ESMManipulate), &target);
	return cRenderer.get();
}

static IRPolyPoint const g_aTriangleOut[] =
{
	{256, 256}, {256, 0}, {0, 256},
};
static IRPolyPoint const g_aTriangleIn[] =
{
	{200, 130}, {200, 200}, {130, 200},
	//{208, 116}, {208, 208}, {116, 208},
};
static IRPolygon const g_aTriangle[] =
{
	{itemsof(g_aTriangleOut), g_aTriangleOut},
	{itemsof(g_aTriangleIn), g_aTriangleIn},
};
static IRGridItem const g_tTriGrid[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 200.0f}, {EGIFInteger, 256.0f}};
//static IRGridItem const g_tTriGrid[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 208.0f}, {EGIFInteger, 256.0f}};
static IRCanvas const g_tTriCanvas = {0, 0, 256, 256, itemsof(g_tTriGrid), itemsof(g_tTriGrid), g_tTriGrid, g_tTriGrid};

HICON GetIconLayout(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.9f, -0.5f, 1));
	cRenderer(&g_tTriCanvas, itemsof(g_aTriangle), g_aTriangle, pSI->GetMaterial(ESMConfirm), IRTarget(0.9f, 0.5f, -1));
	return cRenderer.get();
}

HICON GetIconLayouts(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.85f, -1, -1));
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.85f, 0, 1));
	cRenderer(&g_tTriCanvas, itemsof(g_aTriangle), g_aTriangle, pSI->GetMaterial(ESMConfirm), IRTarget(0.8f, 1, -0.5f));
	return cRenderer.get();
}

HICON GetIconLayoutEdit(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.72f, -1, 1));
	cRenderer(&g_tTriCanvas, itemsof(g_aTriangle), g_aTriangle, pSI->GetMaterial(ESMConfirm), IRTarget(0.72f, -0.9f, 0.2f));
	pSI->GetLayers(ESIModify, cRenderer, IRTarget(0.75f, 1, -1));
	return cRenderer.get();
}

HICON GetIconLayoutNew(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.72f, -1, 1));
	cRenderer(&g_tTriCanvas, itemsof(g_aTriangle), g_aTriangle, pSI->GetMaterial(ESMConfirm), IRTarget(0.72f, -0.9f, 0.2f));
	pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.65f, 1, -1));
	return cRenderer.get();
}

HICON GetIconLayoutDelete(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.72f, -1, 1));
	cRenderer(&g_tTriCanvas, itemsof(g_aTriangle), g_aTriangle, pSI->GetMaterial(ESMConfirm), IRTarget(0.72f, -0.9f, 0.2f));
	pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.65f, 1, -1));
	return cRenderer.get();
}

HICON GetIconLayoutDuplicate(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIDocument, cRenderer, IRTarget(0.72f, -1, 1));
	cRenderer(&g_tTriCanvas, itemsof(g_aTriangle), g_aTriangle, pSI->GetMaterial(ESMConfirm), IRTarget(0.72f, -0.9f, 0.2f));
	pSI->GetLayers(ESIDuplicate, cRenderer, IRTarget(0.65f, 1, -1));
	return cRenderer.get();
}

HICON GetIconHelpLightbulb(ULONG size)
{
	static IRPathPoint const bulb[] =
	{
		{83.9393, 183.323, 14.0477, 11.2543, -14.0477, -11.2543},
		{119.911, 193.716, 22.9788, -30.2816, -0.00447845, 6.40312},
		{215.851, 136.257, 24.8625, -31.0334, -22.2027, 27.7135},
		{203.662, 34.0195, -31.8955, -25.5531, 31.8955, 25.5531},
		{100.497, 43.356, -22.2027, 27.7135, 24.8625, -31.0334},
		{65.8483, 150.297, -6.24792, 1.40118, 24.5396, -29.0311},
	};
	static IRPathPoint const hilight1[] =
	{
		{163.924, 32.5396, -24.6832, -5.57434, 30.6896, 6.93082},
		{106.221, 64.912, -12.3826, 30.0477, 11.008, -26.712},
		{145.86, 66.8401, 19.4844, -11.3188, -18.9865, 11.0296},
	};
	static IRPathPoint const hilight2[] =
	{
		{151.513, 40.5697, -13.7103, -3.4211, 17.0466, 4.25363},
		{121.42, 56.4601, -7.28915, 16.6198, 6.47997, -14.7748},
		{140.803, 58.4538, 11.0194, -6.075, -10.7379, 5.9198},
	};
	static IRPathPoint const contact[] =
	{
		{43.3979, 233.297, 14.0477, 11.2543, -14.0477, -11.2543},
		{71.8335, 236.675, 15.8455, 5.47034, -14.0523, -4.85126},
		{60.6369, 211.924, -6.24345, -5.00195, 6.24344, 5.00192},
		{33.7943, 206.977, 7.79982, 12.6555, -8.79512, -14.2705},
	};
	static IRPathPoint const screw1[] =
	{
		{76.2829, 192.12, 14.0477, 11.2543, -14.0478, -11.2543},
		{113.443, 212.641, 20.7793, -11.5423, -9.29271, 5.16187},
		{91.2868, 173.576, -18.7303, -15.0059, 18.7303, 15.0059},
		{48.0632, 160.282, -3.01042, 10.195, 6.73149, -22.7967},
	};
	static IRPathPoint const screw2[] =
	{
		{64.4651, 207.526, 11.7065, 9.37863, -11.7065, -9.37862},
		{99.7472, 225.503, 20.7792, -11.5423, -9.29271, 5.16183},
		{79.4691, 188.982, -16.3891, -13.1301, 16.389, 13.1301},
		{38.7657, 176.972, -3.01041, 10.195, 6.73151, -22.7967},
	};
	static IRPathPoint const screw3[] =
	{
		{52.6474, 222.932, 10.1456, 8.12814, -10.1456, -8.12814},
		{85.1003, 238.673, 19.3735, -11.3871, -9.16443, 5.38647},
		{67.3423, 203.436, -14.8281, -11.8796, 14.8281, 11.8796},
		{29.4681, 193.662, -3.25763, 10.1187, 6.88669, -21.3909},
	};
	static IRCanvas const canvas = {16, 16, 240, 240, 0, 0, NULL, NULL};
	static IRFill const hilight(0x3fffffff);

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(bulb), bulb, pSI->GetMaterial(ESMInterior));//ESMBrightLight));
	cRenderer(&canvas, itemsof(hilight1), hilight1, &hilight);
	cRenderer(&canvas, itemsof(hilight2), hilight2, &hilight);
	cRenderer(&canvas, itemsof(contact), contact, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(screw1), screw1, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(screw2), screw2, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(screw3), screw3, pSI->GetMaterial(ESMAltBackground));
	return cRenderer.get();
}

HICON GetIconHelpContext(ULONG size)
{
	static IRPathPoint const question1[] =
	{
		{159, 185, -2.10548, -8.9325, 0, 0},
		{156.624, 157.03, 1.62025, -9.5865, -1.29847, 7.68259},
		{169, 130, 6.53324, -7.89714, -6.16455, 7.45148},
		{198, 101, 8.33996, -10.9913, -8.29536, 10.9325},
		{202, 72, -2.99069, -7.91267, 2.69199, 7.12236},
		{179, 58, -17.7877, -0.0808252, 13, 0.0590706},
		{127, 79, 0, 0, 14.2321, -13.6498},
		{127, 31, 13.0169, -9.2616, 0, 0},
		{184, 16, 30.9536, -0.0675106, -16.709, 0.0364427},
		{250, 48, 10.3207, 20.3207, -11.018, -21.6937},
		{235, 121, -9.26787, 8.59005, 21, -19.4641},
		{203.882, 151.359, -6.15696, 8.89337, 6.07596, -8.77637},
		{202, 185, 0, 0, -7, -10},
	};
	static IRPathPoint const question2[] =
	{
		{183, 255, -16.5685, 0, 16.5685, 0},
		{153, 228, 0, -14.3594, 0, 14.9117},
		{183, 202, 16.5685, 0, -16.5685, 0},
		{213, 228, 0, 14.9117, 0, -14.3594},
	};
	static IRPolyPoint const arrow[] =
	{
		{0, 0}, {0, 192}, {40, 152}, {65, 212}, {105, 196}, {80, 136}, {136, 136},
	};
	static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvas, itemsof(question1), question1, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(question2), question2, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(arrow), arrow, pSI->GetMaterial(ESMInterior));
	return cRenderer.get();
}

HICON GetIconRename(ULONG size)
{
	static IRPolyPoint const box[] =
	{
		{0, 80}, {256, 80}, {256, 176}, {0, 176},
	};
	static IRPolyPoint const cursor[] =
	{
		{152, 48}, {168, 48}, {168, 208}, {152, 208}, {152, 232}, {208, 232}, {208, 208}, {192, 208}, {192, 48}, {208, 48}, {208, 24}, {152, 24},
	};
	static IRGridItem const boxGridX[] = {{0, 0}, {0, 256}};
	static IRGridItem const boxGridY[] = {{0, 80}, {0, 176}};
	static IRCanvas const canvasBox = {0, 0, 256, 256, itemsof(boxGridX), itemsof(boxGridY), boxGridX, boxGridY};
	static IRGridItem const cursorGridX[] = {{0, 152}, {0, 168}, {0, 192}, {0, 208}};
	static IRGridItem const cursorGridY[] = {{0, 24}, {0, 48}, {0, 208}, {0, 232}};
	static IRCanvas const canvasCursor = {0, 0, 256, 256, itemsof(cursorGridX), itemsof(cursorGridY), cursorGridX, cursorGridY};

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	cRenderer(&canvasBox, itemsof(box), box, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvasCursor, itemsof(cursor), cursor, pSI->GetMaterial(ESMInterior));
	return cRenderer.get();
}

HICON GetIconMoveDown(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIMoveDown, cRenderer);
	return cRenderer.get();
}

HICON GetIconMoveUp(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIMoveUp, cRenderer);
	return cRenderer.get();
}


extern __declspec(selectany) pfnGetIcon s_aCommonIconProcs[] =
{
	GetIconEditCut,
	GetIconEditCopy,
	GetIconEditPaste,
	GetIconNewDoc,
	GetIconOpenFile,
	GetIconSaveFile,
	GetIconLayout,
	GetIconLayouts,
	GetIconLayoutEdit,
	GetIconHelpLightbulb,
};

STDMETHODIMP CDesignerViewFactoryToolbar::GetIcon(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;

		for (size_t i = 0; i < itemsof(s_aCommonIconUIDs); ++i)
		{
			if (IsEqualGUID(a_tIconID, s_aCommonIconUIDs[i]))
			{
				*a_phIcon = s_aCommonIconProcs[i](a_nSize);//(HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_aResIDs[i]), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
				break;
			}
		}
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}
