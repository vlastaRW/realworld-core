
#pragma once

extern __declspec(selectany) GUID const MenuCommandsDesignerToolsID = {0xc2cd734e, 0xf418, 0x47db, {0x8a, 0x45, 0x13, 0x71, 0xf5, 0x65, 0xe6, 0x46}};


class ATL_NO_VTABLE CDocumentMenuCommandDesignerTool : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	void Init(IDesignerFrameTools* a_pTools, ULONG a_nIndex, IDocument* a_pDocument)
	{
		m_pTools = a_pTools;
		m_nIndex = a_nIndex;
		CoCreateGuid(&m_tIconID);
		m_pDocument = a_pDocument;
	}

BEGIN_COM_MAP(CDocumentMenuCommandDesignerTool)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()


	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		return m_pTools->Name(m_nIndex, a_ppText);
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		return m_pTools->HelpText(m_nIndex, a_ppText);
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			*a_pIconID = m_tIconID;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		return m_pTools->Icon(m_nIndex, a_nSize, a_phIcon);
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = EMCSNormal;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pTools->Activate(m_nIndex, a_hParent, a_tLocaleID, m_pDocument);
	}

private:
	CComPtr<IDesignerFrameTools> m_pTools;
	ULONG m_nIndex;
	GUID m_tIconID;
	CComPtr<IDocument> m_pDocument;
};

void EnumDesignerTools(IOperationContext* UNREF(a_pStates), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{

	// insert external tools menu
	CComPtr<IEnumUnknowns> pTools;
	a_pFrame->GetThread()->M_PlugInCache()->InterfacesEnum(CATID_DesignerFrameTools, __uuidof(IDesignerFrameTools), 0xffffffff, &pTools, NULL);
	CComPtr<IDesignerFrameTools> pToolsItem;
	for (ULONG iToolSet = 0; SUCCEEDED(pTools->Get(iToolSet, __uuidof(IDesignerFrameTools), reinterpret_cast<void**>(&pToolsItem))); iToolSet++, pToolsItem=NULL)
	{
		ULONG nTools = 0;
		pToolsItem->Size(&nTools);
		ULONG i;
		for (i = 0; i < nTools; i++)
		{
			CComObject<CDocumentMenuCommandDesignerTool>* p = NULL;
			CComObject<CDocumentMenuCommandDesignerTool>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(pToolsItem, i, a_pDocument);
			a_pCommands->Insert(pTmp);
		}
	}
}


// Tools->Manage Layouts

extern __declspec(selectany) GUID const MenuCommandsManageLayoutsID = {0x8ed5b4d, 0xe7aa, 0x4bb1, {0xbf, 0x54, 0xb3, 0xe4, 0x27, 0x94, 0x7a, 0xed}};
extern __declspec(selectany) GUID const MenuCommandManageLayoutsID = {0xc9abbdc4, 0x2f, 0x4c01, {0xa7, 0x79, 0xa, 0xcf, 0x6b, 0xed, 0xe2, 0x9f}};

HICON GetIconLayouts(ULONG size);

class ATL_NO_VTABLE CDocumentMenuCommandLayoutMng : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandLayoutMng, IDS_MN_TOOLS_LAYOUTMNG, IDS_MD_TOOLS_LAYOUTMNG, &MenuCommandManageLayoutsID, 0>
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
			*a_phIcon = GetIconLayouts(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		BOOL b;
		m_pFrame->OnProfileManage(0, 0, 0, b);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

#include "ConfigIDsApp.h"

void EnumManageLayouts(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CConfigValue cVal;
	a_pFrame->GetThread()->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_LAYOUTCOMMANDS), &cVal);
	if (cVal.operator bool())
	{
		CComObject<CDocumentMenuCommandLayoutMng>* p = NULL;
		CComObject<CDocumentMenuCommandLayoutMng>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		p->Init(a_pFrame);
		a_pCommands->Insert(pTmp);
	}
}


// Tools->Options

extern __declspec(selectany) GUID const MenuCommandsOptionsID = {0x4a286052, 0xfc7a, 0x4c5a, {0x86, 0xa2, 0x18, 0xb5, 0xab, 0x79, 0x7, 0xd9}};

class ATL_NO_VTABLE CDocumentMenuCommandOptions : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandOptions, IDS_MN_TOOLS_OPTIONS, IDS_MD_TOOLS_OPTIONS, &MenuCommandsOptionsID, 0>
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
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			pSI->GetLayers(ESISettings, cRenderer);
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		BOOL b;
		m_pFrame->OnToolsOptions(0, 0, 0, b);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumOptions(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandOptions>* p = NULL;
	CComObject<CDocumentMenuCommandOptions>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}

