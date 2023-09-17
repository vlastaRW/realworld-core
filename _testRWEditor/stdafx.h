// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#define ISOLATION_AWARE_ENABLED 1

#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0600		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0601	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0700	// Change this to the appropriate value to target IE 5.0 or later.
#endif


#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atltime.h>
using namespace ATL;
#define _CSTRING_NS	ATL

#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>

using namespace WTL;

#include <string>

using namespace std;

typedef basic_string<TCHAR> tstring;

#include <RWErrorCodes.h>
#define RWCOM_ROOTEXE
#include <CodingStyle.h>
