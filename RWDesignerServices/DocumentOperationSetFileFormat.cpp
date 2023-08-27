// DocumentOperationSetFileFormat.cpp : Implementation of CDocumentOperationSetFileFormat

#include "stdafx.h"
#include "DocumentOperationSetFileFormat.h"
#include <MultiLanguageString.h>


// CDocumentOperationSetFileFormat

STDMETHODIMP CDocumentOperationSetFileFormat::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Document - Set File Format[0405]Dokument - nastavit souborový formát");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISetFileFormatDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUISetFileFormatDlg>,
	public CDialogResize<CConfigGUISetFileFormatDlg>
{
public:
	enum
	{
		IDC_FORMATLABEL = 100,
		IDC_FORMAT,
		IDC_CONFIG,
	};

	BEGIN_DIALOG_EX(0, 0, 200, 45, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Format:[0405]Formát:"), IDC_FORMATLABEL, 0, 2, 59, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_FORMAT, 60, 0, 140, 160, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CONFIG, WC_STATIC, SS_GRAYRECT | WS_VISIBLE, 0, 16, 200, 29, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUISetFileFormatDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISetFileFormatDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUISetFileFormatDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_FORMAT, CBN_SELCHANGE, OnFormatSelChange)
		//NOTIFY_HANDLER(IDC_CG_TAB, CTCN_SELCHANGE, OnTabChange)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISetFileFormatDlg)
		DLGRESIZE_CONTROL(IDC_FORMAT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISetFileFormatDlg)
		CONFIGITEM_CONTEXTHELP(IDC_FORMAT, L"FFEncoder")
		CONFIGITEM_SUBCONFIG_NOMARGINS(IDC_CONFIG, L"FFEncoder")
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		m_wnd = GetDlgItem(IDC_FORMAT);
		M_Config()->ItemGetUIInfo(CComBSTR(L"FFEncoder"), __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&m_item));
		if (m_item)
			m_item->OptionsEnum(&m_options);
	}
	void ExtraConfigNotify()
	{
		ULONG nOptions = 0;
		if (m_options == NULL || FAILED(m_options->Size(&nOptions)) || nOptions == 0)
			return;
		CComPtr<IDocument> pDoc;
		GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
		CConfigValue cVal;
		M_Config()->ItemValueGet(CComBSTR(L"FFEncoder"), &cVal);
		CConfigValue cOption;
		m_wnd.ResetContent();
		for (ULONG i = 0; SUCCEEDED(m_options->Get(i, &cOption)); ++i)
		{
			if (pDoc)
			{
				CComPtr<IDocumentEncoder> enc;
				RWCoCreateInstance(enc, cOption);
				if (enc)
				{
					if (S_OK != enc->CanSerialize(pDoc, NULL))
						continue;
				}
			}

			CComBSTR bstrText;
			CComPtr<ILocalizedString> pStr;
			if (SUCCEEDED(m_item->ValueGetName(cOption, &pStr)) &&
				SUCCEEDED(pStr->GetLocalized(m_tLocaleID, &bstrText)))
			{
				int iCombo = m_wnd.AddString(COLE2CT(bstrText));
				m_wnd.SetItemData(iCombo, i);
				if (cOption == cVal)
					m_wnd.SetCurSel(iCombo);
			}
		}
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnFormatSelChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CConfigValue cVal;
		m_options->Get(m_wnd.GetItemData(m_wnd.GetCurSel()), &cVal);
		if (cVal.TypeGet() != ECVTEmpty)
		{
			CComBSTR bstrID(L"FFEncoder");
			M_Config()->ItemValuesSet(1, &(bstrID.m_str), cVal);
		}
		return 0;
	}

private:
	CComboBox m_wnd;
	CComPtr<IConfigItemOptions> m_item;
	CComPtr<IEnumConfigItemOptions> m_options;
};

