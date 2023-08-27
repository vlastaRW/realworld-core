
#pragma once

class CInPlaceCalc
{
public:
	static double EvalExpression(LPCTSTR a_pszExpression, LPCTSTR* a_ppszEnd = NULL)
	{
		LPCTSTR p = a_pszExpression;
		if (a_ppszEnd) *a_ppszEnd = a_pszExpression;
		double fResult = 0.0;
		double fSign = 1.0;
		while (true) switch (*p)
		{
		case _T(' '):
			++p;
			break;
		case _T('+'):
			fSign = 1.0;
			++p;
			break;
		case _T('-'):
			fSign = -1.0;
			++p;
			break;
		case _T('('):
		case _T('0'):
		case _T('1'):
		case _T('2'):
		case _T('3'):
		case _T('4'):
		case _T('5'):
		case _T('6'):
		case _T('7'):
		case _T('8'):
		case _T('9'):
		case _T('.'):
		case _T('p'):
			{
				LPCTSTR p2 = p;
				fResult += fSign*EvalTerm(p, &p2);
				fSign = 1.0;
				if (p == p2)
				{
					if (a_ppszEnd) *a_ppszEnd = p;
					return fResult;
				}
				else
				{
					p = p2;
				}
			}
			break;
		default:
			if (a_ppszEnd) *a_ppszEnd = p;
			return fResult;
		}
	}
	static double EvalTerm(LPCTSTR a_pszTerm, LPCTSTR* a_ppszEnd = NULL)
	{
		LPCTSTR p = a_pszTerm;
		if (a_ppszEnd) *a_ppszEnd = a_pszTerm;
		double fResult = 1.0;
		bool bDiv = false;
		while (true) switch (*p)
		{
		case _T(' '):
			++p;
			break;
		case _T('*'):
			bDiv = false;
			++p;
			break;
		case _T('/'):
			bDiv = true;
			++p;
			break;
		case _T('('):
			++p;
			if (bDiv)
				fResult /= EvalExpression(p, &p);
			else
				fResult *= EvalExpression(p, &p);
			while (*p == _T(' ')) ++p;
			if (*p != _T(')'))
			{
				if (a_ppszEnd) *a_ppszEnd = p;
				return fResult;
			}
			++p;
			break;
		case _T('p'):
			if (p[1] == _T('i'))
			{
				if (bDiv)
					fResult /= 3.14159265358979323846;
				else
					fResult *= 3.14159265358979323846;
				p+=2;
				break;
			}
			else
			{
				if (a_ppszEnd) *a_ppszEnd = p;
				return fResult;
			}
		case _T('0'):
		case _T('1'):
		case _T('2'):
		case _T('3'):
		case _T('4'):
		case _T('5'):
		case _T('6'):
		case _T('7'):
		case _T('8'):
		case _T('9'):
		case _T('.'):
			{
				LPTSTR p2 = const_cast<LPTSTR>(p);
				if (bDiv)
					fResult /= _tcstod(p, &p2);
				else
					fResult *= _tcstod(p, &p2);
				if (p2 == p)
				{
					if (a_ppszEnd) *a_ppszEnd = p;
					return fResult;
				}
				else
				{
					p = p2;
				}
			}
			break;
		default:
			if (a_ppszEnd) *a_ppszEnd = p;
			return fResult;
		}
	}
};
