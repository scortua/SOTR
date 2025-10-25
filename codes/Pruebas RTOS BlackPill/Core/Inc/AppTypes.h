#ifndef INC_APPTYPES_H_
#define INC_APPTYPES_H_

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long s64;

typedef unsigned char * pu8;
typedef unsigned short * pu16;
typedef unsigned int * pu32;
typedef unsigned long * pu64;

typedef signed char * ps8;
typedef signed short * ps16;
typedef signed int * ps32;
typedef signed long * ps64;

typedef float f;
typedef double d;
typedef float * pf;
typedef double * pd;

typedef void v;
typedef void * pv;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif /* INC_APPTYPES_H_ */
