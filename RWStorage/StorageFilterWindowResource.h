// StorageFilterWindowResource.h : Declaration of the CStorageFilterWindowResource

#pragma once
#include "RWStorage.h"

#include <Win32LangEx.h>


// CStorageFilterWindowResource

class ATL_NO_VTABLE CStorageFilterWindowResource : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<CStorageFilterWindowResource>,
	public CDialogResize<CStorageFilterWindowResource>,
	public CChildWindowImpl<CStorageFilterWindowResource, IStorageFilterWindow>
{
public:
	CStorageFilterWindowResource()
	{
	}

	HRESULT Init(LPCOLESTR a_pszInitial, HWND a_hWnd, LCID a_tLocaleID);

	BEGIN_COM_MAP(CStorageFilterWindowResource)
		COM_INTERFACE_ENTRY(IStorageFilterWindow)
	END_COM_MAP()

	enum
	{
		IDC_FWRES_FILENAME = 100,
		IDC_FWRES_TYPE_TEXT,
		IDC_FWRES_TYPE,
		IDC_FWRES_ID_TEXT,
		IDC_FWRES_ID,
	};

	BEGIN_DIALOG_EX(0, 0, 250, 125, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_SYSMENU | DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_CONTROLPARENT)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]File &name:[0405]&Jméno souboru:"), IDC_STATIC, 7, 9, 55, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_FWRES_FILENAME, 63, 7, 180, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Resource &type:[0405]&Typ resource:"), IDC_FWRES_TYPE_TEXT, 7, 25, 113, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_FWRES_TYPE, 7, 38, 113, 81, CBS_SIMPLE | CBS_AUTOHSCROLL | CBS_SORT | CBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Resource &ID:[0405]&ID resource:"), IDC_FWRES_ID_TEXT, 130, 25, 113, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_FWRES_ID, 130, 38, 113, 81, CBS_SIMPLE | CBS_AUTOHSCROLL | CBS_SORT | CBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CStorageFilterWindowResource)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogResize<CStorageFilterWindowResource>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CStorageFilterWindowResource)
		DLGRESIZE_CONTROL(IDC_FWRES_FILENAME, DLSZ_SIZE_X)
		BEGIN_DLGRESIZE_GROUP()
			DLGRESIZE_CONTROL(IDC_FWRES_TYPE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_FWRES_ID, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_GROUP()
		BEGIN_DLGRESIZE_GROUP()
			DLGRESIZE_CONTROL(IDC_FWRES_TYPE_TEXT, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_FWRES_ID_TEXT, DLSZ_SIZE_X)
		END_DLGRESIZE_GROUP()
	END_DLGRESIZE_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IStorageFilterWindow methods
public:
	STDMETHOD(FilterCreate)(IStorageFilter** a_ppFilter);
	STDMETHOD(FiltersCreate)(IEnumUnknowns** a_ppFilters);
	STDMETHOD(DocTypesEnum)(IEnumUnknowns** a_pFormatFilters) { return E_NOTIMPL; }
	STDMETHOD(DocTypeGet)(IDocumentType** a_pFormatFilter) { return E_NOTIMPL; }
	STDMETHOD(DocTypeSet)(IDocumentType* a_pFormatFilter) { return E_NOTIMPL; }
	STDMETHOD(NavigationCommands)(IEnumUnknowns** a_ppCommands) { return E_NOTIMPL; }
	STDMETHOD(OnIdle)() { return S_FALSE; }

	// handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	LRESULT OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled);
	virtual void OnFinalMessage(HWND a_hWnd);

private:
};

