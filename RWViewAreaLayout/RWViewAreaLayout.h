
#pragma once

#include <RWConceptDesignerView.h>
#include <RWProcessing.h>


extern __declspec(selectany) IID const IID_IDesignerViewTabControl = { 0xa21902ce, 0x22c5, 0x45ed, { 0x9b, 0xc7, 0x4c, 0x0e, 0x41, 0x73, 0x14, 0xec } };

MIDL_INTERFACE("A21902CE-22C5-45ED-9BC7-4C0E417314EC")
IDesignerViewTabControl : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE TabID(BSTR* a_pbstrID) = 0;
    virtual HRESULT STDMETHODCALLTYPE ActiveIndexGet(ULONG* a_pnIndex) = 0;
    virtual HRESULT STDMETHODCALLTYPE ActiveIndexSet(ULONG a_nIndex) = 0;
    virtual HRESULT STDMETHODCALLTYPE ItemCount(ULONG* a_pnCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE NameGet(ULONG a_nIndex,ILocalizedString** a_ppName) = 0;
    virtual HRESULT STDMETHODCALLTYPE IconIDGet(ULONG a_nIndex, GUID* a_ptIconID) = 0;
    virtual HRESULT STDMETHODCALLTYPE IconGet(ULONG a_nIndex, ULONG a_nSize, HICON *a_phIcon) = 0;
};


extern __declspec(selectany) IID const IID_IDesignerViewDockingWindows = { 0x8d1187f6, 0xdc70, 0x4a20, { 0xa5, 0x2b, 0x3c, 0x87, 0x60, 0x9a, 0x27, 0xea } };

MIDL_INTERFACE("8D1187F6-DC70-4A20-A52B-3C87609A27EA")
IDesignerViewDockingWindows : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE ItemCount(ULONG* a_pnCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE NameGet(ULONG a_nIndex, ILocalizedString** a_ppName) = 0;
    virtual HRESULT STDMETHODCALLTYPE IconIDGet(ULONG a_nIndex, GUID* a_ptIconID) = 0;
    virtual HRESULT STDMETHODCALLTYPE IconGet(ULONG a_nIndex, ULONG a_nSize, HICON *a_phIcon) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsVisible(ULONG a_nIndex) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetVisible(ULONG a_nIndex, BOOL a_bVisible) = 0;
};


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactorySplitter = { 0xb3012a45, 0xa4f9, 0x4304, { 0xb9, 0xd2, 0x60, 0x67, 0x23, 0x9d, 0x1a, 0xb7 } };
class DECLSPEC_UUID("B3012A45-A4F9-4304-B9D2-6067239D1AB7") DesignerViewFactorySplitter;


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryDockingWindows = { 0x33338b5b, 0x79e3, 0x41f6, { 0xb9, 0x22, 0x87, 0x43, 0xe6, 0x3a, 0x8d, 0xa0 } };
class DECLSPEC_UUID("33338B5B-79E3-41F6-B922-8743E63A8DA0") DesignerViewFactoryDockingWindows;


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryTab = { 0x2ce75e93, 0xf88a, 0x4e49, { 0x80, 0xdc, 0x6c, 0xa5, 0xe8, 0xde, 0x28, 0x0c } };
class DECLSPEC_UUID("2CE75E93-F88A-4E49-80DC-6CA5E8DE280C") DesignerViewFactoryTab;


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryRolldown = { 0xe6264682, 0x5eca, 0x4723, { 0xa7, 0xb2, 0x6e, 0xd9, 0x62, 0xa7, 0x3d, 0xee } };
class DECLSPEC_UUID("E6264682-5ECA-4723-A7B2-6ED962A73DEE") DesignerViewFactoryRolldown;


extern __declspec(selectany) CLSID const CLSID_MenuCommandsTabControl = { 0x105f9237, 0x86d5, 0x4117, { 0x85, 0x55, 0x9e, 0x39, 0x30, 0x99, 0x34, 0xa7 } };
class DECLSPEC_UUID("105F9237-86D5-4117-8555-9E39309934A7") MenuCommandsTabControl;

