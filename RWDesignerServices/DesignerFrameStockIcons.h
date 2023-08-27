// DesignerFrameStockIcons.h : Declaration of the CDesignerFrameStockIcons

#pragma once
#include "resource.h"       // main symbols
#include <RWConceptDesignerExtension.h>


// 5C949D74-C1E9-4ECD-8544-3931FAB8F135
extern __declspec(selectany) CLSID const CLSID_DesignerFrameStockIcons = { 0x5c949d74, 0xc1e9, 0x4ecd, { 0x85, 0x44, 0x39, 0x31, 0xfa, 0xb8, 0xf1, 0x35 } };

//// 90669FCC-9D37-408C-BA02-3CBA7BA0FDB7
//extern __declspec(selectany) CLSID const CLSID_DesignerFrameUserIcons = { 0x90669fcc, 0x9d37, 0x408c, { 0xba, 0x02, 0x3c, 0xba, 0x7b, 0xa0, 0xfd, 0xb7 } };

// CDesignerFrameStockIcons

class ATL_NO_VTABLE CDesignerFrameStockIcons : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerFrameStockIcons, &CLSID_DesignerFrameStockIcons>,
	public IDesignerFrameIcons
{
public:
	CDesignerFrameStockIcons()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerFrameStockIcons)

BEGIN_CATEGORY_MAP(CDesignerFrameStockIcons)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerFrameStockIcons)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDesignerFrameIcons methods
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp);
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs);
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon);
};

OBJECT_ENTRY_AUTO(CLSID_DesignerFrameStockIcons, CDesignerFrameStockIcons)
