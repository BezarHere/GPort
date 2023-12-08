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
#define UNICHAR_MODE
typedef wchar_t nchar_t;
#define STR(s) (L## s)
#define nc_printf wprintf
#define nc_strlen(str) (wcslen(str))
#define nc_strlen_s(str, max_l) (wcsnlen(str, max_l))
#define nc_sprintf _swprintf
#define nc_sprintf_s swprintf
#define nc_strcpy(dst, src) wcscpy(dst, src)
#define nc_strcpy_s(dst, dst_len, src) wcscpy_s(dst, dst_len, src)
#define nc_strchr(str, chr) wcschr(str, chr)
#define nc_strrchr(str, chr) wcsrchr(str, chr)

#define WIN_API(func) func## W
#else
typedef char nchar_t;
#define STR(s) (s)
#define nc_printf printf
#define nc_strlen(str) (strlen(str))
#define nc_strlen_s(str, max_l) (strnlen(str, max_l))
#define nc_sprintf sprintf
#define nc_sprintf_s sprintf_s
#define nc_strcpy(dst, src) strcpy(dst, src)
#define nc_strcpy_s(dst, dst_len, src) strcpy_s(dst, dst_len, src)
#define nc_strchr(str, chr) strchr(str, chr)
#define nc_strrchr(str, chr) strrchr(str, chr)

#define WIN_API(func) func## A
#endif

#define ASSERT(cond) if (!(cond)) { printf("FAILED ASSERTION OF CONDITION \"" #cond "\" AT \"" __FILE__ "::%s\" LINE %d\n", __FUNCTION__, __LINE__); abort(); }


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

typedef struct
{
	size_t begin, end;
} rangei_t;


// includs edges
static inline float fclampf(float value, float min, float max)
{
	if (value > min)
		return value < max ? value : max;
	return min;
}

// includs edges
static inline double fclampd(double value, double min, double max)
{
	if (value > min)
		return value < max ? value : max;
	return min;
}

// includes min, excludes max
static inline int iclampi(int value, int min, int max)
{
	if (value > min)
		return value < max ? value : max - 1;
	return min;
}

// includes min, excludes max
static inline long long iclampll(long long value, long long min, long long max)
{
	if (value > min)
		return value < max ? value : max - 1;
	return min;
}

static inline int imini(int a, int b)
{
	return a > b ? b : a;
}

static inline long long iminll(long long a, long long b)
{
	return a > b ? b : a;
}

static inline int imaxi(int a, int b)
{
	return a < b ? b : a;
}

static inline long long imaxll(long long a, long long b)
{
	return a < b ? b : a;
}


static inline void str_psplit_2(const nchar_t *str, size_t splitpoint, OUT nchar_t *left, OUT nchar_t *right)
{
	const size_t len = nc_strlen(str);
	if (len < splitpoint)
		splitpoint = len;

	if (left)
	{
		memcpy(left, str, splitpoint * sizeof(nchar_t));
		left[splitpoint] = 0;
	}

	if (right)
	{
		memcpy(right, str + splitpoint, len - splitpoint);
		right[len - splitpoint] = 0;
	}
}

