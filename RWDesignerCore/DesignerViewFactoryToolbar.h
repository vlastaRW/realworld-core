// DesignerViewFactoryToolbar.h : Declaration of the CDesignerViewFactoryToolbar

#pragma once
#include "resource.h"       // main symbols
#include "RWDesignerCore.h"
#include <DesignerFrameIconsImpl.h>


extern __declspec(selectany) GUID s_aCommonIconUIDs[] =
{
	{0xc39074fc, 0xa980, 0x4e4a, {0xa1, 0xc3, 0x78, 0x06, 0xc6, 0x16, 0x38, 0x74}},
	{0x4145fbfc, 0x7759, 0x4c97, {0xa0, 0x0b, 0x8b, 0xd5, 0xda, 0xce, 0xc5, 0xdb}},
	{0x980c077b, 0x7d21, 0x4200, {0xa8, 0xbb, 0x9b, 0x4f, 0xb4, 0x7f, 0x60, 0x57}},
	{0x0292a5c5, 0x6afa, 0x4fc0, {0xbd, 0x6e, 0x27, 0xbf, 0xaa, 0x04, 0x52, 0xe5}},
	{0x7bff39a4, 0x668d, 0x46d0, {0xa1, 0x88, 0xd3, 0x6f, 0x32, 0x5c, 0xd9, 0xf7}},
	{0x21fa301a, 0xefe1, 0x414b, {0x81, 0xe7, 0x7a, 0x60, 0x45, 0xb8, 0xd7, 0xbd}},
	{0xd73e7509, 0x363a, 0x4abe, {0xa1, 0xa1, 0x0e, 0x0b, 0xa0, 0x59, 0xdf, 0xe7}},
	{0xc9abbdc4, 0x002f, 0x4c01, {0xa7, 0x79, 0x0a, 0xcf, 0x6b, 0xed, 0xe2, 0x9f}},
	{0x389a7456, 0xf0a4, 0x4fbd, {0x96, 0x52, 0x63, 0x7a, 0xf0, 0xc3, 0x6e, 0x9f}},
	{0xa31e03a7, 0x2ea4, 0x4d0b, {0x8b, 0xf1, 0xdb, 0x6b, 0x1f, 0x92, 0x9c, 0x75}},
};


// CDesignerViewFactoryToolbar

class ATL_NO_VTABLE CDesignerViewFactoryToolbar :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryToolbar, &CLSID_DesignerViewFactoryToolbar>,
	public IDesignerViewFactory,
	public CDesignerFrameIconsImpl<sizeof(s_aCommonIconUIDs)/sizeof(s_aCommonIconUIDs[0]), s_aCommonIconUIDs, NULL>
{
public:
	CDesignerViewFactoryToolbar()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryToolbar)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryToolbar)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryToolbar)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

	// IDesignerFrameIcons methods
public:
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon);
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryToolbar), CDesignerViewFactoryToolbar)
