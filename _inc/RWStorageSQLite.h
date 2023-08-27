
#pragma once

#include <RWStorage.h>


extern __declspec(selectany) IID const IID_IStorageFactorySQLite = { 0xc1aee9d6, 0x9292, 0x4fc6, { 0x9a, 0x9b, 0x82, 0xa4, 0xc7, 0xfa, 0x6f, 0x64 } };

MIDL_INTERFACE("C1AEE9D6-9292-4FC6-9A9B-82A4C7FA6F64")
IStorageFactorySQLite : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE MergeDefaults(ULONG a_nData, BYTE const* a_pData, ULONG a_nVersion, BSTR a_bstrDatabase) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumFiles(BSTR a_bstrNameFilter, ULONG a_nTags, BSTR* a_pTags, IEnum2Strings* a_pFiles, BSTR a_bstrDatabase) = 0;
};

extern __declspec(selectany) CLSID const CLSID_StorageFactorySQLite = { 0x9bdb81b6, 0x14df, 0x4093, { 0xb9, 0x33, 0xf2, 0x5a, 0xfa, 0x41, 0x6e, 0x60 } };

class DECLSPEC_UUID("9BDB81B6-14DF-4093-B933-F25AFA416E60") StorageFactorySQLite;
