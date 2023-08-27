
#pragma once

#include <RWBase.h>
#include <RWConfig.h>


enum ETranslatorChange
{
    ETCCurrentLanguage = 1,
    ETCMajorUpdate = 2,
    ETCMinorUpdate = 4,
    ETCInactiveUpdate = 8,
};


extern __declspec(selectany) IID const IID_ITranslatorObserver = { 0x8ccac9cd, 0x729c, 0x4693, { 0xb7, 0x59, 0xd7, 0xda, 0x4f, 0x00, 0x67, 0x01 } };

MIDL_INTERFACE("8CCAC9CD-729C-4693-B759-D7DA4F006701")
ITranslatorObserver : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Notify(TCookie a_tCookie, ULONG a_nFlags) = 0;
};


extern __declspec(selectany) IID const IID_ITranslatorManager = { 0x93cbd74d, 0xe549, 0x4c10, { 0xa6, 0x4e, 0x61, 0x76, 0x36, 0xde, 0xf7, 0xfa } };

MIDL_INTERFACE("93CBD74D-E549-4C10-A64E-617636DEF7FA")
ITranslatorManager : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Initialize(IApplicationInfo * a_pInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE Finalize(BOOL a_bAllowPOFileUpdates) = 0;
    virtual HRESULT STDMETHODCALLTYPE LangIcon(WORD a_wLangID, HICON* a_pLangIcon) = 0;
    virtual HRESULT STDMETHODCALLTYPE LangInfo(WORD a_wLangID, BOOL* a_pBuiltIn, ULONG* a_pTimestamp, ULONG* a_pStandard, ULONG* a_pCustom, ULONG* a_pMissing) = 0;
    virtual HRESULT STDMETHODCALLTYPE StringsEnum(WORD a_wLangID, IEnumStrings** a_ppStrings) = 0;
    virtual HRESULT STDMETHODCALLTYPE StringInfo(WORD a_wLangID, BSTR a_bstrOrig, BSTR* a_pbstrTransl, BOOL* a_pCustom, ULONG* a_pPriority) = 0;
    virtual HRESULT STDMETHODCALLTYPE StringSet(WORD a_wLangID, BSTR a_bstrOrig, BSTR a_bstrTransl, BOOL a_bCustom) = 0;
    virtual HRESULT STDMETHODCALLTYPE Synchronize(WORD a_wLangID, BYTE a_bUntranslatedStrings) = 0;
    virtual HRESULT STDMETHODCALLTYPE ObserverIns(ITranslatorObserver* a_pObserver, TCookie a_tCookie) = 0;
    virtual HRESULT STDMETHODCALLTYPE ObserverDel(ITranslatorObserver* a_pObserver, TCookie a_tCookie) = 0;
};


extern __declspec(selectany) CLSID const CLSID_Translator = { 0xa8527808, 0x1328, 0x4221, { 0x98, 0x58, 0x18, 0xe4, 0x77, 0x99, 0xc3, 0x79 } };
class DECLSPEC_UUID("A8527808-1328-4221-9858-18E47799C379") Translator;


static const OLECHAR CFGID_LANGUAGECODE[] = L"LanguageCode";

