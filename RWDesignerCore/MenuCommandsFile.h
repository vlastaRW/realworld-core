
#pragma once

#include "MainFrame.h"
#include "RecentFiles.h"
#include "AbbreviateName.h"
#include <SimpleLocalizedString.h>
#include "WindowThread.h"
#include <IconRenderer.h>


extern __declspec(selectany) GUID const MenuCommandsNewOpenSaveID = {0x672af161, 0x9feb, 0x4c07, {0xb9, 0x1a, 0xc3, 0x43, 0xea, 0x50, 0x2f, 0x2f}};

#include "DocumentMenuCommandImpl.h"

HICON GetIconNewDoc(ULONG size);
HICON GetIconOpenFile(ULONG size);
HICON GetIconSaveFile(ULONG size);


// File->Open

extern __declspec(selectany) GUID const MenuCommandsOpenID = {0x7bff39a4, 0x668d, 0x46d0, {0xa1, 0x88, 0xd3, 0x6f, 0x32, 0x5c, 0xd9, 0xf7}};

class ATL_NO_VTABLE CDocumentMenuCommandOpen : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandOpen, IDS_MN_FILE_OPEN, IDS_MD_FILE_OPEN, &MenuCommandsOpenID, 0>
{
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetIconOpenFile(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = 'O';
				a_pAccel->fVirtFlags = FCONTROL;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		CWaitCursor cWait;
		CComPtr<IDesignerCore> p;
		RWCoCreateInstance(p, __uuidof(DesignerCore));
		RWHWND h;
		return p->NewWindowPath(NULL, &h);
	}
};

// File->New

extern __declspec(selectany) GUID const MenuCommandsNewID = {0x292a5c5, 0x6afa, 0x4fc0, {0xbd, 0x6e, 0x27, 0xbf, 0xaa, 0x4, 0x52, 0xe5}};

class ATL_NO_VTABLE CDocumentMenuCommandNew : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandNew, IDS_MN_FILE_NEW, IDS_MD_FILE_NEW, &MenuCommandsNewID, 0>
{
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetIconNewDoc(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = 'N';
				a_pAccel->fVirtFlags = FCONTROL;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		CWaitCursor cWait;
		CComPtr<IDesignerCore> p;
		RWCoCreateInstance(p, __uuidof(DesignerCore));
		RWHWND h;
		return p->NewWindowDocument(NULL, &h);
	}
};

// File->Save

extern __declspec(selectany) GUID const MenuCommandsSaveID = {0x21fa301a, 0xefe1, 0x414b, {0x81, 0xe7, 0x7a, 0x60, 0x45, 0xb8, 0xd7, 0xbd}};

