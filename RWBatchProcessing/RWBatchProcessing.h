
#pragma once

#include <RWConceptDesignerExtension.h>
#include <RWDesignerCore.h>


extern __declspec(selectany) IID const IID_ICommandLineProcessorFeedback = { 0x8bcaf3b0, 0xfe54, 0x4b61, { 0x8c, 0xe4, 0x5e, 0x0f, 0x0b, 0xbb, 0xda, 0x80 } };

MIDL_INTERFACE("8BCAF3B0-FE54-4B61-8CE4-5E0F0BBBDA80")
ICommandLineProcessorFeedback : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Message(BSTR a_bstr) = 0;
};


extern __declspec(selectany) IID const IID_ICommandLineProcessorProgress = { 0x89725335, 0xbb0a, 0x45a7, { 0xa8, 0x57, 0x0d, 0xde, 0x4f, 0xa6, 0x00, 0xa0 } };

MIDL_INTERFACE("89725335-BB0A-45A7-A857-0DDE4FA600A0")
ICommandLineProcessorProgress : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Tick(ULONG a_processed, ULONG a_left) = 0;
};


extern __declspec(selectany) IID const IID_ICommandLineProcessor = { 0x031ab82c, 0x30cf, 0x43fb, { 0x95, 0xab, 0x6c, 0xd4, 0x50, 0x2a, 0xa9, 0xee } };

MIDL_INTERFACE("031AB82C-30CF-43FB-95AB-6CD4502AA9EE")
ICommandLineProcessor : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE TargetFolder(BSTR* a_pbstrPath) = 0;
    virtual HRESULT STDMETHODCALLTYPE Run(IConfig* a_pOperation, IEnumStrings* a_pPaths, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrApplication, ICommandLineProcessorFeedback* a_pFeedback) = 0;
    virtual HRESULT STDMETHODCALLTYPE InitOpConfig(BSTR a_bstrApplication, IConfig** a_ppOperation) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetOpProps(IConfig* a_pOperation, BSTR* a_pName, BSTR* a_pDesc, GUID* a_pIconID, BSTR* a_pOutput, GUID* a_pFactory, BSTR* a_pOrigName, BSTR* a_pCustomFilter) = 0;
    virtual HRESULT STDMETHODCALLTYPE RunEx(IConfig* a_pOperation, IEnumStrings* a_pPaths, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrApplication, BSTR a_bstrOutputFolder, ICommandLineProcessorFeedback* a_pFeedback, ICommandLineProcessorProgress* a_pProgress) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSubOps(IConfig* a_pOperation, LCID a_tLocaleID, IEnumUnknowns** a_pSubOps) = 0;
};


extern __declspec(selectany) CLSID const CLSID_StartPageBatchImageProcessor = { 0x62388dc7, 0x0664, 0x4504, { 0x89, 0x0d, 0xac, 0x11, 0x75, 0x64, 0x41, 0x80 } };
class DECLSPEC_UUID("62388DC7-0664-4504-890D-AC1175644180") StartPageBatchImageProcessor;
