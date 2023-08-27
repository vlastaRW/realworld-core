
#include "stdafx.h"
#include "WindowThread.h"
#include "MainFrame.h"
#include "FatalErrorDlg.h"

void CWindowThread::SetLocaleID(LCID a_tLocaleID)
{
	SetThreadLocale(a_tLocaleID);
	m_tLocaleID = a_tLocaleID;
}

HANDLE CWindowThread::Start()
{
	unsigned uThID;
	return (HANDLE)_beginthreadex(NULL, 0, MessageLoopProc, this, 0, &uThID);
}

unsigned __stdcall CWindowThread::MessageLoopProc(void* a_pThis)
{
	return reinterpret_cast<CWindowThread*>(a_pThis)->MessageLoop();
}

#include "ConfigIDsApp.h"

//bool IsDWMAvailable();
//#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
//#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
//#endif

unsigned CWindowThread::MessageLoop()
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
		CComObject<CMainFrame>* pFrameWnd = NULL;
		CComObject<CMainFrame>::CreateInstance(&pFrameWnd);
		CComPtr<ISharedStateManager> pTmp(pFrameWnd);
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

		if (pFrameWnd->CreateEx(NULL, &myrect) == NULL)
		{
			throw (HRESULT)E_FAIL;
		}

		//if (IsDWMAvailable())
		//{
		//	BOOL value = TRUE;
		//	::DwmSetWindowAttribute(*pFrameWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
		//}

		pFrameWnd->ShowWindow(cMaximized.operator bool() ? SW_SHOWMAXIMIZED : SW_SHOW);
		::SetForegroundWindow(*pFrameWnd);	// Win95 needs this
		CreationFinished(*pFrameWnd);

		try
		{
			BOOL bDoIdle = TRUE;
			int nIdleCount = 0;
			BOOL bRet;
			DWORD dwLastTimer = GetTickCount();

			for(;;)
			{
				while (bDoIdle && !::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE))
				{
					if(!OnIdle(nIdleCount++))
						bDoIdle = FALSE;
				}

				bRet = ::GetMessage(&m_msg, NULL, 0, 0);

				if (bRet == -1)
				{
					ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
					continue;   // error, don't process
				}
				else if (!bRet)
				{
					ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
					break;   // WM_QUIT, exit message loop
				}

				if (!PreTranslateMessage(&m_msg))
				{
					::TranslateMessage(&m_msg);
					::DispatchMessage(&m_msg);
				}

				if (IsIdleMessage(&m_msg))
				{
					if (m_msg.message == WM_TIMER)
					{
						//DWORD const dw = GetTickCount();
						//if (dw < dwLastTimer+500)
							continue;
						//dwLastTimer = dw;
					}
					bDoIdle = TRUE;
					nIdleCount = 0;
				}
			}
		}
		catch (...)
		{
			bInErr = true;
			CFatalErrorDlg cDlg(pFrameWnd->M_Doc(), pCaption, m_tLocaleID);
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

