
#pragma once

#include <RWConceptDesignerView.h>
#include <RWConceptStructuredData.h>
#include <RWProcessing.h>


extern __declspec(selectany) IID const IID_IDesignerViewStructure = { 0x72ea913e, 0x75ba, 0x4af7, { 0xb3, 0xb5, 0x9e, 0x15, 0xbd, 0x0b, 0x59, 0xf0 } };

MIDL_INTERFACE("72EA913E-75BA-4AF7-B3B5-9E15BD0B59F0")
IDesignerViewStructure : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE RootIDGet(IID* a_pID) = 0;
    virtual HRESULT STDMETHODCALLTYPE RootIDSet(REFIID a_tID) = 0;
    virtual HRESULT STDMETHODCALLTYPE SelectionGet(IEnumUnknowns** a_ppSelection) = 0;
};


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryTree = { 0x1f556e10, 0x8a41, 0x4a5a, { 0x89, 0x63, 0x67, 0x45, 0x76, 0xcc, 0x25, 0x28 } };
class DECLSPEC_UUID("1F556E10-8A41-4A5A-8963-674576CC2528") DesignerViewFactoryTree;


extern __declspec(selectany) CLSID const CLSID_MenuCommandsStructureRoot = { 0x71f86ea7, 0x8be7, 0x4799, { 0x80, 0xbd, 0x20, 0xda, 0x32, 0x56, 0x93, 0xd0 } };
class DECLSPEC_UUID("71F86EA7-8BE7-4799-80BD-20DA325693D0") MenuCommandsStructureRoot;


extern __declspec(selectany) CLSID const CLSID_MenuCommandsStructuredItemValue = { 0xade6a7b0, 0x40ef, 0x43f3, { 0xa3, 0xb6, 0x23, 0x7f, 0xfc, 0xf9, 0xc2, 0xf9 } };
class DECLSPEC_UUID("ADE6A7B0-40EF-43F3-A3B6-237FFCF9C2F9") MenuCommandsStructuredItemValue;


extern __declspec(selectany) CLSID const CLSID_DocumentOperationExtractSubDocument = { 0x776a2a10, 0x8a38, 0x4f0f, { 0x8d, 0x39, 0xb8, 0x1c, 0x85, 0x75, 0xe2, 0x43 } };
class DECLSPEC_UUID("776A2A10-8A38-4F0F-8D39-B81C8575E243") DocumentOperationExtractSubDocument;


extern __declspec(selectany) CLSID const CLSID_DesignerViewSubDocumentFrame = { 0xeb5443e1, 0x398a, 0x4c59, { 0x8c, 0x7d, 0xd3, 0xce, 0x83, 0x0b, 0x4a, 0xa0 } };
class DECLSPEC_UUID("EB5443E1-398A-4C59-8C7D-D3CE830B4AA0") DesignerViewSubDocumentFrame;


extern __declspec(selectany) CLSID const CLSID_DesignerViewFactoryChooseBestLayout = { 0x20748c76, 0x68a1, 0x47ea, { 0x8b, 0x21, 0xfa, 0xca, 0xa9, 0x67, 0xf4, 0x54 } };
class DECLSPEC_UUID("20748C76-68A1-47EA-8B21-FACAA967F454") DesignerViewFactoryChooseBestLayout;


extern __declspec(selectany) CLSID const CLSID_MenuCommandsSubDocument = { 0xc42c4259, 0x1bc2, 0x402c, { 0x9d, 0xc0, 0x89, 0xd4, 0xb5, 0xcb, 0x6a, 0x9f } };
class DECLSPEC_UUID("C42C4259-1BC2-402C-9DC0-89D4B5CB6A9F") MenuCommandsSubDocument;


extern __declspec(selectany) CLSID const CLSID_MenuCommandsCondition = { 0x656252a2, 0x785d, 0x413b, { 0xb9, 0xb7, 0xd3, 0xf2, 0xe8, 0x63, 0x2e, 0x32 } };
class DECLSPEC_UUID("656252A2-785D-413B-B9B7-D3F2E8632E32") MenuCommandsCondition;

