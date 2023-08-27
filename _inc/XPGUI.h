
#pragma once

namespace XPGUI
{

	inline bool IsXP()
	{
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
		{
			GetVersionEx(&tVersion);
		}
		return tVersion.dwMajorVersion > 5 || (tVersion.dwMajorVersion == 5 && tVersion.dwMinorVersion >= 1);
	}

	inline bool IsVista()
	{
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
		{
			GetVersionEx(&tVersion);
		}
		return tVersion.dwMajorVersion >= 6;
	}

	inline bool IsWin7()
	{
		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
		{
			GetVersionEx(&tVersion);
		}
		return tVersion.dwMajorVersion > 6 || (tVersion.dwMajorVersion == 6 && tVersion.dwMinorVersion >= 1);
	}

	inline UINT GetImageListColorFlags()
	{
		return IsXP() ? (ILC_MASK|ILC_COLOR32) : (ILC_MASK|ILC_COLOR24); // in XP, the mask is needed when icon w/o alpha channel is added
	}

	inline int GetSmallIconSize()
	{
		static int nSize = 0;
		if (nSize)
			return nSize;
		int nSizeSys = GetSystemMetrics(SM_CXSMICON);
		if (nSizeSys >= 24)
			nSize = nSizeSys;
		else if (nSizeSys >= 20)
			nSize = 20;
		else
			nSize = 16;
		return nSize;
	}

#ifdef _WIN64
#define TBBUTTON_PADDING {0, 0, 0, 0, 0, 0}
#else
#define TBBUTTON_PADDING {0, 0}
#endif
};