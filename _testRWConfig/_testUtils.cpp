
#include "stdafx.h"

#include "_testUtils.h"
#include "resource.h"

#include "CustomGUITest.h"
#include <SharedStringTable.h>


// {1EEB761B-0810-4840-A8D0-64D08E64CE94}
extern const GUID tCustomConfigUID = 
{ 0x1eeb761b, 0x810, 0x4840, { 0xa8, 0xd0, 0x64, 0xd0, 0x8e, 0x64, 0xce, 0x94 } };

IConfig* CreateSimpleConfig()
{
	try
	{
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemIns1ofN(CComBSTR(L"TestInt"), _SharedStringTable.GetStringAuto(IDS_TESTINTNAME), _SharedStringTable.GetStringAuto(IDS_TESTINTHELP), CConfigValue(7L), NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestInt"), CConfigValue(7L), _SharedStringTable.GetStringAuto(IDS_TESTINT7), 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestInt"), CConfigValue(5L), NULL, 0, NULL);

		CComBSTR bstrTestRangeInt(L"TestRangeInt");
		pCfg->ItemInsRanged(bstrTestRangeInt, _SharedStringTable.GetStringAuto(IDS_TESTRNGINTNAME), _SharedStringTable.GetStringAuto(IDS_TESTRNGINTHELP), CConfigValue(5L), NULL, CConfigValue(1L), CConfigValue(10L), CConfigValue(1L), 0, NULL);

		pCfg->ItemIns1ofN(CComBSTR(L"TestString"), _SharedStringTable.GetStringAuto(IDS_TESTSTRINGNAME), _SharedStringTable.GetStringAuto(IDS_TESTSTRINGHELP), CConfigValue(_T("Default")), NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestString"), CConfigValue(_T("str1")), NULL, 0, NULL);

		TConfigOptionCondition tCondition;
		tCondition.bstrID = bstrTestRangeInt;
		tCondition.eConditionType = ECOCGreater;
		tCondition.tValue = CConfigValue(5L);
		pCfg->ItemIns1ofN(CComBSTR(L"TestBool"), _SharedStringTable.GetStringAuto(IDS_TESTBOOLNAME), _SharedStringTable.GetStringAuto(IDS_TESTBOOLHELP), CConfigValue(false), NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestBool"), CConfigValue(false), NULL, 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestBool"), CConfigValue(true), NULL, 1, &tCondition);

		pCfg->ItemInsSimple(CComBSTR(L"SimpleBool"), _SharedStringTable.GetStringAuto(IDS_TESTBOOLNAME), _SharedStringTable.GetStringAuto(IDS_TESTBOOLHELP), CConfigValue(true), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(L"TestEdit"), _SharedStringTable.GetStringAuto(IDS_TESTSTRINGNAME), _SharedStringTable.GetStringAuto(IDS_TESTSTRINGHELP), CConfigValue(L""), NULL, 0, NULL);

		CComObject<CConfigCustomGUI<&tCustomConfigUID, CCustomGUITestDlg> >* pCustomGUI = NULL;
		CComObject<CConfigCustomGUI<&tCustomConfigUID, CCustomGUITestDlg> >::CreateInstance(&pCustomGUI);
		CComPtr<IConfigCustomGUI> pGUITmp = pCustomGUI;
		pCfg->Finalize(pGUITmp);

		CComQIPtr<IConfig> pOut(pCfg.p);
		return pOut.Detach();
	}
	catch (...)
	{
		return NULL;
	}
}

IConfig* CreateSimpleConfig2()
{
	const static GUID g1 = {0xe4562a72, 0x4133, 0x41c3, {0xf0, 0xb7, 0x3b, 0x7e, 0x3b, 0xf9, 0xb0, 0x06}};
	const static GUID g2 = {0xe4562a72, 0x4133, 0x41c3, {0xf0, 0xb7, 0x3b, 0x7e, 0x3b, 0xf9, 0xb0, 0x07}};
	try
	{
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemIns1ofN(CComBSTR(L"TestGUID"), _SharedStringTable.GetStringAuto(IDS_TESTGUIDNAME), _SharedStringTable.GetStringAuto(IDS_TESTGUIDHELP), CConfigValue(g1), NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestGUID"), CConfigValue(g1), NULL, 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestGUID"), CConfigValue(g2), _SharedStringTable.GetStringAuto(IDS_TESTGUID1), 0, NULL);

		pCfg->Finalize(NULL);

		CComQIPtr<IConfig> pOut(pCfg.p);
		return pOut.Detach();
	}
	catch (...)
	{
		return NULL;
	}
}

ISubConfig* CreateVectorConfig()
{
	try
	{
		CComPtr<ISubConfigVector> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(SubConfigVector));

		CComPtr<IConfig> pSubCfgItem;
		pSubCfgItem.Attach(CreateSimpleConfig());

//		pCfg->Init(FALSE, pSubCfgItem);
		pCfg->Init(TRUE, pSubCfgItem);

		CComQIPtr<ISubConfig> pOut(pCfg.p);
		return pOut.Detach();
	}
	catch (...)
	{
		return NULL;
	}
}

IConfig* CreateCompositeConfig()
{
	try
	{
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		CComPtr<IConfig> pSubCfgItem;
		pSubCfgItem.Attach(CreateSimpleConfig());
		CComPtr<IConfig> pSubCfgItem2;
		pSubCfgItem2.Attach(CreateSimpleConfig2());

		CComPtr<ISubConfigSwitch> pSubCfg;
		RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitch));
		pSubCfg->ItemInsert(CConfigValue(0L), pSubCfgItem);
		pSubCfg->ItemInsert(CConfigValue(1L), pSubCfgItem2);

		CComPtr<ISubConfig> pSubCfgVect;
		pSubCfgVect.Attach(CreateVectorConfig());

		pCfg->ItemIns1ofN(CComBSTR(L"TestVector"), _SharedStringTable.GetStringAuto(IDS_TESTVECTORNAME), _SharedStringTable.GetStringAuto(IDS_TESTVECTORHELP), CConfigValue(1L), pSubCfgVect.p);
		pCfg->ItemOptionAdd(CComBSTR(L"TestVector"), CConfigValue(0L), NULL, 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestVector"), CConfigValue(1L), NULL, 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestVector"), CConfigValue(2L), NULL, 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestVector"), CConfigValue(3L), NULL, 0, NULL);

		pCfg->ItemIns1ofN(CComBSTR(L"TestSwitch"), _SharedStringTable.GetStringAuto(IDS_TESTSWITCHNAME), _SharedStringTable.GetStringAuto(IDS_TESTSWITCHHELP), CConfigValue(0L), pSubCfg.p);
		pCfg->ItemOptionAdd(CComBSTR(L"TestSwitch"), CConfigValue(0L), NULL, 0, NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"TestSwitch"), CConfigValue(1L), NULL, 0, NULL);

		pCfg->Finalize(NULL);

		CComQIPtr<IConfig> pOut(pCfg.p);
		return pOut.Detach();
	}
	catch (...)
	{
		return NULL;
	}
}

