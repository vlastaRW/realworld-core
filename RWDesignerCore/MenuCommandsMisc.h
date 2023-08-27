
#pragma once

#include "MainFrame.h"
#include <MultiLanguageString.h>


extern __declspec(selectany) GUID const MenuCommandsExecuteCommandID = {0x2f7c8fa0, 0x9c18, 0x4dc6, {0xad, 0x12, 0xc4, 0x7b, 0x23, 0x78, 0xb4, 0x37}};

// Misc->Execute Command

class ATL_NO_VTABLE CDocumentMenuCommandExecuteCommand : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	void Init(CMainFrame* a_pFrame, IDocument* a_pDocument, ILocalizedString* a_pName, ILocalizedString* a_pDesc, BSTR a_bstrCommand, BSTR m_bstrParams, GUID a_nIconID)
	{
		m_pName = a_pName;
		m_pDesc = a_pDesc;
		if (a_bstrCommand)
			m_strCommand = COLE2T(a_bstrCommand);
		if (m_bstrParams)
			m_strParams = COLE2T(m_bstrParams);
		m_nIconID = a_nIconID;
		m_pFrame = a_pFrame;
		m_pDocument = a_pDocument;
	}

BEGIN_COM_MAP(CDocumentMenuCommandExecuteCommand)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			(*a_ppText = m_pName)->AddRef();
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
			(*a_ppText = m_pDesc)->AddRef();
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
			*a_pIconID = m_nIconID;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			CComPtr<IDesignerFrameIcons> pIcons;
			RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
			return pIcons->GetIcon(m_nIconID, a_nSize, a_phIcon);
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
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
	static DWORD WINAPI OpenIEThreadProc(LPVOID a_pParam)
	{
		::CoInitialize(NULL);
		try
		{
			if (reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_strParams.find(_T("%DOCUMENTPATH%")) == std::tstring::npos)
			{
				::ShellExecute(0, _T("open"),
					reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_strCommand.c_str(),
					reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_strParams.c_str(),
					0, SW_SHOWNORMAL);
			}
			else
			{
				std::tstring strParams(reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_strParams);
				CComPtr<IStorageFilter> pLocation;
				if (reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_pDocument)
					reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_pDocument->LocationGet(&pLocation);
				CComBSTR bstr;
				if (pLocation)
					pLocation->ToText(NULL, &bstr);
				if (bstr)
				{
					COLE2CT str(bstr);
					size_t n;
					while (std::tstring::npos != (n = strParams.find(_T("%DOCUMENTPATH%"))))
						strParams.replace(n, 14, LPCTSTR(str));
				}
				::ShellExecute(0, _T("open"),
					reinterpret_cast<CDocumentMenuCommandExecuteCommand*>(a_pParam)->m_strCommand.c_str(),
					strParams.c_str(),
					0, SW_SHOWNORMAL);
			}
		}
		catch (...)
		{
		}
		::CoUninitialize();

		return 0;
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		DWORD dwThID;
		HANDLE hThread = CreateThread(NULL, 0, OpenIEThreadProc, this, 0, &dwThID);
		AtlWaitWithMessageLoop(hThread);
		CloseHandle(hThread);
		return S_OK;
	}

private:
	CComPtr<ILocalizedString> m_pName;
	CComPtr<ILocalizedString> m_pDesc;
	std::tstring m_strCommand;
	std::tstring m_strParams;
	GUID m_nIconID;
	CMainFrame* m_pFrame;
	CComPtr<IDocument> m_pDocument;
};

static const OLECHAR CFGID_EXEC_NAME[] = L"Name";
static const OLECHAR CFGID_EXEC_DESC[] = L"Description";
static const OLECHAR CFGID_EXEC_COMMAND[] = L"Command";
static const OLECHAR CFGID_EXEC_PARAMS[] = L"Parameters";
static const OLECHAR CFGID_EXEC_ICONID[] = L"IconID";

void EnumExecuteCommand(IOperationContext* a_pStates, IConfig* a_pConfig, IDocument* a_pDocument, IDesignerView* UNREF(a_pView), CMainFrame* a_pFrame, IEnumUnknownsInit* a_pCommands)
{
	CConfigValue cName;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_EXEC_NAME), &cName);
	CConfigValue cDesc;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_EXEC_DESC), &cDesc);
	CComPtr<ILocalizedString> pName;
	pName.Attach(new CMultiLanguageString(cName.Detach().bstrVal));
	CComPtr<ILocalizedString> pDesc;
	pDesc.Attach(new CMultiLanguageString(cDesc.Detach().bstrVal));
	CConfigValue cCmd;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_EXEC_COMMAND), &cCmd);
	CConfigValue cParam;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_EXEC_PARAMS), &cParam);
	CConfigValue cIconID;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_EXEC_ICONID), &cIconID);
	CComObject<CDocumentMenuCommandExecuteCommand>* p = NULL;
	CComObject<CDocumentMenuCommandExecuteCommand>::CreateInstance(&p);
	CComPtr<IDocumentMenuCommand> pTmp = p;
	p->Init(a_pFrame, a_pDocument, pName, pDesc, cCmd, cParam, cIconID);
	a_pCommands->Insert(pTmp);
}

void ConfigExecuteCommand(IConfigWithDependencies* a_pConfig)
{
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_EXEC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_NAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_NAME_DESC), CConfigValue(L""), NULL, 0, NULL);
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_EXEC_DESC), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_DESC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_DESC_DESC), CConfigValue(L""), NULL, 0, NULL);
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_EXEC_COMMAND), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_COMMAND_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_COMMAND_DESC), CConfigValue(L"explorer.exe"), NULL, 0, NULL);
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_EXEC_PARAMS), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_PARAMS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_EXEC_PARAMS_DESC), CConfigValue(L""), NULL, 0, NULL);

	CComBSTR cCFGID_ICONID(CFGID_ICONID);
	CComPtr<IConfigItemCustomOptions> pCustIconIDs;
	RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
	if (pCustIconIDs != NULL)
		a_pConfig->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
	else
		a_pConfig->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);
}

