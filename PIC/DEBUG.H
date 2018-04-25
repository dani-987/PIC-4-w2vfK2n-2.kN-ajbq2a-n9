#pragma once

//some header need WIN32 defined, but VS sets _WIN32 instead
#ifdef _WIN32
#define WIN32
#endif

#define USE_STD_MUTEX
//#define USE_MY_MUTEX

//lock and unlock functions of mutex
#define LOCK_MUTEX(m)			m.lock()
#define UNLOCK_MUTEX(m)			m.unlock()

//functions for debuging (e.g. PRINTF or Variabledefines, that are ignored at release-build)
#ifdef _DEBUG
#define PRINTF(a)					printf(a)
#define PRINTF1(a,b)				printf(a,b)
#define PRINTF2(a,b,c)				printf(a,b,c)
#define PRINTF3(a,b,c,d)			printf(a,b,c,d)
#define PRINTF4(a,b,c,d,e)			printf(a,b,c,d,e)
#define PRINTF5(a,b,c,d,e,f)		printf(a,b,c,d,e,f)
#define PRINTF6(a,b,c,d,e,f,g)		printf(a,b,c,d,e,f,g)
#define PRINTF7(a,b,c,d,e,f,g,h)	printf(a,b,c,d,e,f,g,h)
#define PRINTF8(a,b,c,d,e,f,g,h,i)	printf(a,b,c,d,e,f,g,h,i)
#define DOIF(a)					if(a)
#define VARDEF(type,name,value)	type name = value
#else
#define PRINTF(a)
#define PRINTF1(a,b)
#define PRINTF2(a,b,c)
#define PRINTF3(a,b,c,d)
#define PRINTF4(a,b,c,d,e)
#define PRINTF5(a,b,c,d,e,f)
#define PRINTF6(a,b,c,d,e,f,g)
#define PRINTF7(a,b,c,d,e,f,g,h)
#define PRINTF8(a,b,c,d,e,f,g,h,i)
#define DOIF(a)
#define VARDEF(type, name, value)
#endif