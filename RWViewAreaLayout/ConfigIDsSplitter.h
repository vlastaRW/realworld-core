
#pragma once

extern __declspec(selectany) OLECHAR const CFGID_SPLITTYPE[] =		L"SplitType";
extern __declspec(selectany) OLECHAR const CFGID_HORDIVTYPE[] =		L"HorDivType";
extern __declspec(selectany) OLECHAR const CFGID_HORRELATIVE[] =	L"HorRelative";
extern __declspec(selectany) OLECHAR const CFGID_HORABSOLUTE[] =	L"HorAbsolute";
extern __declspec(selectany) OLECHAR const CFGID_VERDIVTYPE[] =		L"VerDivType";
extern __declspec(selectany) OLECHAR const CFGID_VERRELATIVE[] =	L"VerRelative";
extern __declspec(selectany) OLECHAR const CFGID_VERABSOLUTE[] =	L"VerAbsolute";
extern __declspec(selectany) OLECHAR const CFGID_SUBVIEWLT[] =		L"SubViewLT";
extern __declspec(selectany) OLECHAR const CFGID_SUBVIEWLB[] =		L"SubViewLB";
extern __declspec(selectany) OLECHAR const CFGID_SUBVIEWRT[] =		L"SubViewRT";
extern __declspec(selectany) OLECHAR const CFGID_SUBVIEWRB[] =		L"SubViewRB";

static const LONG ESTVertical = 0;
static const LONG ESTBoth = 1;
static const LONG ESTHorizontal = 2;

static const LONG EDTAdjustableMask = 1;
static const LONG EDTRelativeFixed = 0;
static const LONG EDTRelativeAdjustable = 1;
static const LONG EDTAbsoluteLTFixed = 2;
static const LONG EDTAbsoluteLTAdjustable = 3;
static const LONG EDTAbsoluteRBFixed = 4;
static const LONG EDTAbsoluteRBAdjustable = 5;