class ATL_NO_VTABLE CDocumentMenuCommandSave : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandSave, IDS_MN_FILE_SAVE, IDS_MD_FILE_SAVE, &MenuCommandsSaveID, 0>
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
			*a_phIcon = GetIconSaveFile(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = 'S';
				a_pAccel->fVirtFlags = FCONTROL;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		return m_pFrame->M_Doc() ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		BOOL b;
		m_pFrame->OnFileSave(false);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumNewOpenSave(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandNew>* p2 = NULL;
	CComObject<CDocumentMenuCommandNew>::CreateInstance(&p2);
	CComPtr<IDocumentMenuCommand> pTmp = p2;
	a_pCommands->Insert(pTmp);
	CComObject<CDocumentMenuCommandOpen>* p = NULL;
	CComObject<CDocumentMenuCommandOpen>::CreateInstance(&p);
	pTmp = p;
	a_pCommands->Insert(pTmp);
	CComObject<CDocumentMenuCommandSave>* p3 = NULL;
	CComObject<CDocumentMenuCommandSave>::CreateInstance(&p3);
	pTmp = p3;
	p3->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}


// File->Save As

extern __declspec(selectany) GUID const MenuCommandsSaveAsID = {0xcf3cce7f, 0x1928, 0x4c6b, {0xbe, 0x40, 0x19, 0x58, 0x70, 0xff, 0x86, 0xcf}};

class ATL_NO_VTABLE CDocumentMenuCommandSaveAs : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandSaveAs, IDS_MN_FILE_SAVEAS, IDS_MD_FILE_SAVEAS, NULL, 0>
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = 'S';
				a_pAccel->fVirtFlags = FCONTROL|FSHIFT;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		return m_pFrame->M_Doc() ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		m_pFrame->OnFileSave(true);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumSaveAs(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandSaveAs>* p = NULL;
	CComObject<CDocumentMenuCommandSaveAs>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}


// File->Close

extern __declspec(selectany) GUID const MenuCommandsCloseID = {0x254d445a, 0xda18, 0x47ae, {0xa1, 0xd0, 0xbf, 0xb2, 0x20, 0x63, 0xb1, 0xd8}};

class ATL_NO_VTABLE CDocumentMenuCommandClose : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandClose, IDS_MN_FILE_CLOSE, IDS_MD_FILE_CLOSE, NULL, 0>
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			if (a_pAccel)
			{
				a_pAccel->wKeyCode = VK_F4;
				a_pAccel->fVirtFlags = FCONTROL|FVIRTKEY;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	EMenuCommandState IntState()
	{
		return m_pFrame->M_Doc() ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		BOOL b;
		m_pFrame->OnFileClose(0, 0, 0, b);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumClose(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandClose>* p = NULL;
	CComObject<CDocumentMenuCommandClose>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}


// File->Exit

extern __declspec(selectany) GUID const MenuCommandsExitID = {0x15dc3b22, 0xaba7, 0x438a, {0x9f, 0x14, 0x78, 0x68, 0x3, 0x83, 0xfa, 0x64}};

class ATL_NO_VTABLE CDocumentMenuCommandExit : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandExit, IDS_MN_FILE_EXIT, IDS_MD_FILE_EXIT, NULL, 0>
{
public:
	void Init(CMainFrame* a_pFrame)
	{
		m_pFrame = a_pFrame;
	}

	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		BOOL b;
		m_pFrame->OnFileExit(0, 0, 0, b);
		return S_OK;
	}

private:
	CMainFrame* m_pFrame;
};

void EnumExit(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CComObject<CDocumentMenuCommandExit>* p = NULL;
	CComObject<CDocumentMenuCommandExit>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame);
	a_pCommands->Insert(pTmp);
}


// File->Recent

extern __declspec(selectany) GUID const MenuCommandsRecentID = {0xbd5e1ba7, 0x22ba, 0x4094, {0x89, 0xe2, 0x6d, 0x70, 0x10, 0x82, 0xce, 0x96}};

class ATL_NO_VTABLE CDocumentMenuCommandRecent : 
	public CDocumentMenuCommandImpl<CDocumentMenuCommandRecent, 0, IDS_MD_FILE_RECENT, NULL, 0>
{
public:
	void Init(std::tstring a_strRecentFile)
	{
		m_strRecentFile = a_strRecentFile;
	}

	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			TCHAR szTmp[MAX_PATH]; // TODO: ...
			_tcscpy(szTmp, m_strRecentFile.c_str());
			AbbreviateName(szTmp, 40, TRUE);
			*a_ppText = new CSimpleLocalizedString(CComBSTR(szTmp).Detach());
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		CComPtr<IDesignerCore> pDesignerCore;
		RWCoCreateInstance(pDesignerCore, __uuidof(DesignerCore));
		return pDesignerCore->NewWindowPath(CComBSTR(m_strRecentFile.c_str()), NULL);
	}

private:
	std::tstring m_strRecentFile;
};

void EnumRecent(IOperationContext* a_pStates, IConfig* UNREF(a_pConfig), IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	std::vector<std::tstring> cRecent;
	{
		CConfigLock cLock(a_pFrame->GetThread());
		RecentFiles::GetRecentFileList(a_pFrame->GetThread()->M_Config(), cRecent);
	}
	int iCnt = 16;
	for (std::vector<std::tstring>::const_iterator i = cRecent.begin(); i != cRecent.end() && iCnt > 0; ++i, --iCnt)
	{
		CComObject<CDocumentMenuCommandRecent>* p = NULL;
		CComObject<CDocumentMenuCommandRecent>::CreateInstance(&p);
		CComPtr<IDocumentMenuCommand> pTmp = p;
		p->Init(*i);
		a_pCommands->Insert(pTmp);
	}
}

