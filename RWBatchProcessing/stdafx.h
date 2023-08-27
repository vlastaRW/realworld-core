// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#define ISOLATION_AWARE_ENABLED 1

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0600		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0600	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_FREE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit


#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <shlobj.h>

// shitty ATL7 x WTL3 hacks
#define _Module (*_pModule)
using namespace ATL;

#include <atlapp.h>
#include <atlctrls.h>
#include <atluser.h>
#include <atlframe.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <atltheme.h>
#undef _Module

using namespace WTL;

#include <queue>
#include <string>
#include <set>
namespace std
{
	typedef basic_string<TCHAR> tstring;
}

#include <RWErrorCodes.h>
#include <CodingStyle.h>
