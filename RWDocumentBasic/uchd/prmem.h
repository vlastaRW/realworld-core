
#pragma once

inline void* PR_Malloc(unsigned nLen)
{
	try
	{
		return nLen ? new char[nLen] : 0;
	}
	catch (...)
	{
		return 0;
	}
}

inline void PR_FREEIF(void* p)
{
	delete[] (char*)p;
}