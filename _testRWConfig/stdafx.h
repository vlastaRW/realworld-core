// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__604BB641_F3EA_4BD0_8618_E0D0A7B70220__INCLUDED_)
#define AFX_STDAFX_H__604BB641_F3EA_4BD0_8618_E0D0A7B70220__INCLUDED_

// Change these values to use different versions
#define WINVER		0x0600
#define _WIN32_IE	0x0600
#define _RICHEDIT_VER	0x0300

#define _WIN32_WINNT 0x0600
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
#include <atlapp.h>

extern CServerAppModule _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <atlctrls.h>

#define RWCOM_ROOTEXE
#include "CodingStyle.h"

#include "RWConfig.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__604BB641_F3EA_4BD0_8618_E0D0A7B70220__INCLUDED_)
