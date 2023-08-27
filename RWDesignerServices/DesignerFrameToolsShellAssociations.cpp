// DesignerFrameToolsShellAssociations.cpp : Implementation of CDesignerFrameToolsShellAssociations

#include "stdafx.h"
#include "DesignerFrameToolsShellAssociations.h"
#include "ShellAssociationsDlg.h"

#include <SharedStringTable.h>
#include <DPIUtils.h>
#include <XPGUI.h>
#include <IconRenderer.h>


// CDesignerFrameToolsShellAssociations

STDMETHODIMP CDesignerFrameToolsShellAssociations::Size(ULONG* a_pnCount)
{
	try
	{
		*a_pnCount = 1;
		return S_OK;
	}
	catch (...)
	{
		return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameToolsShellAssociations::Name(ULONG a_nIndex, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_nIndex > 0) return E_RW_INDEXOUTOFRANGE;
		*a_ppName = new CMultiLanguageString(L"[0409]File associations...[0405]Asociované typy souborů...");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameToolsShellAssociations::HelpText(ULONG a_nIndex, ILocalizedString** a_ppHelpText)
{
	try
	{
		*a_ppHelpText = NULL;
		if (a_nIndex > 0) return E_RW_INDEXOUTOFRANGE;
		*a_ppHelpText = new CMultiLanguageString(L"[0409]Manage Windows shell associations[0405]Správa asociovaných typů soborů v uživatelském rozhraní Windows");
		return S_OK;
	}
	catch (...)
	{
		return a_ppHelpText == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameToolsShellAssociations::Icon(ULONG a_nIndex, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		if (a_nIndex > 0) return E_RW_INDEXOUTOFRANGE;

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(1.0f, 0, -1));
		pSI->GetLayers(ESIModify, cRenderer, IRTarget(0.75f, 0.5, 1));
		*a_phIcon = cRenderer.get();
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerFrameToolsShellAssociations::Activate(ULONG a_nIndex, RWHWND a_hFrameWnd, LCID a_tLocaleID, IDocument* UNREF(a_pDocument))
{
	try
	{
		if (a_nIndex > 0) return E_RW_INDEXOUTOFRANGE;
		static bool s_bInit = false;
		if (!s_bInit)
		{
			INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_LISTVIEW_CLASSES};
			InitCommonControlsEx(&tICCE);
			s_bInit = true;
		}
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(GetSystemMetrics(SM_CXSMICON));
		pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(1.0f, 0, -1));
		pSI->GetLayers(ESIModify, cRenderer, IRTarget(0.75f, 0.5, 1));
		HICON hIcon = cRenderer.get();
		CShellAssociationsDlg(a_tLocaleID, hIcon).DoModal(reinterpret_cast<HWND>(a_hFrameWnd));
		DestroyIcon(hIcon);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

