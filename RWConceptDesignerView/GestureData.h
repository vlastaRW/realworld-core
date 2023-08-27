/*
 Copyright (c) 2001 
 Author: Konstantin Boukreev 
 E-mail: konstantin@mail.primorye.ru 

 Created: 02.11.2001 14:36:27
 Version: 1.0.0

*/

#ifndef _GestureData_5e525b2b_09ab_4246_b72a_da99c2fb07d7
#define _GestureData_5e525b2b_09ab_4246_b72a_da99c2fb07d7

#if _MSC_VER > 1000 
#pragma once
#endif // _MSC_VER > 1000

#define RANGE_SIZE			16
#define NET_INPUT_SIZE		(RANGE_SIZE*2)
#define NET_OUTPUT_SIZE		(sizeof(g_aGestureSpecifications)/sizeof(*g_aGestureSpecifications))
#define NUMBER_OF_PATTERNS	NET_OUTPUT_SIZE


#endif //_GestureData_5e525b2b_09ab_4246_b72a_da99c2fb07d7

