
#ifndef _STRINGPARSING_H_
#define _STRINGPARSING_H_

inline bool ByteFromString(LPCSTR a_pszNumber, BYTE* a_pbOut)
{
	*a_pbOut = 0;
	if (a_pszNumber[0] >= '0' && a_pszNumber[0] <= '9')
	{
		*a_pbOut = ((*a_pbOut) << 4) | (a_pszNumber[0]-'0');
	}
	else if (a_pszNumber[0] >= 'a' && a_pszNumber[0] <= 'f')
	{
		*a_pbOut = ((*a_pbOut) << 4) | (a_pszNumber[0]-'a'+10);
	}
	else if (a_pszNumber[0] >= 'A' && a_pszNumber[0] <= 'F')
	{
		*a_pbOut = ((*a_pbOut) << 4) | (a_pszNumber[0]-'A'+10);
	}
	else
	{
		return false;
	}
	if (a_pszNumber[1] >= '0' && a_pszNumber[1] <= '9')
	{
		*a_pbOut = ((*a_pbOut) << 4) | (a_pszNumber[1]-'0');
	}
	else if (a_pszNumber[1] >= 'a' && a_pszNumber[1] <= 'f')
	{
		*a_pbOut = ((*a_pbOut) << 4) | (a_pszNumber[1]-'a'+10);
	}
	else if (a_pszNumber[1] >= 'A' && a_pszNumber[1] <= 'F')
	{
		*a_pbOut = ((*a_pbOut) << 4) | (a_pszNumber[1]-'A'+10);
	}
	else
	{
		return false;
	}
	return true;
}

inline bool ByteFromString(LPCWSTR a_pszNumber, BYTE* a_pbOut)
{
	*a_pbOut = 0;
	if (a_pszNumber[0] >= L'0' && a_pszNumber[0] <= L'9')
	{
		*a_pbOut = static_cast<BYTE>(((*a_pbOut) << 4) | (a_pszNumber[0]-L'0'));
	}
	else if (a_pszNumber[0] >= L'a' && a_pszNumber[0] <= L'f')
	{
		*a_pbOut = static_cast<BYTE>(((*a_pbOut) << 4) | (a_pszNumber[0]-L'a'+10));
	}
	else if (a_pszNumber[0] >= L'A' && a_pszNumber[0] <= L'F')
	{
		*a_pbOut = static_cast<BYTE>(((*a_pbOut) << 4) | (a_pszNumber[0]-L'A'+10));
	}
	else
	{
		return false;
	}
	if (a_pszNumber[1] >= L'0' && a_pszNumber[1] <= L'9')
	{
		*a_pbOut = static_cast<BYTE>(((*a_pbOut) << 4) | (a_pszNumber[1]-L'0'));
	}
	else if (a_pszNumber[1] >= L'a' && a_pszNumber[1] <= L'f')
	{
		*a_pbOut = static_cast<BYTE>(((*a_pbOut) << 4) | (a_pszNumber[1]-L'a'+10));
	}
	else if (a_pszNumber[1] >= L'A' && a_pszNumber[1] <= L'F')
	{
		*a_pbOut = static_cast<BYTE>(((*a_pbOut) << 4) | (a_pszNumber[1]-L'A'+10));
	}
	else
	{
		return false;
	}
	return true;
}

template<typename TChar>
inline bool WordFromString(TChar const* a_pszNumber, WORD* a_pwOut)
{
	BYTE b1;
	BYTE b2;
	bool bRet = true;
	if (!ByteFromString(a_pszNumber, &b1)) bRet = false;
	if (!ByteFromString(a_pszNumber+2, &b2)) bRet = false;
	*a_pwOut = (((WORD)b1)<<8) + b2;
	return bRet;
}

template<typename TChar>
inline bool DwordFromString(TChar const* a_pszNumber, DWORD* a_pdwOut)
{
	WORD w1;
	WORD w2;
	bool bRet = true;
	if (!WordFromString(a_pszNumber, &w1)) bRet = false;
	if (!WordFromString(a_pszNumber+4, &w2)) bRet = false;
	*a_pdwOut = (((DWORD)w1)<<16) + w2;
	return bRet;
}

template<typename TChar>
inline bool GUIDFromString(TChar const* a_pszNumber, GUID* a_ptOut)
{
	bool bRet = true;
	// 5042BB30-DECC-4B81-A542-722381C3C5EB
	if (!DwordFromString(a_pszNumber+ 0, &a_ptOut->Data1)) bRet = false;
	if (!WordFromString (a_pszNumber+ 9, &a_ptOut->Data2)) bRet = false;
	if (!WordFromString (a_pszNumber+14, &a_ptOut->Data3)) bRet = false;
	if (!ByteFromString (a_pszNumber+19, a_ptOut->Data4)) bRet = false;
	if (!ByteFromString (a_pszNumber+21, a_ptOut->Data4+1)) bRet = false;
	if (!ByteFromString (a_pszNumber+24, a_ptOut->Data4+2)) bRet = false;
	if (!ByteFromString (a_pszNumber+26, a_ptOut->Data4+3)) bRet = false;
	if (!ByteFromString (a_pszNumber+28, a_ptOut->Data4+4)) bRet = false;
	if (!ByteFromString (a_pszNumber+30, a_ptOut->Data4+5)) bRet = false;
	if (!ByteFromString (a_pszNumber+32, a_ptOut->Data4+6)) bRet = false;
	if (!ByteFromString (a_pszNumber+34, a_ptOut->Data4+7)) bRet = false;
	return bRet;
}

inline LPSTR StringFromGUID(const GUID& a_guidIn, LPSTR a_pszOut)
{
	sprintf(a_pszOut, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
				a_guidIn.Data1, (DWORD)a_guidIn.Data2, (DWORD)a_guidIn.Data3,
				(DWORD)a_guidIn.Data4[0], (DWORD)a_guidIn.Data4[1],
				(DWORD)a_guidIn.Data4[2], (DWORD)a_guidIn.Data4[3],
				(DWORD)a_guidIn.Data4[4], (DWORD)a_guidIn.Data4[5],
				(DWORD)a_guidIn.Data4[6], (DWORD)a_guidIn.Data4[7]);
	return a_pszOut;
}

inline LPWSTR StringFromGUID(const GUID& a_guidIn, LPWSTR a_pszOut)
{
	swprintf(a_pszOut, L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
				a_guidIn.Data1, (DWORD)a_guidIn.Data2, (DWORD)a_guidIn.Data3,
				(DWORD)a_guidIn.Data4[0], (DWORD)a_guidIn.Data4[1],
				(DWORD)a_guidIn.Data4[2], (DWORD)a_guidIn.Data4[3],
				(DWORD)a_guidIn.Data4[4], (DWORD)a_guidIn.Data4[5],
				(DWORD)a_guidIn.Data4[6], (DWORD)a_guidIn.Data4[7]);
	return a_pszOut;
}

#endif//_STRINGPARSING_H_
