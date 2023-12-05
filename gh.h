#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef unsigned char uchar, ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulong;

#ifdef UNICODE
#define CHAR wchar_t
#define STR(s) L## s
#define char_printf wprintf
#define char_strlen(str) (wcslen(str))
#define char_strlen_s(str, max_l) (wcsnlen(str, max_l))
#define char_sprintf _swprintf
#define char_sprintf_s swprintf
#define char_strcpy(dst, src) wcscpy(dst, src)
#define char_strcpy_s(dst, dst_len, src) wcscpy_s(dst, dst_len, src)
#define char_strchr(str, chr) wcschr(str, chr)
#else
#define CHAR char
#define STR(s) s
#define char_printf printf
#define char_strlen(str) (strlen(str))
#define char_strlen_s(str, max_l) (strnlen(str, max_l))
#define char_sprintf sprintf
#define char_sprintf_s sprintf_s
#define char_strcpy(dst, src) strcpy(dst, src)
#define char_strcpy_s(dst, dst_len, src) strcpy_s(dst, dst_len, src)
#define char_strchr(str, chr) strchr(str, chr)
#endif

#define ASSERT(cond) if (!(cond)) { printf("FAILED ASSERTION OF CONDITION \"" #cond "\" AT \"" __FILE__ "\" LINE %d\n", __LINE__); abort(); }


#define OUT
#define INOUT

// #define VERBOSE


typedef struct
{
	float x, y;
} vec2f_t;

typedef struct
{
	int x, y;
} vec2i_t;

typedef struct
{
	int x, y, w, h;
} rect2i_t;

typedef struct
{
	float x, y, w, h;
} rect2f_t;

// includs edges
static inline float clampf(float value, float min, float max)
{
	if (value > min)
		return value < max ? value : max;
	return min;
}

// includs edges
static inline double clampd(double value, double min, double max)
{
	if (value > min)
		return value < max ? value : max;
	return min;
}

// includes min, excludes max
static inline int clampi(int value, int min, int max)
{
	if (value > min)
		return value < max ? value : max - 1;
	return min;
}


