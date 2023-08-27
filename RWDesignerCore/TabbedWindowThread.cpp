
#include "stdafx.h"
/*
#include "TabbedWindowThread.h"
#include "TabbedMainFrame.h"
#include "FatalErrorDlg.h"
#include "ConfigIDsApp.h"


unsigned CTabbedWindowThread::MessageLoop()
{
	if (FAILED(OleInitialize(NULL)))
		return 0;
	//if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	//	return 0;

	bool bInErr = false;

	CComPtr<ILocalizedString> pCaption;
	if (m_pAppInfo) m_pAppInfo->Name(&pCaption);

	try
	{
		SetThreadLocale(m_tLocaleID);
		CComObject<CTabbedMainFrame>* pFrameWnd = NULL;
		CComObject<CTabbedMainFrame>::CreateInstance(&pFrameWnd);
		CComPtr<IUnknown> pTmp(pFrameWnd);
		pFrameWnd->Init(m_pOrder, this);

		CConfigValue cXPos, cYPos, cWidth, cHeight, cMaximized;
		{
			CConfigLock cLock(this);
			M_Config()->ItemValueGet(CComBSTR(CFGID_WIN_X_POS), &cXPos);
			M_Config()->ItemValueGet(CComBSTR(CFGID_WIN_Y_POS), &cYPos);
			M_Config()->ItemValueGet(CComBSTR(CFGID_WIN_WIDTH), &cWidth);
			M_Config()->ItemValueGet(CComBSTR(CFGID_WIN_HEIGHT), &cHeight);
			M_Config()->ItemValueGet(CComBSTR(CFGID_WIN_MAXIMIZED), &cMaximized);
		}
		RECT myrect;
		myrect.left = implicit_cast<LONG>(cXPos);
		myrect.top = cYPos.operator LONG();
		myrect.right = myrect.left + cWidth.operator LONG();
		myrect.bottom = myrect.top + cHeight.operator LONG();

		if (pFrameWnd->Create(NULL, &myrect, _T("RealWorld Designer"), WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS) == NULL)
		{
			throw (HRESULT)E_FAIL;
		}

		pFrameWnd->ShowWindow(cMaximized.operator bool() ? SW_SHOWMAXIMIZED : SW_SHOW);
		::SetForegroundWindow(*pFrameWnd);	// Win95 needs this
		CreationFinished();

		try
		{
			Run();
		}
		catch (...)
		{
			bInErr = true;
			CFatalErrorDlg cDlg(NULL/*pFrameWnd->M_Doc()/, pCaption, m_tLocaleID);
			cDlg.DoModal();
		}
	}
	catch (...)
	{
		if (!bInErr)
		{
			CFatalErrorDlg cDlg(NULL, pCaption, m_tLocaleID);
			cDlg.DoModal();
		}
	}

	delete this;
	//CoUninitialize();
	OleUninitialize();
	return 0;
}
*/