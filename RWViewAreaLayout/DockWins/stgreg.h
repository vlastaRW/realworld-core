// Copyright (c) 2006
// Sergey Klimov (kidd@ukr.net)

#ifndef WTL_DW_STGREG_H_INCLUDED_
#define WTL_DW_STGREG_H_INCLUDED_

#include <stg.h>
#include <cassert>


namespace sstate{

namespace{
REGSAM ModesMap[]={KEY_READ,KEY_WRITE,KEY_ALL_ACCESS};
}

class CStgRegistry
	: public IStorge
{
public:
	CStgRegistry(HKEY key=0)
		:m_key(key)
	{

	}

	virtual ~CStgRegistry(void)
	{
		if(m_key!=0)
			::RegCloseKey(m_key);
	}

	virtual long Create(IStorge& parent,LPCTSTR name,Modes mode)
	{
		DWORD disposition;
		return RegCreateKeyEx(static_cast<CStgRegistry&>(parent).m_key,name,0,0,
								REG_OPTION_NON_VOLATILE,ModesMap[mode],0,&m_key,&disposition);
	}

	virtual long Open(IStorge& parent,LPCTSTR name,Modes mode)
	{
		return RegOpenKeyEx(static_cast<CStgRegistry&>(parent).m_key,name,0,ModesMap[mode],&m_key);
	}

	virtual long SetBinary(LPCTSTR name,const void* data,size_t size)
	{
		assert(m_key!=0);
		DWORD type=(size==sizeof(DWORD) ? REG_DWORD : REG_BINARY);
		return ::RegSetValueEx(m_key,name,0,type,static_cast<const BYTE*>(data),size);
	}

	virtual long GetBinary(LPCTSTR name,void* data,size_t& size)
	{
		assert(m_key!=0);
		DWORD type;
		DWORD cbData=DWORD(size);
		long res=::RegQueryValueEx(m_key,name,0,&type,static_cast<LPBYTE>(data),&cbData);
		size=size_t(cbData);
		return res;
	}

	virtual long SetString(LPCTSTR name,LPCTSTR data)
	{
		assert(m_key!=0);
		assert(data);
		return ::RegSetValueEx(m_key,name,0,REG_SZ,
			reinterpret_cast<const BYTE*>(data),std::char_traits<TCHAR>::length(data)*sizeof(TCHAR));
	}

	virtual long GetString(LPCTSTR name,LPTSTR data,size_t& size)
	{
		assert(m_key!=0);
		DWORD type;
		DWORD cbData=DWORD(size);
		long res=::RegQueryValueEx(m_key,name,0,&type,reinterpret_cast<LPBYTE>(data),&cbData);
		assert( res!=ERROR_SUCCESS 
				|| (type==REG_SZ)
				|| (type==REG_EXPAND_SZ)
				);
		size=size_t(cbData);
		return res;
	}

private:
	CStgRegistry(const CStgRegistry&);
	CStgRegistry& operator=(const CStgRegistry&);
private:
	HKEY m_key;
};

}//namespace sstate

#endif // WTL_DW_STGREG_H_INCLUDED_