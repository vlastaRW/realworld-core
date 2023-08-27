
#include "stdafx.h"
#include <math.h>
#include <GammaCorrection.h>


class ATL_NO_VTABLE CGammaTableCache : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGammaTableCache, &__uuidof(GammaTableCache)>,
	public IGammaTableCache
{
public:
	CGammaTableCache()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CGammaTableCache)

BEGIN_COM_MAP(CGammaTableCache)
	COM_INTERFACE_ENTRY(IGammaTableCache)
END_COM_MAP()


	// IGammaTableCache methods
public:
	CGammaTables const* STDMETHODCALLTYPE GetSRGBTable()
	{
		return &m_sRGB;
	}
	CFloatSRGB const* STDMETHODCALLTYPE GetFloatSRGB()
	{
		return &m_cFloatSRGB;
	}

private:
	CGammaTables m_sRGB;
	CFloatSRGB m_cFloatSRGB;
};

OBJECT_ENTRY_AUTO(__uuidof(GammaTableCache), CGammaTableCache)
