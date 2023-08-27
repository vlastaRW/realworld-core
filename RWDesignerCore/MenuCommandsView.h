
#pragma once

#include "MainFrame.h"
#include "ConfigIDsApp.h"
#include <MultiLanguageString.h>


extern __declspec(selectany) GUID const MenuCommandsStatusBarID = {0x27d1c04b, 0x4418, 0x4296, {0xa2, 0x1d, 0x80, 0x20, 0x26, 0x92, 0xb6, 0xb9}};

// View->Status Bar

class ATL_NO_VTABLE CDocumentMenuCommandStatus : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandStatus, IDS_MN_VIEW_STATUS, IDS_MD_VIEW_STATUS, NULL, 0>
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

	EMenuCommandState IntState()
	{
		return m_pFrame->IsStatusBarVisible() ? EMCSChecked : EMCSNormal;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		m_pFrame->ShowStatusBar(!m_pFrame->IsStatusBarVisible());
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumStatusBar(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandStatus>* p = NULL;
	CComObject<CDocumentMenuCommandStatus>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}


// View->Configure Layout

extern __declspec(selectany) GUID const MenuCommandsLayoutCfgID = {0x3ae975e9, 0x191c, 0x40d8, {0x8f, 0x8c, 0xa4, 0x2e, 0x7c, 0x74, 0x4, 0xbc}};
extern __declspec(selectany) GUID const MenuCommandLayoutCfgID = {0x3ac3a106, 0xa837, 0x4a2a, {0xbf, 0xff, 0xc9, 0xd8, 0x18, 0x8d, 0x9f, 0x22}};

HICON GetIconLayoutEdit(ULONG size);

class ATL_NO_VTABLE CDocumentMenuCommandLayoutCfg : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandLayoutCfg, IDS_MN_VIEW_LAYOUTCFG, IDS_MD_VIEW_LAYOUTCFG, &MenuCommandLayoutCfgID, 0>
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
			*a_phIcon = GetIconLayoutEdit(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		return m_pFrame->M_ActiveLayout().Length() ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		BOOL b;
		m_pFrame->OnProfileConfigureCurrent(0, 0, 0, b);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumLayoutCfg(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CConfigValue cVal;
	a_pFrame->GetThread()->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_LAYOUTCOMMANDS), &cVal);
	if (cVal.operator bool())
	{
		CComObject<CDocumentMenuCommandLayoutCfg>* p = NULL;
		CComObject<CDocumentMenuCommandLayoutCfg>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		p->Init(a_pFrame);
		a_pCommands->Insert(pTmp);
	}
}


// View->Switch Layout

extern __declspec(selectany) GUID const MenuCommandsLayoutsID = {0x389a7456, 0xf0a4, 0x4fbd, {0x96, 0x52, 0x63, 0x7a, 0xf0, 0xc3, 0x6e, 0x9f}};


class ATL_NO_VTABLE CDocumentMenuCommandLayout : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	void Init(CMainFrame* a_pFrame, BSTR a_bstrID, BSTR a_bstrName, GUID a_tIconID, IDocument* a_pDocument)
	{
		m_pFrame = a_pFrame;
		m_bstrName.Attach(a_bstrName);
		m_bstrID.Attach(a_bstrID);
		m_tIconID = a_tIconID;
		m_pDocument = a_pDocument;
	}

BEGIN_COM_MAP(CDocumentMenuCommandDesignerTool)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()


	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			*a_ppText = new CMultiLanguageString(m_bstrName.Copy());
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			*a_ppText = _SharedStringTable.GetString(IDS_MD_VIEW_SWITCHLAYOUT);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
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
		CComPtr<IDesignerFrameIcons> p;
		RWCoCreateInstance(p, __uuidof(DesignerFrameIconsManager));
		return p->GetIcon(m_tIconID, a_nSize, a_phIcon);
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
			*a_peState = m_pDocument ? (m_pFrame->M_ActiveLayout() == m_bstrName ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		m_pFrame->SwitchLayout(m_bstrID);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
	CComBSTR m_bstrID;
	CComBSTR m_bstrName;
	GUID m_tIconID;
	CComPtr<IDocument> m_pDocument;
};

void EnumLayouts(IOperationContext* UNREF(a_pStates), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CConfigValue cVal;
	a_pFrame->GetThread()->M_FrameConfig()->ItemValueGet(CComBSTR(CFGID_LAYOUTCOMMANDS), &cVal);
	if (cVal.operator bool())
	{
		CConfigLock cLock(a_pFrame->GetThread());
		a_pFrame->GetThread()->M_Config()->ItemValueGet(CComBSTR(CFGID_VIEWPROFILES), &cVal);
		LONG nCount = cVal;

		for (LONG i = 0; i < nCount; i++)
		{
			OLECHAR szNameID[64+16];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_VIEWPROFILES, i);
			a_pFrame->GetThread()->M_Config()->ItemValueGet(CComBSTR(szNameID), &cVal);
			BSTR bstrID = cVal.Detach().bstrVal;
			wcscat(szNameID, L"\\");
			LPOLESTR p = szNameID+wcslen(szNameID);
			wcscpy(p, CFGID_ICONID);
			a_pFrame->GetThread()->M_Config()->ItemValueGet(CComBSTR(szNameID), &cVal);
			GUID tIconID = cVal;
			wcscpy(p, CFGID_LAYOUTNAME);
			a_pFrame->GetThread()->M_Config()->ItemValueGet(CComBSTR(szNameID), &cVal);
			BSTR bstrName = cVal.Detach().bstrVal;
			{
				CComObject<CDocumentMenuCommandLayout>* p = NULL;
				CComObject<CDocumentMenuCommandLayout>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pFrame, bstrID, bstrName, tIconID, a_pDocument);
				a_pCommands->Insert(pTmp);
			}
		}
	}
}
