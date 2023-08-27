
#pragma once

#include <RWConceptDesignerView.h>


extern __declspec(selectany) IID const IID_IScintillaFactory = { 0xeac1611d, 0x15c0, 0x461b, { 0xb3, 0x8e, 0xe7, 0xe3, 0xfb, 0xe0, 0xe0, 0xcf } };

MIDL_INTERFACE("EAC1611D-15C0-461B-B38E-E7E3FBE0E0CF")
IScintillaFactory : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE RegisterClasses() = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterClasses() = 0;
    //[helpstring("method CreateWindow")] HRESULT CreateWindow([in] RWHWND a_hParent, [in] RECT const* a_prc, [out] RWHWND* a_phWnd);
};

extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryText = { 0x4b01b979, 0xa6b1, 0x4144, { 0x83, 0xb6, 0x49, 0x57, 0x1b, 0x46, 0xce, 0xb6 } };
class DECLSPEC_UUID("4B01B979-A6B1-4144-83B6-49571B46CEB6") DesignerViewFactoryText;


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryBinary = { 0x8384447b, 0x0059, 0x4075, { 0x91, 0xa3, 0x63, 0xf1, 0x05, 0xca, 0x8d, 0x4c } };
class DECLSPEC_UUID("8384447B-0059-4075-91A3-63F105CA8D4C") DesignerViewFactoryBinary;
