// NamedColors.cpp : Implementation of CNamedColors

#include "stdafx.h"
#include "NamedColors.h"

#include <MultiLanguageString.h>


// CNamedColors

STDMETHODIMP CNamedColors::ColorToName(DWORD a_dwRGBA, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if ((a_dwRGBA&0xff000000) == 0)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]transparent[0405]průhledná");
			return S_OK;
		}
		struct {COLORREF clr; LPCOLESTR psz;} static const s_aTbl[] =
		{
			{RGB(0x00, 0x00, 0x00), L"[0409]black[0405]černá"},
			{RGB(0xA5, 0x2A, 0x00), L"[0409]brown[0405]hnědá"},
			{RGB(0x00, 0x40, 0x40), L"[0409]dark olive green[0405]tmavá olivovězelená"},
			{RGB(0x00, 0x55, 0x00), L"[0409]dark green[0405]tmavězelená"},
			{RGB(0x00, 0x00, 0x5E), L"[0409]dark teal[0405]tmavá modrozelená"},
			{RGB(0x00, 0x00, 0x8B), L"[0409]dark blue[0405]|tmavěmodrá"},
			{RGB(0x4B, 0x00, 0x82), L"[0409]indigo[0405]indigová modř"},
			{RGB(0x28, 0x28, 0x28), L"[0409]dark grey[0405]tmavěšedá"},
			{RGB(0x8B, 0x00, 0x00), L"[0409]dark red[0405]rudá"},
			{RGB(0xFF, 0x68, 0x20), L"[0409]orange[0405]oranžová"},
			{RGB(0x8B, 0x8B, 0x00), L"[0409]dark yellow[0405]tmavěžlutá"},
			{RGB(0x00, 0x93, 0x00), L"[0409]green[0405]zelená"},
			{RGB(0x38, 0x8E, 0x8E), L"[0409]teal[0405]tmavězelenomodrá"},
			{RGB(0x00, 0x00, 0xFF), L"[0409]blue[0405]modrá"},
			{RGB(0x7B, 0x7B, 0xC0), L"[0409]blue-grey[0405]šedomodrá"},
			{RGB(0x66, 0x66, 0x66), L"[0409]dim grey[0405]tmavší šedá"},
			{RGB(0xFF, 0x00, 0x00), L"[0409]red[0405]červená"},
			{RGB(0xFF, 0xAD, 0x5B), L"[0409]light orange[0405]světleoranžová"},
			{RGB(0x32, 0xCD, 0x32), L"[0409]lime[0405]žlutozelená"},
			{RGB(0x3C, 0xB3, 0x71), L"[0409]sea green[0405]mořská zeleň"},
			{RGB(0x7F, 0xFF, 0xD4), L"[0409]aqua[0405]zelenomodrá"},
			{RGB(0x7D, 0x9E, 0xC0), L"[0409]light blue[0405]světlemodrá"},
			{RGB(0x80, 0x00, 0x80), L"[0409]violet[0405]fialová"},
			{RGB(0x7F, 0x7F, 0x7F), L"[0409]grey[0405]šedá"},
			{RGB(0xFF, 0xC0, 0xCB), L"[0409]pink[0405]Růžová"},
			{RGB(0xFF, 0xD7, 0x00), L"[0409]gold[0405]zlatá"},
			{RGB(0xFF, 0xFF, 0x00), L"[0409]yellow[0405]žlutá"},
			{RGB(0x00, 0xFF, 0x00), L"[0409]bright green[0405]jasnězelená"},
			{RGB(0x40, 0xE0, 0xD0), L"[0409]turquoise[0405]tyrkysová"},
			{RGB(0xC0, 0xFF, 0xFF), L"[0409]skyblue[0405]Azurová"},
			{RGB(0x48, 0x00, 0x48), L"[0409]plum[0405]švestková"},
			{RGB(0xC0, 0xC0, 0xC0), L"[0409]light grey[0405]světlešedá"},
			{RGB(0xFF, 0xE4, 0xE1), L"[0409]rose[0405]starorůžová"},
			{RGB(0xD2, 0xB4, 0x8C), L"[0409]tan[0405]světlehnědá"},
			{RGB(0xFF, 0xFF, 0xE0), L"[0409]light yellow[0405]světležlutá"},
			{RGB(0x98, 0xFB, 0x98), L"[0409]pale green[0405]bledězelená"},
			{RGB(0xAF, 0xEE, 0xEE), L"[0409]pale turquoise[0405]světletyrkysová"},
			{RGB(0x68, 0x83, 0x8B), L"[0409]pale blue[0405]modrošedá"},
			{RGB(0xE6, 0xE6, 0xFA), L"[0409]lavender[0405]levandulová"},
			{RGB(0xFF, 0xFF, 0xFF), L"[0409]white[0405]bílá"},
		};
		int iBestDist = 1000000;
		int iBest = 0;
		for (int i = 0; i < sizeof(s_aTbl)/sizeof(s_aTbl[0]); ++i)
		{
			int iDistR = int(GetRValue(s_aTbl[i].clr))-int(GetRValue(a_dwRGBA));
			int iDistG = int(GetGValue(s_aTbl[i].clr))-int(GetGValue(a_dwRGBA));
			int iDistB = int(GetBValue(s_aTbl[i].clr))-int(GetBValue(a_dwRGBA));
			int iDist = iDistR*iDistR + iDistG*iDistG + iDistB*iDistB;
			if (iBestDist > iDist)
			{
				iBest = i;
				iBestDist = iDist;
			}
		}
		*a_ppName = new CMultiLanguageString(s_aTbl[iBest].psz);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}