STDMETHODIMP CDocumentOperationSetFileFormat::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IPlugInCache> pPIC;
		RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
		CComPtr<IEnumGUIDs> pEncGUIDs;
		CComPtr<IEnumUnknowns> pEncoders;
		pPIC->InterfacesEnum(CATID_DocumentEncoder, __uuidof(IDocumentEncoder), 0xffffffff, &pEncoders, &pEncGUIDs);
		ULONG nEncoders = 0;
		if (pEncoders) pEncoders->Size(&nEncoders);
		if (nEncoders == 0)
			return E_FAIL; // no compatible encoders

		CComPtr<IConfigWithDependencies> pSO;
		RWCoCreateInstance(pSO, __uuidof(ConfigWithDependencies));
		CComPtr<ISubConfigSwitch> pSwitch;
		RWCoCreateInstance(pSwitch, __uuidof(SubConfigSwitch));
		CComBSTR cCFGID_FFSWITCH(L"FFEncoder");
		pSO->ItemIns1ofN(cCFGID_FFSWITCH, CMultiLanguageString::GetAuto(L"[0409]File format[0405]Formát souboru"), CMultiLanguageString::GetAuto(L"[0409]File format[0405]Formát souboru"), CConfigValue(GUID_NULL), pSwitch);

		for (ULONG i = 0; i < nEncoders; ++i)
		{
			GUID tID = GUID_NULL;
			pEncGUIDs->Get(i, &tID);
			CComPtr<IDocumentEncoder> pEnc;
			pEncoders->Get(i, __uuidof(IDocumentEncoder), reinterpret_cast<void**>(&pEnc));

			CComPtr<IConfig> pCfg;
			pEnc->DefaultConfig(&pCfg);
			if (pCfg) pSwitch->ItemInsert(CConfigValue(tID), pCfg);
			CComPtr<IDocumentType> pDocType;
			pEnc->DocumentType(&pDocType);
			CComPtr<ILocalizedString> pTypeName;
			pDocType->TypeNameGet(NULL, &pTypeName);
			pSO->ItemOptionAdd(cCFGID_FFSWITCH, CConfigValue(tID), pTypeName, 0, NULL);
		}

		CConfigCustomGUI<&CLSID_DocumentOperationSetFileFormat, CConfigGUISetFileFormatDlg>::FinalizeConfig(pSO);

		*a_ppDefaultConfig = pSO.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSetFileFormat::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* UNREF(a_pDocument), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	return S_OK;
}

#include <DocumentName.h>

STDMETHODIMP CDocumentOperationSetFileFormat::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComBSTR bstrID(L"FFEncoder");
		CConfigValue cID;
		a_pConfig->ItemValueGet(bstrID, &cID);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstrID, &pCfg);
		HRESULT hRes = a_pDocument->EncoderSet(cID, pCfg);
		if (FAILED(hRes))
			return hRes;

		// change extension in file name
		CComPtr<IDocumentEncoder> pEnc;
		RWCoCreateInstance(pEnc, cID);
		if (pEnc == NULL)
			return hRes;
		CComPtr<IDocumentType> pDocType;
		pEnc->DocumentType(&pDocType);
		CComBSTR bstrExt;
		if (pDocType) pDocType->DefaultExtensionGet(&bstrExt);
		if (bstrExt.Length() == 0)
			return hRes;

		CComPtr<IStorageFilter> pLoc;
		a_pDocument->LocationGet(&pLoc);
		if (pLoc == NULL)
			return hRes;

		CComQIPtr<IDocumentName> pDN(pLoc);
		if (pDN)
		{
			CDocumentName::ChangeExtension(a_pDocument, bstrExt);
		}
		else
		{
			CComBSTR bstrText;
			pLoc->ToText(NULL, &bstrText);
			LPWSTR pDot = NULL;
			if (bstrText.Length() == 0 || (pDot = wcsrchr(bstrText, L'.')) == NULL)
				return hRes;

			CComBSTR bstr(pDot+1-bstrText.m_str, bstrText);
			bstr += bstrExt;

			CComPtr<IStorageManager> pSM;
			RWCoCreateInstance(pSM, __uuidof(StorageManager));
			CComPtr<IStorageFilter> pNewLoc;
			pSM->FilterCreate(bstr, 0, &pNewLoc);
			a_pDocument->LocationSet(pNewLoc);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationSetFileFormat::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	if (a_ppName == NULL)
		return E_POINTER;
	*a_ppName = NULL;
	if (a_pConfig)
	{
		CComBSTR cCFGID_FFSWITCH(L"FFEncoder");
		CConfigValue format;
		a_pConfig->ItemValueGet(cCFGID_FFSWITCH, &format);
		CComPtr<IConfigItem> info;
		a_pConfig->ItemGetUIInfo(cCFGID_FFSWITCH, __uuidof(IConfigItem), reinterpret_cast<void**>(&info));
		if (info)
		{
			CComPtr<IConfigDescriptor> encAss;
			RWCoCreateInstance(encAss, format);
			CComPtr<ILocalizedString> encoderProps;
			if (encAss)
			{
				CComPtr<IConfig> sub;
				a_pConfig->SubConfigGet(cCFGID_FFSWITCH, &sub);
				encAss->Name(NULL, sub, &encoderProps);
			}
			CComPtr<ILocalizedString> encoderName;
			info->ValueGetName(format, &encoderName);
			if (encoderProps == NULL)
			{
				*a_ppName = encoderName.Detach();
				return S_OK;
			}

			CComObject<CPrintfLocalizedString>* p = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&p);
			CComPtr<ILocalizedString> out = p;
			CComPtr<ILocalizedString> templ;
			templ.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));
			p->Init(templ, encoderName, encoderProps);
			*a_ppName = out.Detach();
			return S_OK;
		}
	}
	*a_ppName = new CMultiLanguageString(L"[0409]Set file format[0405]Nastavit souborový formát");
	return S_OK;
}

